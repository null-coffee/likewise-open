/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

/*
 * Copyright Likewise Software
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        ioctl.c
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Protocols API - SMBV2
 *
 *        IOCTL
 *
 * Authors: Krishna Ganugapati (kganugapati@likewise.com)
 *          Sriram Nambakam (snambakam@likewise.com)
 *
 *
 */

#include "includes.h"

static
NTSTATUS
SrvBuildIOCTLResponse_SMB_V2(
    PSMB2_MESSAGE              pSmbRequest,
    PLWIO_SRV_CONNECTION       pConnection,
    PSMB2_IOCTL_REQUEST_HEADER pRequestHeader,
    PBYTE                      pResponseBuffer,
    ULONG                      ulResponseBufferLen,
    PSMB_PACKET                pSmbResponse
    );

NTSTATUS
SrvProcessIOCTL_SMB_V2(
    IN     PLWIO_SRV_CONNECTION pConnection,
    IN     PSMB2_MESSAGE        pSmbRequest,
    IN OUT PSMB_PACKET          pSmbResponse
    )
{
    NTSTATUS                   ntStatus = STATUS_SUCCESS;
    PLWIO_SRV_SESSION_2        pSession = NULL;
    PLWIO_SRV_TREE_2           pTree    = NULL;
    PLWIO_SRV_FILE_2           pFile    = NULL;
    PSMB2_IOCTL_REQUEST_HEADER pRequestHeader  = NULL; // Do not free
    PBYTE                      pData           = NULL; // Do not free
    IO_STATUS_BLOCK            ioStatusBlock   = {0};
    PBYTE                      pResponseBuffer = NULL;
    size_t                     sResponseBufferLen  = 0;
    ULONG                      ulResponseBufferLen = 0;

    ntStatus = SrvConnection2FindSession(
                    pConnection,
                    pSmbRequest->pHeader->ullSessionId,
                    &pSession);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvSession2FindTree(
                    pSession,
                    pSmbRequest->pHeader->ulTid,
                    &pTree);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SMB2UnmarshalIOCTLRequest(
                    pSmbRequest,
                    &pRequestHeader,
                    &pData);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvTree2FindFile(
                    pTree,
                    pRequestHeader->fid.ullVolatileId,
                    &pFile);
    BAIL_ON_NT_STATUS(ntStatus);

    // TODO: Should we just allocate 64 * 1024 bytes in this buffer?

    ntStatus = SMBPacketBufferAllocate(
                    pConnection->hPacketAllocator,
                    pRequestHeader->ulMaxOutLength,
                    &pResponseBuffer,
                    &sResponseBufferLen);
    BAIL_ON_NT_STATUS(ntStatus);

    ulResponseBufferLen = sResponseBufferLen;

    if (pRequestHeader->ulFlags & 0x1)
    {
        ntStatus = IoFsControlFile(
                                pFile->hFile,
                                NULL,
                                &ioStatusBlock,
                                pRequestHeader->ulFunctionCode,
                                pData,
                                pRequestHeader->ulInLength,
                                pResponseBuffer,
                                ulResponseBufferLen);
    }
    else
    {
        ntStatus = IoDeviceIoControlFile(
                                pFile->hFile,
                                NULL,
                                &ioStatusBlock,
                                pRequestHeader->ulFunctionCode,
                                pData,
                                pRequestHeader->ulInLength,
                                pResponseBuffer,
                                ulResponseBufferLen);
    }
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvBuildIOCTLResponse_SMB_V2(
                    pSmbRequest,
                    pConnection,
                    pRequestHeader,
                    pResponseBuffer,
                    ioStatusBlock.BytesTransferred,
                    pSmbResponse);
    BAIL_ON_NT_STATUS(ntStatus);

cleanup:

    if (pResponseBuffer)
    {
        SMBPacketBufferFree(
            pConnection->hPacketAllocator,
            pResponseBuffer,
            sResponseBufferLen);
    }

    if (pFile)
    {
        SrvFile2Release(pFile);
    }

    if (pTree)
    {
        SrvTree2Release(pTree);
    }

    if (pSession)
    {
        SrvSession2Release(pSession);
    }

    return ntStatus;

error:

    goto cleanup;
}

static
NTSTATUS
SrvBuildIOCTLResponse_SMB_V2(
    PSMB2_MESSAGE              pSmbRequest,
    PLWIO_SRV_CONNECTION       pConnection,
    PSMB2_IOCTL_REQUEST_HEADER pRequestHeader,
    PBYTE                      pResponseBuffer,
    ULONG                      ulResponseBufferLen,
    PSMB_PACKET                pSmbResponse
    )
{
    NTSTATUS ntStatus = 0;
    PBYTE pOutBufferRef = pSmbResponse->pRawBuffer + pSmbResponse->bufferUsed;
    PBYTE pOutBuffer = pOutBufferRef;
    ULONG ulBytesAvailable = pSmbResponse->bufferLen - pSmbResponse->bufferUsed;
    ULONG ulOffset    = pSmbResponse->bufferUsed - sizeof(NETBIOS_HEADER);
    ULONG ulBytesUsed = 0;
    ULONG ulTotalBytesUsed = 0;

    ntStatus = SMB2MarshalHeader(
                    pOutBuffer,
                    ulOffset,
                    ulBytesAvailable,
                    COM2_IOCTL,
                    0,
                    1,
                    pSmbRequest->pHeader->ulPid,
                    pSmbRequest->pHeader->ullCommandSequence,
                    pSmbRequest->pHeader->ulTid,
                    pSmbRequest->pHeader->ullSessionId,
                    STATUS_SUCCESS,
                    TRUE,
                    NULL,
                    &ulBytesUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    ulTotalBytesUsed += ulBytesUsed;
    pOutBuffer += ulBytesUsed;
    ulOffset += ulBytesUsed;
    ulBytesAvailable -= ulBytesUsed;

    ntStatus = SMB2MarshalIOCTLResponse(
                    pOutBuffer,
                    ulOffset,
                    ulBytesAvailable,
                    pRequestHeader,
                    pResponseBuffer,
                    SMB_MIN(ulResponseBufferLen, pRequestHeader->ulMaxOutLength),
                    &ulBytesUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    ulTotalBytesUsed += ulBytesUsed;
    // pOutBuffer += ulBytesUsed;
    // ulOffset += ulBytesUsed;
    // ulBytesAvailable -= ulBytesUsed;

    pSmbResponse->bufferUsed += ulTotalBytesUsed;

cleanup:

    return ntStatus;

error:

    if (ulTotalBytesUsed)
    {
        memset(pOutBufferRef, 0, ulTotalBytesUsed);
    }

    goto cleanup;
}
