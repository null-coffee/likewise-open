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
 *        samdbdefs.c
 *
 * Abstract:
 *
 *
 *      Likewise SAM Database Provider
 *
 *      Provider macros and definitions
 *
 * Authors: Krishna Ganugapati (krishnag@likewise.com)
 *          Sriram Nambakam (snambakam@likewise.com)
 *          Rafal Szczesniak (rafal@likewise.com)
 *
 */

#ifndef __VMDIRDBDEFS_H__
#define __VMDIRDBDEFS_H__

#define SAM_DB_DIR CACHEDIR   "/db"
#define SAM_DB     SAM_DB_DIR "/sam.db"

#define SAM_DB_CONTEXT_POOL_MAX_ENTRIES 10

#define SAM_DB_DEFAULT_ADMINISTRATOR_SHELL   "/bin/sh"
#define SAM_DB_DEFAULT_ADMINISTRATOR_HOMEDIR "/"

#define SAM_DB_DEFAULT_GUEST_SHELL           "/bin/sh"
#define SAM_DB_DEFAULT_GUEST_HOMEDIR         "/tmp"

#define VMDIRDB_LOG_ERROR(pszFormat, ...) LSA_LOG_ERROR(pszFormat, ## __VA_ARGS__)
#define VMDIRDB_LOG_WARNING(pszFormat, ...) LSA_LOG_WARNING(pszFormat, ## __VA_ARGS__)
#define VMDIRDB_LOG_INFO(pszFormat, ...) LSA_LOG_INFO(pszFormat, ## __VA_ARGS__)
#define VMDIRDB_LOG_VERBOSE(pszFormat, ...) LSA_LOG_VERBOSE(pszFormat, ## __VA_ARGS__)
#define VMDIRDB_LOG_DEBUG(pszFormat, ...) LSA_LOG_DEBUG(pszFormat, ## __VA_ARGS__)

#define BAIL_ON_VMDIRDB_ERROR(dwError) \
    BAIL_ON_LSA_ERROR((dwError))

#define BAIL_ON_VMDIRDB_SQLITE_ERROR(dwError, pszError)   \
    if (dwError) {                                      \
        VMDIRDB_LOG_DEBUG("Sqlite3 Error (code: %u): %s", \
                        dwError,                        \
                        LSA_SAFE_LOG_STRING(pszError)); \
        dwError = LW_ERROR_SAM_DATABASE_ERROR;         \
        goto error;                                     \
    }

#define BAIL_ON_VMDIRDB_SQLITE_ERROR_DB(dwError, pDb) \
    BAIL_ON_VMDIRDB_SQLITE_ERROR(dwError, sqlite3_errmsg(pDb))

#define BAIL_ON_VMDIRDB_SQLITE_ERROR_STMT(dwError, pStatement) \
    BAIL_ON_VMDIRDB_SQLITE_ERROR_DB(dwError, sqlite3_db_handle(pStatement))

#define VMDIRDB_LOCK_MUTEX(bInLock, mutex) \
    if (!bInLock) { \
       int thr_err = pthread_mutex_lock(mutex); \
       if (thr_err) { \
           VMDIRDB_LOG_ERROR("Failed to lock mutex. Aborting program"); \
           abort(); \
       } \
       bInLock = TRUE; \
    }

#define VMDIRDB_UNLOCK_MUTEX(bInLock, mutex) \
    if (bInLock) { \
       int thr_err = pthread_mutex_unlock(mutex); \
       if (thr_err) { \
           VMDIRDB_LOG_ERROR("Failed to unlock mutex. Aborting program"); \
           abort(); \
       } \
       bInLock = FALSE; \
    }

#define VMDIRDB_LOCK_RWMUTEX_SHARED(bInLock, mutex) \
    if (!bInLock) { \
       int thr_err = pthread_rwlock_rdlock(mutex); \
       if (thr_err) { \
           VMDIRDB_LOG_ERROR("Failed to acquire shared lock on rw mutex. Aborting program"); \
           abort(); \
       } \
       bInLock = TRUE; \
    }

#define VMDIRDB_LOCK_RWMUTEX_EXCLUSIVE(bInLock, mutex) \
    if (!bInLock) { \
       int thr_err = pthread_rwlock_wrlock(mutex); \
       if (thr_err) { \
           VMDIRDB_LOG_ERROR("Failed to acquire exclusive lock on rw mutex. Aborting program"); \
           abort(); \
       } \
       bInLock = TRUE; \
    }

