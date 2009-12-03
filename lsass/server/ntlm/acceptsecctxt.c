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
 *        acceptsecctxt.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        AcceptSecurityContext client wrapper API
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Marc Guy (mguy@likewisesoftware.com)
 */

#include "ntlmsrvapi.h"
#include "lsasrvapi.h"

DWORD
NtlmServerAcceptSecurityContext(
    IN HANDLE Handle,
    IN NTLM_CRED_HANDLE hCred,
    IN OUT PNTLM_CONTEXT_HANDLE phContext,
    IN const SecBuffer* pInput,
    IN DWORD fContextReq,
    IN DWORD TargetDataRep,
    IN OUT PNTLM_CONTEXT_HANDLE phNewContext,
    OUT PSecBuffer pOutput,
    OUT PDWORD  pfContextAttr,
    OUT PTimeStamp ptsTimeStamp
    )
{
    DWORD dwError = LW_ERROR_SUCCESS;
    PNTLM_CONTEXT pNtlmContext = NULL;
    PNTLM_CONTEXT pNtlmCtxtOut = NULL;
    PNTLM_CONTEXT pNtlmCtxtChlng = NULL;
    NTLM_CONTEXT_HANDLE ContextHandle = NULL;
    const NTLM_NEGOTIATE_MESSAGE_V1* pNegMsg = NULL;
    PNTLM_RESPONSE_MESSAGE_V1 pRespMsg = NULL;
    DWORD dwMessageSize = 0;
    BYTE SessionKey[NTLM_SESSION_KEY_SIZE] = {0};

    ptsTimeStamp = 0;

    pOutput->pvBuffer = NULL;

    if (phContext)
    {
        pNtlmContext = *phContext;
    }

    if (!pNtlmContext)
    {
        if (pInput->BufferType != SECBUFFER_TOKEN ||
           pInput->cbBuffer == 0)
        {
            dwError = LW_ERROR_INVALID_PARAMETER;
            BAIL_ON_LSA_ERROR(dwError);
        }

        pNegMsg = pInput->pvBuffer;
        dwMessageSize = pInput->cbBuffer;

        dwError = NtlmCreateChallengeContext(
            pNegMsg,
            hCred,
            &pNtlmCtxtOut,
            pOutput);
        BAIL_ON_LSA_ERROR(dwError);

        dwError = LW_WARNING_CONTINUE_NEEDED;
    }
    else
    {
        // The only time we should get a context handle passed in is when
        // we are validating a challenge and we need to look up the original
        // challenge sent
        pNtlmCtxtChlng = pNtlmContext;

        // In this case we need to grab the response message sent in
        if (pInput->BufferType != SECBUFFER_TOKEN ||
           pInput->cbBuffer == 0)
        {
            dwError = LW_ERROR_INVALID_PARAMETER;
            BAIL_ON_LSA_ERROR(dwError);
        }

        pRespMsg = pInput->pvBuffer;
        dwMessageSize = pInput->cbBuffer;

        dwError = NtlmValidateResponse(
            Handle,
            pRespMsg,
            dwMessageSize,
            pNtlmCtxtChlng,
            SessionKey);
        BAIL_ON_LSA_ERROR(dwError);

        // The response has been validated and contains all the information we
        // are looking for; retain it... the original (challenge) context will
        // be freed when we return.
        dwError = NtlmCreateValidatedContext(
            pRespMsg,
            dwMessageSize,
            pNtlmCtxtChlng->NegotiatedFlags,
            SessionKey,
            NTLM_SESSION_KEY_SIZE,
            hCred,
            &pNtlmCtxtOut
            );
        BAIL_ON_LSA_ERROR(dwError);
    }

    ContextHandle = pNtlmCtxtOut;

cleanup:
    *phNewContext = ContextHandle;

    return(dwError);
error:
    LW_SAFE_FREE_MEMORY(pOutput->pvBuffer);
    pOutput->cbBuffer = 0;
    pOutput->BufferType = 0;
    if (ContextHandle)
    {
        NtlmReleaseContext(&ContextHandle);
    }
    else if (pNtlmCtxtOut)
    {
        NtlmReleaseContext(&pNtlmCtxtOut);
    }
    goto cleanup;
}

