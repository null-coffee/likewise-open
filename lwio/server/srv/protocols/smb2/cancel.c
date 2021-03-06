/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*-
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * Editor Settings: expandtabs and use 4 spaces for indentation */

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
 *        cancel.c
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Protocols API - SMBV2
 *
 *        Cancel
 *
 * Authors: Sriram Nambakam (snambakam@likewise.com)
 *
 */

#include "includes.h"

NTSTATUS
SrvProcessCancel_SMB_V2(
    PSRV_EXEC_CONTEXT pExecContext
    )
{
    NTSTATUS                   ntStatus     = STATUS_SUCCESS;
    PLWIO_SRV_CONNECTION       pConnection  = pExecContext->pConnection;
    PSRV_PROTOCOL_EXEC_CONTEXT pCtxProtocol = pExecContext->pProtocolContext;
    PSRV_EXEC_CONTEXT_SMB_V2   pCtxSmb2     = pCtxProtocol->pSmb2Context;
    ULONG                      iMsg         = pCtxSmb2->iMsg;
    PSRV_MESSAGE_SMB_V2        pSmbRequest  = &pCtxSmb2->pRequests[iMsg];
    PLWIO_ASYNC_STATE          pAsyncState  = NULL;
    ULONG64                    ullAsyncId   = 0LL;

    SRV_LOG_DEBUG(
            pExecContext->pLogContext,
            SMB_PROTOCOL_VERSION_2,
            pSmbRequest->pHeader->command,
            "Cancel request: "
            "command(%u),uid(%llu),cmd-seq(%llu),pid(%u),tid(%u),"
            "credits(%u),flags(0x%x),chain-offset(%u),",
            pSmbRequest->pHeader->command,
            (long long)pSmbRequest->pHeader->ullSessionId,
            (long long)pSmbRequest->pHeader->ullCommandSequence,
            pSmbRequest->pHeader->ulPid,
            pSmbRequest->pHeader->ulTid,
            pSmbRequest->pHeader->usCredits,
            pSmbRequest->pHeader->ulFlags,
            pSmbRequest->pHeader->ulChainOffset);

    if (!(pSmbRequest->pHeader->ulFlags & SMB2_FLAGS_ASYNC_COMMAND) ||
         (pSmbRequest->pHeader->ullCommandSequence != 0))
    {
        ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    ntStatus = SMB2GetAsyncId(pSmbRequest->pHeader, &ullAsyncId);
    BAIL_ON_NT_STATUS(ntStatus);

    SRV_LOG_DEBUG(
            pExecContext->pLogContext,
            SMB_PROTOCOL_VERSION_2,
            pSmbRequest->pHeader->command,
            "Cancel request: "
            "command(%u),uid(%llu),cmd-seq(%llu),pid(%u),tid(%u),"
            "credits(%u),flags(0x%x),chain-offset(%u),async-id(%llu)",
            pSmbRequest->pHeader->command,
            (long long)pSmbRequest->pHeader->ullSessionId,
            (long long)pSmbRequest->pHeader->ullCommandSequence,
            pSmbRequest->pHeader->ulPid,
            pSmbRequest->pHeader->ulTid,
            pSmbRequest->pHeader->usCredits,
            pSmbRequest->pHeader->ulFlags,
            pSmbRequest->pHeader->ulChainOffset,
            (long long) ullAsyncId);

    ntStatus = SrvConnection2FindAsyncState(pConnection, ullAsyncId, &pAsyncState);
    BAIL_ON_NT_STATUS(ntStatus);

    SRV_LOG_DEBUG(
            pExecContext->pLogContext,
            SMB_PROTOCOL_VERSION_2,
            pSmbRequest->pHeader->command,
            "Cancel request: "
            "command(%u),uid(%llu),cmd-seq(%llu),pid(%u),tid(%u),"
            "credits(%u),flags(0x%x),chain-offset(%u),cancel-command(%u:%s)",
            pSmbRequest->pHeader->command,
            (long long)pSmbRequest->pHeader->ullSessionId,
            (long long)pSmbRequest->pHeader->ullCommandSequence,
            pSmbRequest->pHeader->ulPid,
            pSmbRequest->pHeader->ulTid,
            pSmbRequest->pHeader->usCredits,
            pSmbRequest->pHeader->ulFlags,
            pSmbRequest->pHeader->ulChainOffset,
            pAsyncState->usCommand,
            LWIO_SAFE_LOG_STRING(SrvGetCommandDescription_SMB_V2(pAsyncState->usCommand)));

    switch (pAsyncState->usCommand)
    {
        case COM2_NOTIFY:

            SrvCancelChangeNotify_SMB_V2(pAsyncState);

            break;

        case COM2_CREATE:

            SrvCancelCreate_SMB_V2(pAsyncState);

            break;

        case COM2_LOCK:

            SrvCancelLockRequest_SMB_V2(pAsyncState);

            break;

        default:

            ntStatus = STATUS_NOT_SUPPORTED;

            break;
    }
    BAIL_ON_NT_STATUS(ntStatus);

cleanup:

    if (pAsyncState)
    {
        SrvAsyncStateRelease(pAsyncState);
    }

    return ntStatus;

error:

    ntStatus = STATUS_SUCCESS;

    goto cleanup;
}
