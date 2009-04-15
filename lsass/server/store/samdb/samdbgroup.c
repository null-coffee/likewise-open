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
 *        samdbgroup.c
 *
 * Abstract:
 *
 *
 *      Likewise SAM Database Provider
 *
 *      SAM Group Management
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *
 */

#include "includes.h"

static
DWORD
SamDbBuildGroupSearchSqlQuery(
    PSAM_DIRECTORY_CONTEXT pDirectoryContext,
    PCSTR                  pszSqlQueryTemplate,
    PWSTR                  wszAttributes[],
    PSTR*                  ppszQuery,
    PSAM_DB_COLUMN_VALUE*  ppColumnValueList
    );

static
DWORD
SamDbGroupSearchExecute(
    PSAM_DIRECTORY_CONTEXT pDirectoryContext,
    PCSTR                  pszQueryTemplate,
    PSAM_DB_COLUMN_VALUE   pColumnValueList,
    LONG64                 llObjectRecordId,
    PDIRECTORY_ENTRY*      ppDirectoryEntries,
    PDWORD                 pdwNumEntries
    );

DWORD
SamDbGetGroupCount(
    HANDLE hBindHandle,
    PDWORD pdwNumGroups
    )
{
    return SamDbGetObjectCount(
                hBindHandle,
                SAMDB_OBJECT_CLASS_GROUP,
                pdwNumGroups);
}

DWORD
SamDbGetGroupMembers(
    HANDLE            hBindHandle,
    PWSTR             pwszGroupDN,
    PWSTR             pwszAttrs[],
    PDIRECTORY_ENTRY* ppDirectoryEntries,
    PDWORD            pdwNumEntries
    )
{
    DWORD dwError = 0;
    PCSTR pszSqlQueryTemplate = \
            "  FROM " SAM_DB_OBJECTS_TABLE " sdo" \
            " WHERE sdo." SAM_DB_COL_RECORD_ID \
            "    IN (SELECT " SAM_DB_COL_MEMBER_RECORD_ID \
            "          FROM " SAM_DB_MEMBERS_TABLE "sdm" \
            "         WHERE sdm." SAM_DB_COL_GROUP_RECORD_ID " = ?1);";
    PSAM_DIRECTORY_CONTEXT pDirectoryContext = NULL;
    PSAM_DB_COLUMN_VALUE pColumnValueList = NULL;
    PDIRECTORY_ENTRY     pDirectoryEntries = NULL;
    DWORD   dwNumEntries = 0;
    LONG64  llObjectRecordId = 0;
    PSTR    pszGroupDN = NULL;
    PSTR    pszSqlQuery = NULL;
    BOOLEAN bInLock = FALSE;

    pDirectoryContext = (PSAM_DIRECTORY_CONTEXT)hBindHandle;

    dwError = LsaWc16sToMbs(
                    pwszGroupDN,
                    &pszGroupDN);
    BAIL_ON_SAMDB_ERROR(dwError);

    SAMDB_LOCK_RWMUTEX_SHARED(bInLock, &pDirectoryContext->rwLock);

    dwError = SamDbGetObjectRecordId_inlock(
                    pDirectoryContext,
                    SAMDB_OBJECT_CLASS_GROUP,
                    pszGroupDN,
                    &llObjectRecordId);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwError = SamDbBuildGroupSearchSqlQuery(
                    pDirectoryContext,
                    pszSqlQueryTemplate,
                    pwszAttrs,
                    &pszSqlQuery,
                    &pColumnValueList);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwError = SamDbGroupSearchExecute(
                    pDirectoryContext,
                    pszSqlQuery,
                    pColumnValueList,
                    llObjectRecordId,
                    &pDirectoryEntries,
                    &dwNumEntries);
    BAIL_ON_SAMDB_ERROR(dwError);

    *ppDirectoryEntries = pDirectoryEntries;
    *pdwNumEntries = dwNumEntries;

cleanup:

    if (pColumnValueList)
    {
        SamDbFreeColumnValueList(pColumnValueList);
    }

    SAMDB_UNLOCK_RWMUTEX(bInLock, &pDirectoryContext->rwLock);

    LSA_SAFE_FREE_STRING(pszGroupDN);
    LSA_SAFE_FREE_STRING(pszSqlQuery);

    return dwError;

error:

    *ppDirectoryEntries = NULL;
    *pdwNumEntries = 0;

    if (pDirectoryEntries)
    {
        DirectoryFreeEntries(pDirectoryEntries, dwNumEntries);
    }

    goto cleanup;
}

