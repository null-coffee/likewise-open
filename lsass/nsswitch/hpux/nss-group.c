/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
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
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        nss-group.c
 *
 * Abstract:
 * 
 *        Name Server Switch (Likewise LSASS)
 * 
 *        Handle NSS Group Information
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Brian Koropoff (bkoropoff@likewisesoftware.com)
 *
 */

#include "lsanss.h"
#include "nss-group.h"

typedef struct
{
    nss_backend_t base;
    LSA_ENUMGROUPS_STATE enumGroupsState;
} LSA_NSS_GROUP_BACKEND, *PLSA_NSS_GROUP_BACKEND;

typedef NSS_STATUS (*NSS_ENTRYPOINT)(nss_backend_t*, void*);

static
NSS_STATUS
LsaNssHpuxGroupDestructor(
    nss_backend_t* pBackend,
    void* pArgs)
{
    PLSA_NSS_GROUP_BACKEND  pLsaBackend = (PLSA_NSS_GROUP_BACKEND) pBackend;
    PLSA_ENUMGROUPS_STATE   pEnumGroupsState = &pLsaBackend->enumGroupsState;
    int                     ret = NSS_STATUS_SUCCESS;   

    LsaNssClearEnumGroupsState(pEnumGroupsState);
    LsaFreeMemory(pBackend);

    return ret;
}

static
NSS_STATUS
LsaNssHpuxGroupSetgrent(
    nss_backend_t* pBackend,
    void* pArgs
    )
{
    PLSA_NSS_GROUP_BACKEND    pLsaBackend = (PLSA_NSS_GROUP_BACKEND) pBackend;
    PLSA_ENUMGROUPS_STATE     pEnumGroupsState = &pLsaBackend->enumGroupsState;

    return LsaNssCommonGroupSetgrent(pEnumGroupsState);
}

static
NSS_STATUS
LsaNssHpuxGroupGetgrent(
    nss_backend_t* pBackend,
    void* pArgs
    )
{
    PLSA_NSS_GROUP_BACKEND    pLsaBackend = (PLSA_NSS_GROUP_BACKEND) pBackend;
    PLSA_ENUMGROUPS_STATE     pEnumGroupsState = &pLsaBackend->enumGroupsState;
    nss_XbyY_args_t*          pXbyYArgs = (nss_XbyY_args_t*) pArgs;
    struct group*             pResultGroup = pXbyYArgs->buf.result;
    char *                    pszBuf = pXbyYArgs->buf.buffer;
    size_t                    bufLen = pXbyYArgs->buf.buflen;
    int                       err = 0;
    int*                      pErrorNumber = &err;
    int                       ret = NSS_STATUS_NOTFOUND;

    ret = LsaNssCommonGroupGetgrent(pEnumGroupsState,
                                    pResultGroup,
                                    pszBuf,
                                    bufLen,
                                    pErrorNumber);

    if (ret == NSS_STATUS_SUCCESS)
    {
        pXbyYArgs->returnval = pXbyYArgs->buf.result;
    }
    else if (ret == NSS_STATUS_TRYAGAIN  && err == ERANGE)
    {
        pXbyYArgs->erange = 1;
    }
    else
    {
        errno = err;
    }

    return ret;
}

static
NSS_STATUS
LsaNssHpuxGroupEndgrent(
    nss_backend_t* pBackend,
    void* pArgs
    )
{
    PLSA_NSS_GROUP_BACKEND    pLsaBackend = (PLSA_NSS_GROUP_BACKEND) pBackend;
    PLSA_ENUMGROUPS_STATE     pEnumGroupsState = &pLsaBackend->enumGroupsState;

    return LsaNssCommonGroupEndgrent(pEnumGroupsState);
}

