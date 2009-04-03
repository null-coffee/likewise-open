/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
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
 *        lpuser.h
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Local Authentication Provider
 *
 *        User Management Routines
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 */
#ifndef __LP_USER_H__
#define __LP_USER_H__

DWORD
LsaLPDbFindUserByName(
    HANDLE  hDb,
    PCSTR   pszDomain,
    PCSTR   pszUserName,
    DWORD   dwUserInfoLevel,
    PVOID*  ppUserInfo
    );

DWORD
LsaLPDbEnumUsers_0(
    HANDLE hDb,
    DWORD  dwOffset,
    DWORD  dwLimit,
    PDWORD pdwNumUsersFound,
    PVOID** pppUserInfoList
    );


DWORD
LsaLPDbFindUserByName_0(
    HANDLE hDb,
    PCSTR  pszUserName,
    PVOID* ppUserInfo
    );

DWORD
LsaLPDbFindUserByName_1(
    HANDLE hDb,
    PCSTR  pszUserName,
    PVOID* ppUserInfo
    );

DWORD
LsaLPDbFindUserByName_2(
    HANDLE hDb,
    PCSTR  pszUserName,
    PVOID* ppUserInfo
    );

DWORD
LsaLPDbEnumUsers_1(
    HANDLE hDb,
    DWORD  dwOffset,
    DWORD  dwLimit,
    PDWORD pdwNumUsersFound,
    PVOID** pppUserInfoList
    );

DWORD
LsaLPDbEnumUsers(
    HANDLE  hDb,
    DWORD   dwUserInfoLevel,
    DWORD   dwStartingRecordId,
    DWORD   nMaxUsers,
    PDWORD  pdwNumUsersFound,
    PVOID** pppUserInfoList
    );;

DWORD
LsaLPDbFindUserById(
    HANDLE hDb,
    uid_t  uid,
    DWORD  dwUserInfoLevel,
    PVOID* ppUserInfo
    );

DWORD
LsaLPDbGetGroupsForUser_0_Unsafe(
    HANDLE  hDb,
    uid_t uid,
    PDWORD  pdwGroupsFound,
    PVOID** pppGroupInfoList
    );

DWORD
LsaLPDbGetGroupsForUser_1_Unsafe(
    HANDLE  hDb,
    uid_t uid,
    PDWORD  pdwGroupsFound,
    PVOID** pppGroupInfoList
    );


DWORD
LsaLPDbFindUserById_0_Unsafe(
    HANDLE  hDb,
    uid_t   uid,
    PVOID*  ppUserInfo
    );


DWORD
LsaLPDbFindUserById_0(
    HANDLE hDb,
    uid_t  uid,
    PVOID* ppUserInfo
    );


DWORD
LsaLPDbFindUserById_1(
    HANDLE hDb,
    uid_t  uid,
    PVOID* ppUserInfo
    );


DWORD
LsaLPDbFindUserById_2(
    HANDLE hDb,
    uid_t  uid,
    PVOID* ppUserInfo
    );

DWORD
LsaLPDbGetGroupsForUser(
    HANDLE  hDb,
    uid_t   uid,
    DWORD   dwGroupInfoLevel,
    PDWORD  pdwGroupsFound,
    PVOID** pppGroupInfoList
    );


DWORD
LsaLPCreateHomeDirectory(
    PLSA_USER_INFO_0 pUserInfo
    );

DWORD
LsaLPProvisionHomeDir(
    uid_t ownerUid,
    gid_t ownerGid,
    PCSTR pszHomedirPath
    );

#endif /* __LP_USER_H__ */