DWORD
SamDbGetUserMemberships(
    HANDLE            hBindHandle,
    PWSTR             pwszUserDN,
    PWSTR             pwszAttrs[],
    PDIRECTORY_ENTRY* ppDirectoryEntries,
    PDWORD            pdwNumEntries
    )
{
    DWORD dwError = 0;
    PCSTR pszSqlQueryTemplate = \
            "  FROM " SAM_DB_OBJECTS_TABLE " sdo" \
            " WHERE sdo." SAM_DB_COL_RECORD_ID \
            "    IN (SELECT " SAM_DB_COL_GROUP_RECORD_ID \
            "          FROM " SAM_DB_MEMBERS_TABLE "sdm" \
            "         WHERE sdm." SAM_DB_COL_MEMBER_RECORD_ID " = ?1);";
    PSAM_DIRECTORY_CONTEXT pDirectoryContext = NULL;
    PSAM_DB_COLUMN_VALUE pColumnValueList = NULL;
    PDIRECTORY_ENTRY     pDirectoryEntries = NULL;
    DWORD   dwNumEntries = 0;
    LONG64  llObjectRecordId = 0;
    PSTR    pszUserDN = NULL;
    PSTR    pszSqlQuery = NULL;
    BOOLEAN bInLock = FALSE;

    pDirectoryContext = (PSAM_DIRECTORY_CONTEXT)hBindHandle;

    dwError = LsaWc16sToMbs(
                    pwszUserDN,
                    &pszUserDN);
    BAIL_ON_SAMDB_ERROR(dwError);

    SAMDB_LOCK_RWMUTEX_SHARED(bInLock, &pDirectoryContext->rwLock);

    dwError = SamDbGetObjectRecordId_inlock(
                    pDirectoryContext,
                    SAMDB_OBJECT_CLASS_USER,
                    pszUserDN,
                    &llObjectRecordId);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwError = SamDbBuildGroupSearchSqlQuery(
                    pDirectoryContext,
                    pszSqlQueryTemplate,
                    pwszAttrs,
                    &pszSqlQuery,
                    &pColumnValueList);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwError = SamDbGroupSearchExecute(
                    pDirectoryContext,
                    pszSqlQuery,
                    pColumnValueList,
                    llObjectRecordId,
                    &pDirectoryEntries,
                    &dwNumEntries);
    BAIL_ON_SAMDB_ERROR(dwError);

    *ppDirectoryEntries = pDirectoryEntries;
    *pdwNumEntries = dwNumEntries;

cleanup:

    if (pColumnValueList)
    {
        SamDbFreeColumnValueList(pColumnValueList);
    }

    SAMDB_UNLOCK_RWMUTEX(bInLock, &pDirectoryContext->rwLock);

    LSA_SAFE_FREE_STRING(pszUserDN);
    LSA_SAFE_FREE_STRING(pszSqlQuery);

    return dwError;

error:

    *ppDirectoryEntries = NULL;
    *pdwNumEntries = 0;

    if (pDirectoryEntries)
    {
        DirectoryFreeEntries(pDirectoryEntries, dwNumEntries);
    }

    goto cleanup;
}

#define SAM_DB_GROUP_SEARCH_QUERY_PREFIX          "SELECT "
#define SAM_DB_GROUP_SEARCH_QUERY_FIELD_SEPARATOR ","