DWORD
NtlmCreateChallengeContext(
    IN const NTLM_NEGOTIATE_MESSAGE_V1* pNtlmNegMsg,
    IN NTLM_CRED_HANDLE hCred,
    OUT PNTLM_CONTEXT *ppNtlmContext,
    OUT PSecBuffer pOutput
    )
{
    DWORD dwError = LW_ERROR_SUCCESS;
    PNTLM_CONTEXT pNtlmContext = NULL;
    DWORD dwMessageSize = 0;
    PNTLM_CHALLENGE_MESSAGE pMessage = NULL;
    PSTR pServerName = NULL;
    PSTR pDomainName = NULL;
    PSTR pDnsServerName = NULL;
    PSTR pDnsDomainName = NULL;

    *ppNtlmContext = NULL;

    dwError = NtlmCreateContext(hCred, &pNtlmContext);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = NtlmGetNameInformation(
        &pServerName,
        &pDomainName,
        &pDnsServerName,
        &pDnsDomainName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = NtlmGetRandomBuffer(
        pNtlmContext->Challenge,
        NTLM_CHALLENGE_SIZE
        );
    BAIL_ON_LSA_ERROR(dwError);

    dwError = NtlmCreateChallengeMessage(
        pNtlmNegMsg,
        pServerName,
        pDomainName,
        pDnsServerName,
        pDnsDomainName,
        (PBYTE)&gW2KSpoof,
        pNtlmContext->Challenge,
        &dwMessageSize,
        &pMessage
        );

    BAIL_ON_LSA_ERROR(dwError);

    pNtlmContext->NegotiatedFlags = pMessage->NtlmFlags;
    pOutput->cbBuffer = dwMessageSize;
    pOutput->BufferType = SECBUFFER_TOKEN;
    pOutput->pvBuffer = pMessage;
    pNtlmContext->NtlmState = NtlmStateChallenge;

cleanup:
    *ppNtlmContext = pNtlmContext;
    LW_SAFE_FREE_STRING(pServerName);
    LW_SAFE_FREE_STRING(pDomainName);
    LW_SAFE_FREE_STRING(pDnsServerName);
    LW_SAFE_FREE_STRING(pDnsDomainName);
    return dwError;

error:
    LW_SAFE_FREE_MEMORY(pMessage);

    if (pNtlmContext)
    {
        NtlmReleaseContext(&pNtlmContext);
        *ppNtlmContext = NULL;
    }
    pOutput->cbBuffer = 0;
    pOutput->BufferType = 0;
    pOutput->pvBuffer = NULL;
    goto cleanup;
}

DWORD
NtlmCreateValidatedContext(
    IN PNTLM_RESPONSE_MESSAGE_V1 pNtlmRespMsg,
    IN DWORD dwMsgSize,
    IN DWORD NegotiatedFlags,
    IN PBYTE pSessionKey,
    IN DWORD dwSessionKeyLen,
    IN NTLM_CRED_HANDLE hCred,
    OUT PNTLM_CONTEXT *ppNtlmContext
    )
{
    DWORD dwError = LW_ERROR_SUCCESS;
    PNTLM_CONTEXT pNtlmContext = NULL;
    SEC_CHAR* pUserName = NULL;
    SEC_CHAR* pDomainName = NULL;

    *ppNtlmContext = NULL;

    dwError = NtlmCreateContext(hCred, &pNtlmContext);
    BAIL_ON_LSA_ERROR(dwError);

    pNtlmContext->NtlmState = NtlmStateResponse;

    pNtlmContext->NegotiatedFlags = NegotiatedFlags;

    dwError = NtlmGetUserNameFromResponse(
        pNtlmRespMsg,
        NegotiatedFlags & NTLM_FLAG_UNICODE,
        &pUserName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = NtlmGetDomainNameFromResponse(
        pNtlmRespMsg,
        NegotiatedFlags & NTLM_FLAG_UNICODE,
        &pDomainName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = LwAllocateStringPrintf(
        &pNtlmContext->pszClientUsername,
        "%s\\%s",
        pDomainName,
        pUserName);
    BAIL_ON_LSA_ERROR(dwError);

    memcpy(pNtlmContext->SessionKey, pSessionKey, NTLM_SESSION_KEY_SIZE);
    pNtlmContext->cbSessionKeyLen = dwSessionKeyLen;
    pNtlmContext->bInitiatedSide = FALSE;

    dwError = NtlmInitializeKeys(pNtlmContext);
    BAIL_ON_LSA_ERROR(dwError);

cleanup:
    LW_SAFE_FREE_MEMORY(pUserName);
    LW_SAFE_FREE_MEMORY(pDomainName);
    *ppNtlmContext = pNtlmContext;
    return dwError;

error:
    if (pNtlmContext)
    {
        NtlmFreeContext(&pNtlmContext);
    }
    goto cleanup;
}

DWORD
NtlmCreateSubkey(
    DWORD dwMasterKeyLen,
    PBYTE pMasterKey,
    PCSTR pszSubkeyMagic,
    RC4_KEY** ppResult
    )
{
    DWORD dwError = ERROR_SUCCESS;
    RC4_KEY* pResult = NULL;
    BYTE md5Result[MD5_DIGEST_LENGTH];
    MD5_CTX ctx = {0};

    dwError = LwAllocateMemory(
        sizeof(*pResult),
        OUT_PPVOID(&pResult));
    BAIL_ON_LSA_ERROR(dwError);

    MD5_Init(&ctx);
    MD5_Update(
        &ctx,
        pMasterKey,
        dwMasterKeyLen);
    MD5_Update(
        &ctx,
        pszSubkeyMagic,
        strlen(pszSubkeyMagic) + 1); //Include the null
    MD5_Final(md5Result, &ctx);

    RC4_set_key(
        pResult,
        sizeof(md5Result),
        md5Result);

    *ppResult = pResult;

cleanup:
    memset(md5Result, 0, sizeof(md5Result));
    return dwError;

error:
    *ppResult = NULL;
    LW_SAFE_FREE_MEMORY(pResult);
    goto cleanup;
}

DWORD
NtlmInitializeKeys(
    PNTLM_CONTEXT pNtlmContext
    )
{
    MD5_CTX ctx = {0};
    DWORD dwError = 0;
    const char *clientServerSign = "session key to client-to-server signing key magic constant";
    const char *serverClientSign = "session key to server-to-client signing key magic constant";
    const char *clientServerSeal = "session key to client-to-server sealing key magic constant";
    const char *serverClientSeal = "session key to server-to-client sealing key magic constant";

    if (pNtlmContext->NegotiatedFlags & NTLM_FLAG_NTLM2)
    {
        dwError = LwAllocateMemory(
            sizeof(*pNtlmContext->pdwSendMsgSeq),
            OUT_PPVOID(&pNtlmContext->pdwSendMsgSeq));
        BAIL_ON_LSA_ERROR(dwError);
        dwError = LwAllocateMemory(
            sizeof(*pNtlmContext->pdwRecvMsgSeq),
            OUT_PPVOID(&pNtlmContext->pdwRecvMsgSeq));
        BAIL_ON_LSA_ERROR(dwError);

        MD5_Init(&ctx);
        MD5_Update(
            &ctx,
            pNtlmContext->SessionKey,
            pNtlmContext->cbSessionKeyLen);
        MD5_Update(
            &ctx,
            clientServerSign,
            strlen(clientServerSign) + 1); //Include the null
        MD5_Final(
            pNtlmContext->bInitiatedSide ?
                pNtlmContext->SignKey : pNtlmContext->VerifyKey,
            &ctx);

        MD5_Init(&ctx);
        MD5_Update(
            &ctx,
            pNtlmContext->SessionKey,
            pNtlmContext->cbSessionKeyLen);
        MD5_Update(
            &ctx,
            serverClientSign,
            strlen(serverClientSign) + 1); //Include the null
        MD5_Final(
            pNtlmContext->bInitiatedSide ?
                pNtlmContext->VerifyKey : pNtlmContext->SignKey,
            &ctx);

        dwError = NtlmCreateSubkey(
            pNtlmContext->cbSessionKeyLen,
            pNtlmContext->SessionKey,
            clientServerSeal,
            pNtlmContext->bInitiatedSide ?
                &pNtlmContext->pSealKey : &pNtlmContext->pUnsealKey
            );
        BAIL_ON_LSA_ERROR(dwError);

        dwError = NtlmCreateSubkey(
            pNtlmContext->cbSessionKeyLen,
            pNtlmContext->SessionKey,
            serverClientSeal,
            pNtlmContext->bInitiatedSide ?
                &pNtlmContext->pUnsealKey : &pNtlmContext->pSealKey
            );
        BAIL_ON_LSA_ERROR(dwError);
    }
    else
    {
        dwError = LwAllocateMemory(
            sizeof(*pNtlmContext->pdwSendMsgSeq),
            OUT_PPVOID(&pNtlmContext->pdwSendMsgSeq));
        BAIL_ON_LSA_ERROR(dwError);
        pNtlmContext->pdwRecvMsgSeq = pNtlmContext->pdwSendMsgSeq;

        dwError = LwAllocateMemory(
            sizeof(*pNtlmContext->pSealKey),
            OUT_PPVOID(&pNtlmContext->pSealKey));
        BAIL_ON_LSA_ERROR(dwError);

        RC4_set_key(
            pNtlmContext->pSealKey,
            pNtlmContext->cbSessionKeyLen,
            pNtlmContext->SessionKey);

        pNtlmContext->pUnsealKey = pNtlmContext->pSealKey;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
NtlmValidateResponse(
    IN HANDLE Handle,
    IN PNTLM_RESPONSE_MESSAGE_V1 pRespMsg,
    IN DWORD dwRespMsgSize,
    IN PNTLM_CONTEXT pChlngCtxt,
    OUT BYTE pSessionKey[NTLM_SESSION_KEY_SIZE]
    )
{
    DWORD dwError = LW_ERROR_SUCCESS;
    LSA_AUTH_USER_PARAMS Params;
    PLSA_AUTH_USER_INFO pUserInfo = NULL;
    PBYTE pLMRespBuffer = NULL;
    PBYTE pNTRespBuffer = NULL;
    LW_LSA_DATA_BLOB Challenge;
    LW_LSA_DATA_BLOB LMResp;
    LW_LSA_DATA_BLOB NTResp;
    PSTR pUserName = NULL;
    PSTR pDomainName = NULL;
    PSTR pWorkstation = NULL;
    BYTE sessionNonce[MD5_DIGEST_LENGTH];
    BYTE sessionHashUntrunc[MD5_DIGEST_LENGTH];

    memset(&Params, 0, sizeof(Params));
    memset(&Challenge, 0, sizeof(Challenge));
    memset(&LMResp, 0, sizeof(LMResp));
    memset(&NTResp, 0, sizeof(NTResp));

    // sanity check
    if (!pRespMsg || ! pChlngCtxt)
    {
        dwError = LW_ERROR_INVALID_PARAMETER;
        BAIL_ON_LSA_ERROR(dwError);
    }

    dwError = LwAllocateMemory(
        pRespMsg->LmResponse.usLength,
        OUT_PPVOID(&pLMRespBuffer));
    BAIL_ON_LSA_ERROR(dwError);

    dwError = LwAllocateMemory(
        pRespMsg->NtResponse.usLength,
        OUT_PPVOID(&pNTRespBuffer));
    BAIL_ON_LSA_ERROR(dwError);

    // The username, domain, and workstation values might come back as Unicode.
    // We could technically prevent this by not allowing NTLM_FLAG_UNICODE to be
    // set during the negotiation phase, but that seems like an odd restriction
    // for now.

    dwError = NtlmGetUserNameFromResponse(
        pRespMsg,
        pChlngCtxt->NegotiatedFlags & NTLM_FLAG_UNICODE,
        &pUserName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = NtlmGetDomainNameFromResponse(
        pRespMsg,
        pChlngCtxt->NegotiatedFlags & NTLM_FLAG_UNICODE,
        &pDomainName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = NtlmGetWorkstationFromResponse(
        pRespMsg,
        pChlngCtxt->NegotiatedFlags & NTLM_FLAG_UNICODE,
        &pWorkstation);
    BAIL_ON_LSA_ERROR(dwError);

    memcpy(
        pLMRespBuffer,
        (PBYTE)pRespMsg + pRespMsg->LmResponse.dwOffset,
        pRespMsg->LmResponse.usLength);

    memcpy(
        pNTRespBuffer,
        (PBYTE)pRespMsg + pRespMsg->NtResponse.dwOffset,
        pRespMsg->NtResponse.usLength);

    if (pRespMsg->NtResponse.usLength == 24 &&
            pChlngCtxt->NegotiatedFlags & NTLM_FLAG_NTLM2)
    {
        // The client sent an NTLM2 session response. That means we need to
        // calculate the challenge the client used.

        if (pRespMsg->LmResponse.usLength < 8)
        {
            dwError = LW_ERROR_INVALID_PARAMETER;
            BAIL_ON_LSA_ERROR(dwError);
        }

        // Calculate the session nonce first
        memcpy(sessionNonce + 0, pChlngCtxt->Challenge, 8);
        memcpy(sessionNonce + 8, pLMRespBuffer, 8);

        MD5(sessionNonce, 16, sessionHashUntrunc);

        Challenge.dwLen = NTLM_CHALLENGE_SIZE;
        Challenge.pData = sessionHashUntrunc;
    }
    else
    {
        Challenge.dwLen = NTLM_CHALLENGE_SIZE;
        Challenge.pData = pChlngCtxt->Challenge;
    }

    LMResp.dwLen = pRespMsg->LmResponse.usLength;
    LMResp.pData = pLMRespBuffer;

    NTResp.dwLen = pRespMsg->NtResponse.usLength;
    NTResp.pData = pNTRespBuffer;

    Params.AuthType = LSA_AUTH_CHAP;

    Params.pass.chap.pChallenge = &Challenge;
    Params.pass.chap.pLM_resp = &LMResp;
    Params.pass.chap.pNT_resp = &NTResp;

    Params.pszAccountName = pUserName;

    Params.pszDomain = pDomainName;

    Params.pszWorkstation = pWorkstation;

    dwError = LsaSrvAuthenticateUserEx(
        Handle,
        &Params,
        &pUserInfo
        );
    BAIL_ON_LSA_ERROR(dwError);

    LW_ASSERT(pUserInfo->pSessionKey->dwLen == NTLM_SESSION_KEY_SIZE);

    if (pRespMsg->NtResponse.usLength == 24 &&
            pChlngCtxt->NegotiatedFlags & NTLM_FLAG_NTLM2)
    {
        HMAC(
            EVP_md5(),
            pUserInfo->pSessionKey->pData,
            NTLM_SESSION_KEY_SIZE,
            sessionNonce,
            16,
            pSessionKey,
            NULL);
    }
    else
    {
        memcpy(pSessionKey, pUserInfo->pSessionKey->pData, NTLM_SESSION_KEY_SIZE);
    }

    // Free the pUserInfo for now... we may want to save this off later
    //
    dwError = LsaFreeAuthUserInfo(&pUserInfo);
    BAIL_ON_LSA_ERROR(dwError);

cleanup:

    LW_SAFE_FREE_MEMORY(pLMRespBuffer);
    LW_SAFE_FREE_MEMORY(pNTRespBuffer);
    LW_SAFE_FREE_STRING(pUserName);
    LW_SAFE_FREE_STRING(pDomainName);
    LW_SAFE_FREE_STRING(pWorkstation);

    return dwError;
error:
    goto cleanup;
}

/******************************************************************************/
DWORD
NtlmGetDomainNameFromResponse(
    IN PNTLM_RESPONSE_MESSAGE_V1 pRespMsg,
    IN BOOLEAN bUnicode,
    OUT PSTR* ppDomainName
    )
{
    DWORD dwError = LW_ERROR_SUCCESS;
    PCHAR pName = NULL;
    DWORD dwNameLength = 0;
    PBYTE pBuffer = NULL;
    PNTLM_SEC_BUFFER pSecBuffer = &pRespMsg->AuthTargetName;
    DWORD nIndex = 0;

    *ppDomainName = NULL;

    dwNameLength = pSecBuffer->usLength;
    pBuffer = pSecBuffer->dwOffset + (PBYTE)pRespMsg;

    if (!bUnicode)
    {
        dwError = LwAllocateMemory(dwNameLength + 1, OUT_PPVOID(&pName));
        BAIL_ON_LSA_ERROR(dwError);

        memcpy(pName, pBuffer, dwNameLength);
    }
    else
    {
        dwNameLength = dwNameLength / sizeof(WCHAR);

        dwError = LwAllocateMemory(dwNameLength + 1, OUT_PPVOID(&pName));
        BAIL_ON_LSA_ERROR(dwError);

        for (nIndex = 0; nIndex < dwNameLength; nIndex++)
        {
            pName[nIndex] = pBuffer[nIndex * sizeof(WCHAR)];
        }
    }

cleanup:
    *ppDomainName = pName;
    return dwError;
error:
    LW_SAFE_FREE_STRING(pName);
    goto cleanup;
}

/******************************************************************************/
DWORD
NtlmGetWorkstationFromResponse(
    IN PNTLM_RESPONSE_MESSAGE_V1 pRespMsg,
    IN BOOLEAN bUnicode,
    OUT PSTR* ppWorkstation
    )
{
    DWORD dwError = LW_ERROR_SUCCESS;
    PCHAR pName = NULL;
    DWORD dwNameLength = 0;
    PBYTE pBuffer = NULL;
    PNTLM_SEC_BUFFER pSecBuffer = &pRespMsg->Workstation;
    DWORD nIndex = 0;

    *ppWorkstation = NULL;

    dwNameLength = pSecBuffer->usLength;
    pBuffer = pSecBuffer->dwOffset + (PBYTE)pRespMsg;

    if (!bUnicode)
    {
        dwError = LwAllocateMemory(dwNameLength + 1, OUT_PPVOID(&pName));
        BAIL_ON_LSA_ERROR(dwError);

        memcpy(pName, pBuffer, dwNameLength);
    }
    else
    {
        dwNameLength = dwNameLength / sizeof(WCHAR);

        dwError = LwAllocateMemory(dwNameLength + 1, OUT_PPVOID(&pName));
        BAIL_ON_LSA_ERROR(dwError);

        for (nIndex = 0; nIndex < dwNameLength; nIndex++)
        {
            pName[nIndex] = pBuffer[nIndex * sizeof(WCHAR)];
        }
    }

cleanup:
    *ppWorkstation = pName;
    return dwError;
error:
    LW_SAFE_FREE_STRING(pName);
    goto cleanup;
}
