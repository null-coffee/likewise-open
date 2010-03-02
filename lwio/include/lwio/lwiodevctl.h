/*
 * Copyright (c) Likewise Software.  All rights Reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewise.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        lwio/lwiodevctl.h
 *
 * Abstract:
 *
 *        Public Device Control codes and structures
 *
 * Authors: Gerald Carter <gcarter@likewise.com>
 */

#ifndef __LW_IO_PUBLIC_DEVICECTL_H__
#define __LW_IO_PUBLIC_DEVICECTL_H__

/* Control Codes */

#define IO_DEVICE_TYPE_DISK_FILE_SYSTEM     0x00080000
#define IO_DEVICE_TYPE_NETWORK_FILE_SYSTEM  0x00140000

#define IO_DEVICE_REQ_ACCESS_READ_DATA      0x00004000

#define IO_DEVICE_CUSTOM_CONTROL_CODE       0x00002000

#define IO_DEVICE_FUNC_CODE_LIST_OPEN_FILES 0x00008010
#define IO_DEVICE_FUNC_CODE_GET_STATS       0x00008020

#define IO_DEVICE_TRANSFER_TYPE_BUFFERED    0x00000000
#define IO_DEVICE_TRANSFER_TYPE_IN_DIRECT   0x00000001
#define IO_DEVICE_TRANSFER_TYPE_OUT_DIRECT  0x00000002
#define IO_DEVICE_TRANSFER_TYPE_NEITHER     0x00000003

#define IO_DEVICE_CTL_OPEN_FILE_INFO   ( IO_DEVICE_TYPE_DISK_FILE_SYSTEM     | \
                                         IO_DEVICE_REQ_ACCESS_READ_DATA      | \
                                         IO_DEVICE_CUSTOM_CONTROL_CODE       | \
                                         IO_DEVICE_FUNC_CODE_LIST_OPEN_FILES | \
                                         IO_DEVICE_TRANSFER_TYPE_NEITHER       \
                                       )
#define IO_DEVICE_CTL_STATISTICS       ( IO_DEVICE_TYPE_NETWORK_FILE_SYSTEM | \
                                         IO_DEVICE_REQ_ACCESS_READ_DATA     | \
                                         IO_DEVICE_CUSTOM_CONTROL_CODE      | \
                                         IO_DEVICE_FUNC_CODE_GET_STATS      | \
                                         IO_DEVICE_TRANSFER_TYPE_NEITHER      \
                                       )

/* Device IoControl structures */

typedef struct _IO_OPEN_FILE_INFO_0
{
    ULONG NextEntryOffset;
    ULONG OpenHandleCount;
    ULONG FileNameLength;
    PWSTR pwszFileName[1];

} IO_OPEN_FILE_INFO_0, *PIO_OPEN_FILE_INFO_0;

typedef struct _IO_OPEN_FILE_INFO_100
{
    ULONG NextEntryOffset;
    ULONG OpenHandleCount;
    BOOLEAN bDeleteOnClose;
    ULONG FileNameLength;
    PWSTR pwszFileName[1];

} IO_OPEN_FILE_INFO_100, *PIO_OPEN_FILE_INFO_100;

typedef struct _IO_OPEN_FILE_INFO_INPUT_BUFFER
{
    DWORD Level;

} IO_OPEN_FILE_INFO_INPUT_BUFFER, *PIO_OPEN_FILE_INFO_INPUT_BUFFER;

typedef struct _IO_STATISTICS_INFO_0
{
    ULONG64 ullNumConnections;
    ULONG64 ullMaxNumConnections;

    ULONG64 ullNumSessions;
    ULONG64 ullMaxNumSessions;

    ULONG64 ullNumTreeConnects;
    ULONG64 ullMaxNumTreeConnects;

    ULONG64 ullNumOpenFiles;
    ULONG64 ullMaxNumOpenFiles;

} IO_STATISTICS_INFO_0, *PIO_STATISTICS_INFO_0;

typedef struct _IO_STATISTICS_INFO_INPUT_BUFFER
{
    DWORD dwInfoLevel;

} IO_STATISTICS_INFO_INPUT_BUFFER, *PIO_STATISTICS_INFO_INPUT_BUFFER;

#endif   /* __LW_IO_PUBLIC_DEVICECTL_H__ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