static
DWORD
SamDbBuildGroupSearchSqlQuery(
    PSAM_DIRECTORY_CONTEXT pDirectoryContext,
    PCSTR                  pszSqlQueryTemplate,
    PWSTR                  wszAttributes[],
    PSTR*                  ppszQuery,
    PSAM_DB_COLUMN_VALUE*  ppColumnValueList
    )
{
    DWORD dwError = 0;
    PSTR  pszQuery = NULL;
    PSTR  pszQueryCursor = NULL;
    PCSTR pszCursor = NULL;
    PSAM_DB_COLUMN_VALUE pColumnValueList = NULL;
    PSAM_DB_COLUMN_VALUE pIter = NULL;
    DWORD dwNumAttrs = 0;
    DWORD dwColNamesLen = 0;
    DWORD dwQueryLen = 0;

    while (wszAttributes[dwNumAttrs])
    {
        PWSTR pwszAttrName = wszAttributes[dwNumAttrs];

        PSAM_DB_COLUMN_VALUE  pColumnValue = NULL;
        PSAM_DB_ATTRIBUTE_MAP pAttrMap = NULL;

        dwError = SamDbAttributeLookupByName(
                        pDirectoryContext->pAttrLookup,
                        pwszAttrName,
                        &pAttrMap);
        BAIL_ON_SAMDB_ERROR(dwError);

        if (!pAttrMap->bIsQueryable)
        {
            dwError = LSA_ERROR_INVALID_PARAMETER;
            BAIL_ON_SAMDB_ERROR(dwError);
        }

        dwError = DirectoryAllocateMemory(
                        sizeof(SAM_DB_COLUMN_VALUE),
                        (PVOID*)&pColumnValue);
        BAIL_ON_SAMDB_ERROR(dwError);

        pColumnValue->pAttrMap = pAttrMap;
        pColumnValue->pNext = pColumnValueList;
        pColumnValueList = pColumnValue;
        pColumnValue = NULL;

        if (dwColNamesLen)
        {
            dwColNamesLen += sizeof(SAM_DB_GROUP_SEARCH_QUERY_FIELD_SEPARATOR)-1;
        }

        dwColNamesLen += strlen(&pAttrMap->szDbColumnName[0]);

        dwNumAttrs++;
    }

    dwQueryLen = sizeof(SAM_DB_GROUP_SEARCH_QUERY_PREFIX) - 1;
    dwQueryLen += dwColNamesLen;
    dwQueryLen += strlen(pszSqlQueryTemplate);
    dwQueryLen++;

    dwError = DirectoryAllocateMemory(
                    dwQueryLen,
                    (PVOID*)&pszQuery);
    BAIL_ON_SAMDB_ERROR(dwError);

    pColumnValueList = SamDbReverseColumnValueList(pColumnValueList);

    pszQueryCursor = pszQuery;
    dwColNamesLen = 0;

    pszCursor = SAM_DB_GROUP_SEARCH_QUERY_PREFIX;
    while (pszCursor && *pszCursor)
    {
        *pszQueryCursor++ = *pszCursor++;
    }

    for (pIter = pColumnValueList; pIter; pIter = pIter->pNext)
    {
        if (dwColNamesLen)
        {
            pszCursor = SAM_DB_GROUP_SEARCH_QUERY_FIELD_SEPARATOR;
            while (pszCursor && *pszCursor)
            {
                *pszQueryCursor++ = *pszCursor++;
                dwColNamesLen++;
            }
        }

        pszCursor = &pIter->pAttrMap->szDbColumnName[0];
        while (pszCursor && *pszCursor)
        {
            *pszQueryCursor++ = *pszCursor++;
            dwColNamesLen++;
        }
    }

    pszCursor = pszSqlQueryTemplate;
    while (pszCursor && *pszCursor)
    {
        *pszQueryCursor++ = *pszCursor++;
    }

    *ppszQuery = pszQuery;
    *ppColumnValueList = pColumnValueList;

cleanup:

    return dwError;

error:

    *ppszQuery = NULL;
    *ppColumnValueList = NULL;

    LSA_SAFE_FREE_STRING(pszQuery);

    if (pColumnValueList)
    {
        SamDbFreeColumnValueList(pColumnValueList);
    }

    goto cleanup;
}