static
NSS_STATUS
LsaNssHpuxGroupGetgrgid(
    nss_backend_t* pBackend,
    void* pArgs
    )
{
    nss_XbyY_args_t*          pXbyYArgs = (nss_XbyY_args_t*) pArgs;
    gid_t                     gid = pXbyYArgs->key.gid;
    struct group*             pResultGroup = pXbyYArgs->buf.result;
    char*                     pszBuf = pXbyYArgs->buf.buffer;
    size_t                    bufLen = pXbyYArgs->buf.buflen;
    int                       err = 0;
    int*                      pErrorNumber = &err;
    int                       ret = NSS_STATUS_SUCCESS;

    ret = LsaNssCommonGroupGetgrgid(gid,
                                    pResultGroup,
                                    pszBuf,
                                    bufLen,
                                    pErrorNumber);

    if (ret == NSS_STATUS_SUCCESS)
    {
        pXbyYArgs->returnval = pXbyYArgs->buf.result;
    }
    else if (ret == NSS_STATUS_TRYAGAIN  && err == ERANGE)
    {
        pXbyYArgs->erange = 1;
    }
    else
    {
        errno = err;
    }

    return ret;
}

NSS_STATUS
LsaNssHpuxGroupGetgrnam(
    nss_backend_t* pBackend,
    void* pArgs
    )
{
    nss_XbyY_args_t*          pXbyYArgs = (nss_XbyY_args_t*) pArgs;
    const char *              pszGroupName = pXbyYArgs->key.name;
    struct group *            pResultGroup = (struct group*) pXbyYArgs->buf.result;
    char *                    pszBuf = pXbyYArgs->buf.buffer;
    size_t                    bufLen = pXbyYArgs->buf.buflen;
    int                       err = 0;
    int*                      pErrorNumber = &err;
    int                       ret = NSS_STATUS_SUCCESS;

    ret = LsaNssCommonGroupGetgrnam(pszGroupName,
                                    pResultGroup,
                                    pszBuf,
                                    bufLen,
                                    pErrorNumber);

    if (ret == NSS_STATUS_SUCCESS)
    {
        pXbyYArgs->returnval = pXbyYArgs->buf.result;
    }
    else if (ret == NSS_STATUS_TRYAGAIN  && err == ERANGE)
    {
        pXbyYArgs->erange = 1;
    }
    else
    {
        errno = err;
    }

    return ret;
}

NSS_STATUS
LsaNssHpuxGroupGetgroupsbymember(
    nss_backend_t* pBackend,
    void* pArgs
    )
{
    struct nss_groupsbymem* pGroupsByMem = (struct nss_groupsbymem*) pArgs;
    PCSTR                   pszUserName = pGroupsByMem->username;
    int*                    pResultsSize = &pGroupsByMem->numgids;
    int*                    pResultsCapacity = &pGroupsByMem->maxgids;
    gid_t*                  pGidResults = pGroupsByMem->gid_array;
    int                     err = 0;
    int*                    pErrorNumber = &err;
    int                     ret = NSS_STATUS_SUCCESS;
    
    size_t myResultsSize = *pResultsSize;
    size_t myResultsCapacity = *pResultsCapacity;
    

    ret = LsaNssCommonGroupGetGroupsByUserName(
        pszUserName,
	myResultsSize,
        myResultsCapacity,
        &myResultsSize,
        pGidResults,
        pErrorNumber);

    if (ret == NSS_STATUS_SUCCESS)
    {
        if (myResultsSize > myResultsCapacity)
            myResultsSize = myResultsCapacity;

        *pResultsSize = (int) myResultsSize;
    }
  
    return ret;
}

static
NSS_ENTRYPOINT
LsaNssHpuxGroupOps[] =
{
    LsaNssHpuxGroupDestructor,
    LsaNssHpuxGroupEndgrent,
    LsaNssHpuxGroupSetgrent,
    LsaNssHpuxGroupGetgrent,
    LsaNssHpuxGroupGetgrnam,
    LsaNssHpuxGroupGetgrgid,
    LsaNssHpuxGroupGetgroupsbymember
};

static
nss_backend_t
LsaNssHpuxGroupBackend =
{
    .n_ops = 7,
    .ops = LsaNssHpuxGroupOps
};

nss_backend_t*
LsaNssHpuxGroupCreateBackend(
    void			       
    )
{
    PLSA_NSS_GROUP_BACKEND pLsaBackend = NULL;

    if (LsaAllocateMemory(sizeof(*pLsaBackend), (void**) &pLsaBackend))
    {
        return NULL;
    }

    pLsaBackend->base = LsaNssHpuxGroupBackend;

    return (nss_backend_t*) pLsaBackend;
}