#define VMDIRDB_UNLOCK_RWMUTEX(bInLock, mutex) \
    if (bInLock) { \
       int thr_err = pthread_rwlock_unlock(mutex); \
       if (thr_err) { \
           VMDIRDB_LOG_ERROR("Failed to unlock rw mutex. Aborting program"); \
           abort(); \
       } \
       bInLock = FALSE; \
    }

typedef enum
{
    VMDIR_BIND_PROTOCOL_UNSET,
    VMDIR_BIND_PROTOCOL_KERBEROS,
    VMDIR_BIND_PROTOCOL_SRP,
    VMDIR_BIND_PROTOCOL_SPNEGO
} VMDIRDB_BIND_PROTOCOL;

typedef enum
{
    VMDIRDB_ENTRY_TYPE_UNKNOWN = 0,
    VMDIRDB_ENTRY_TYPE_USER_OR_GROUP,
    VMDIRDB_ENTRY_TYPE_DOMAIN

} VMDIRDB_ENTRY_TYPE, *PVMDIRDB_ENTRY_TYPE;

#define ATTR_IS_MANDATORY     TRUE
#define ATTR_IS_NOT_MANDATORY FALSE
#define ATTR_IS_MUTABLE       TRUE
#define ATTR_IS_IMMUTABLE     FALSE

typedef enum
{
    VMDIRDB_DN_TOKEN_TYPE_UNKNOWN = 0,
    VMDIRDB_DN_TOKEN_TYPE_DC,
    VMDIRDB_DN_TOKEN_TYPE_CN,
    VMDIRDB_DN_TOKEN_TYPE_OU

} VMDIRDB_DN_TOKEN_TYPE;

typedef DWORD VMDIRDB_ACB, *PVMDIRDB_ACB;

#define VMDIRDB_ACB_DISABLED                 (0x00000001)
#define VMDIRDB_ACB_HOMDIRREQ                (0x00000002)
#define VMDIRDB_ACB_PWNOTREQ                 (0x00000004)
#define VMDIRDB_ACB_TEMPDUP                  (0x00000008)
#define VMDIRDB_ACB_NORMAL                   (0x00000010)
#define VMDIRDB_ACB_MNS                      (0x00000020)
#define VMDIRDB_ACB_DOMTRUST                 (0x00000040)
#define VMDIRDB_ACB_WSTRUST                  (0x00000080)
#define VMDIRDB_ACB_SVRTRUST                 (0x00000100)
#define VMDIRDB_ACB_PWNOEXP                  (0x00000200)
#define VMDIRDB_ACB_AUTOLOCK                 (0x00000400)
#define VMDIRDB_ACB_ENC_TXT_PWD_ALLOWED      (0x00000800)
#define VMDIRDB_ACB_SMARTCARD_REQUIRED       (0x00001000)
#define VMDIRDB_ACB_TRUSTED_FOR_DELEGATION   (0x00002000)
#define VMDIRDB_ACB_NOT_DELEGATED            (0x00004000)
#define VMDIRDB_ACB_USE_DES_KEY_ONLY         (0x00008000)
#define VMDIRDB_ACB_DONT_REQUIRE_PREAUTH     (0x00010000)
#define VMDIRDB_ACB_PW_EXPIRED               (0x00020000)
#define VMDIRDB_ACB_NO_AUTH_DATA_REQD        (0x00080000)

#define VMDIRDB_SECONDS_IN_HOUR (60 * 60)
#define VMDIRDB_SECONDS_IN_DAY  (24 * VMDIRDB_SECONDS_IN_HOUR)

#define VMDIRDB_MIN_PWD_AGE           (0)
#define VMDIRDB_MAX_PWD_AGE           (30 * VMDIRDB_SECONDS_IN_DAY)
#define VMDIRDB_PWD_PROMPT_TIME       (14 * VMDIRDB_SECONDS_IN_DAY)
#define VMDIRDB_LOCKOUT_THRESHOLD     (0)
#define VMDIRDB_LOCKOUT_DURATION      (0)
#define VMDIRDB_LOCKOUT_WINDOW        (0)


#endif /* __VMDIRDBDEFS_H__ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/