static
DWORD
SamDbGroupSearchExecute(
    PSAM_DIRECTORY_CONTEXT pDirectoryContext,
    PCSTR                  pszQueryTemplate,
    PSAM_DB_COLUMN_VALUE   pColumnValueList,
    LONG64                 llObjectRecordId,
    PDIRECTORY_ENTRY*      ppDirectoryEntries,
    PDWORD                 pdwNumEntries
    )
{
    DWORD                dwError = 0;
    PDIRECTORY_ENTRY     pDirectoryEntries = NULL;
    DWORD                dwNumEntries = 0;
    DWORD                dwTotalEntries = 0;
    DWORD                dwEntriesAvailable = 0;
    sqlite3_stmt*        pSqlStatement = NULL;
    DWORD                dwNumCols = 0;
    PSAM_DB_COLUMN_VALUE pIter = NULL;
    PDIRECTORY_ATTRIBUTE pAttrs = NULL;
    DWORD                dwNumAttrs = 0;

    for (pIter = pColumnValueList; pIter; pIter = pIter->pNext)
    {
        dwNumCols++;
    }

    dwError = sqlite3_prepare_v2(
                    pDirectoryContext->pDbContext->pDbHandle,
                    pszQueryTemplate,
                    -1,
                    &pSqlStatement,
                    NULL);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwError = sqlite3_bind_int64(
                    pSqlStatement,
                    1,
                    llObjectRecordId);
    BAIL_ON_SAMDB_ERROR(dwError);

    while ((dwError = sqlite3_step(pSqlStatement)) == SQLITE_ROW)
    {
        DWORD iCol = 0;

        dwNumAttrs = sqlite3_column_count(pSqlStatement);
        if (dwNumAttrs != dwNumCols)
        {
            dwError = LSA_ERROR_DATA_ERROR;
            BAIL_ON_SAMDB_ERROR(dwError);
        }

        if (!dwEntriesAvailable)
        {
            DWORD dwNewEntryCount = dwTotalEntries + 5;

            dwError = DirectoryReallocMemory(
                            pDirectoryEntries,
                            (PVOID*)&pDirectoryEntries,
                            dwNewEntryCount * sizeof(DIRECTORY_ENTRY));
            BAIL_ON_SAMDB_ERROR(dwError);

            dwEntriesAvailable = dwNewEntryCount - dwTotalEntries;

            memset(pDirectoryEntries+(dwTotalEntries * sizeof(DIRECTORY_ENTRY)),
                   0,
                   dwEntriesAvailable * sizeof(DIRECTORY_ENTRY));

            dwTotalEntries = dwNewEntryCount;
        }

        dwError = DirectoryAllocateMemory(
                        sizeof(DIRECTORY_ATTRIBUTE) * dwNumAttrs,
                        (PVOID*)&pAttrs);
        BAIL_ON_SAMDB_ERROR(dwError);

        for (pIter = pColumnValueList; pIter; pIter = pIter->pNext, iCol++)
        {
            PDIRECTORY_ATTRIBUTE pAttr = &pAttrs[iCol];
            DWORD dwAttrLen = 0;

            dwError = DirectoryAllocateStringW(
                            pIter->pAttrMap->wszDirectoryAttribute,
                            &pAttr->pwszName);
            BAIL_ON_SAMDB_ERROR(dwError);

            switch (pIter->pAttrMap->attributeType)
            {
                case SAMDB_ATTR_TYPE_TEXT:

                    dwAttrLen = sqlite3_column_bytes(pSqlStatement, iCol);
                    if (dwAttrLen)
                    {
                        PATTRIBUTE_VALUE pAttrVal = NULL;

                        const unsigned char* pszStringVal = NULL;

                        pszStringVal = sqlite3_column_text(
                                            pSqlStatement,
                                            iCol);

                        dwError = DirectoryAllocateMemory(
                                        sizeof(ATTRIBUTE_VALUE),
                                        (PVOID*)&pAttr->pValues);
                        BAIL_ON_SAMDB_ERROR(dwError);

                        pAttr->ulNumValues = 1;

                        pAttrVal = &pAttr->pValues[0];

                        pAttrVal->Type = DIRECTORY_ATTR_TYPE_UNICODE_STRING;

                        dwError = LsaMbsToWc16s(
                                        (PCSTR)pszStringVal,
                                        &pAttrVal->data.pwszStringValue);
                        BAIL_ON_SAMDB_ERROR(dwError);
                    }
                    else
                    {
                        pAttr->ulNumValues = 0;
                    }

                    break;

                case SAMDB_ATTR_TYPE_INT32:
                case SAMDB_ATTR_TYPE_DATETIME:

                    dwError = DirectoryAllocateMemory(
                            sizeof(ATTRIBUTE_VALUE),
                            (PVOID*)&pAttr->pValues);
                    BAIL_ON_SAMDB_ERROR(dwError);

                    pAttr->ulNumValues = 1;

                    pAttr->pValues[0].Type = DIRECTORY_ATTR_TYPE_INTEGER;
                    pAttr->pValues[0].data.ulValue = sqlite3_column_int(
                                                        pSqlStatement,
                                                        iCol);

                    break;

                case SAMDB_ATTR_TYPE_INT64:

                    dwError = DirectoryAllocateMemory(
                                    sizeof(ATTRIBUTE_VALUE),
                                    (PVOID*)&pAttr->pValues);
                    BAIL_ON_SAMDB_ERROR(dwError);

                    pAttr->ulNumValues = 1;

                    pAttr->pValues[0].Type = DIRECTORY_ATTR_TYPE_LARGE_INTEGER;
                    pAttr->pValues[0].data.llValue = sqlite3_column_int64(
                                                        pSqlStatement,
                                                        iCol);

                    break;

                case SAMDB_ATTR_TYPE_BOOLEAN:

                    dwError = DirectoryAllocateMemory(
                                    sizeof(ATTRIBUTE_VALUE),
                                    (PVOID*)&pAttr->pValues);
                    BAIL_ON_SAMDB_ERROR(dwError);

                    pAttr->ulNumValues = 1;

                    pAttr->pValues[0].Type = DIRECTORY_ATTR_TYPE_BOOLEAN;

                    if (sqlite3_column_int(pSqlStatement, iCol))
                    {
                        pAttr->pValues[0].data.bBooleanValue = TRUE;
                    }
                    else
                    {
                        pAttr->pValues[0].data.bBooleanValue = FALSE;
                    }

                    break;

                case SAMDB_ATTR_TYPE_BLOB:

                    dwAttrLen = sqlite3_column_bytes(pSqlStatement, iCol);
                    if (dwAttrLen)
                    {
                        PATTRIBUTE_VALUE pAttrVal = NULL;

                        PCVOID pBlob = sqlite3_column_blob(
                                                pSqlStatement,
                                                iCol);

                        dwError = DirectoryAllocateMemory(
                                        sizeof(ATTRIBUTE_VALUE),
                                        (PVOID*)&pAttr->pValues);
                        BAIL_ON_SAMDB_ERROR(dwError);

                        pAttr->ulNumValues = 1;

                        pAttrVal = &pAttr->pValues[0];

                        pAttrVal->Type = DIRECTORY_ATTR_TYPE_OCTET_STREAM;

                        dwError = DirectoryAllocateMemory(
                                    sizeof(OCTET_STRING),
                                    (PVOID*)&pAttrVal->data.pOctetString);
                        BAIL_ON_SAMDB_ERROR(dwError);

                        dwError = DirectoryAllocateMemory(
                                    dwAttrLen,
                                    (PVOID*)&pAttrVal->data.pOctetString->pBytes);
                        BAIL_ON_SAMDB_ERROR(dwError);

                        memcpy(pAttrVal->data.pOctetString->pBytes,
                                pBlob,
                                dwAttrLen);

                        pAttrVal->data.pOctetString->ulNumBytes = dwAttrLen;
                    }
                    else
                    {
                        pAttr->ulNumValues = 0;
                    }

                    break;

                default:

                    dwError = LSA_ERROR_INTERNAL;
                    BAIL_ON_SAMDB_ERROR(dwError);
            }
        }

        pDirectoryEntries[dwNumEntries].ulNumAttributes = dwNumAttrs;
        pDirectoryEntries[dwNumEntries].pAttributes = pAttrs;

        pAttrs = NULL;
        dwNumAttrs = 0;

        dwNumEntries++;
        dwEntriesAvailable--;
    }

    if (dwError == SQLITE_DONE)
    {
        dwError = LSA_ERROR_SUCCESS;
    }
    BAIL_ON_SAMDB_ERROR(dwError);

    *ppDirectoryEntries = pDirectoryEntries;
    *pdwNumEntries = dwNumEntries;

cleanup:

    if (pSqlStatement)
    {
        sqlite3_finalize(pSqlStatement);
    }

    return dwError;

error:

    *ppDirectoryEntries = NULL;
    *pdwNumEntries = 0;

    if (pDirectoryEntries)
    {
        DirectoryFreeEntries(pDirectoryEntries, dwTotalEntries);
    }

    goto cleanup;
}

DWORD
SamDbAddToGroup(
    HANDLE hBindHandle,
    PWSTR  pwszGroupDN,
    PWSTR  pwszMemberDN
    )
{
    DWORD dwError = 0;

    // TODO:

    return dwError;
}

DWORD
SamDbRemoveFromGroup(
    HANDLE hBindHandle,
    PWSTR  pwszGroupDN,
    PWSTR  pwszMemberDN
    )
{
    DWORD dwError = 0;

    // TODO:

    return dwError;
}

