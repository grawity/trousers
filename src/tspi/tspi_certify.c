
/*
 * Licensed Materials - Property of IBM
 *
 * trousers - An open source TCG Software Stack
 *
 * (C) Copyright International Business Machines Corp. 2004-2006
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trousers/tss.h"
#include "trousers/trousers.h"
#include "trousers_types.h"
#include "spi_internal_types.h"
#include "spi_utils.h"
#include "capabilities.h"
#include "tsplog.h"
#include "obj.h"


TSS_RESULT
Tspi_Key_CertifyKey(TSS_HKEY hKey,			/* in */
		    TSS_HKEY hCertifyingKey,		/* in */
		    TSS_VALIDATION * pValidationData)	/* in, out */
{

	TCS_CONTEXT_HANDLE tcsContext;
	TCPA_RESULT result;
	TPM_AUTH certAuth;
	TPM_AUTH keyAuth;
	UINT64 offset = 0;
	//BYTE hashBlob[1024];
	TCPA_DIGEST digest;
	TCPA_NONCE antiReplay;
	UINT32 CertifyInfoSize;
	BYTE *CertifyInfo;
	UINT32 outDataSize;
	BYTE *outData;
	TSS_HPOLICY hPolicy;
	TSS_HPOLICY hCertPolicy;
	TCS_KEY_HANDLE certifyTCSKeyHandle, keyTCSKeyHandle;
	BYTE *keyData = NULL;
	UINT32 keyDataSize;
	TCPA_KEY keyContainer;
	TSS_BOOL useAuthCert;
	TSS_BOOL useAuthKey;
	TPM_AUTH *pCertAuth = &certAuth;
	TPM_AUTH *pKeyAuth = &keyAuth;
	TSS_HCONTEXT tspContext;
	Trspi_HashCtx hashCtx;


	if ((result = obj_rsakey_get_tsp_context(hKey, &tspContext)))
		return result;

	if ((result = obj_context_is_connected(tspContext, &tcsContext)))
		return result;

	if ((result = obj_rsakey_get_policy(hKey, TSS_POLICY_USAGE,
					    &hPolicy, &useAuthKey)))
		return result;

	if ((result = obj_rsakey_get_policy(hCertifyingKey, TSS_POLICY_USAGE,
					    &hCertPolicy, &useAuthCert)))
		return result;

	if ((result = obj_rsakey_get_tcs_handle(hCertifyingKey, &certifyTCSKeyHandle)))
		return result;

	if ((result = obj_rsakey_get_tcs_handle(hKey, &keyTCSKeyHandle)))
		return result;

	if (pValidationData == NULL) {
		LogDebug("Internal Verify");
		if ((result = internal_GetRandomNonce(tcsContext, &antiReplay)))
			return result;
	} else {
		LogDebug("External Verify");
		if (pValidationData->ulExternalDataLength < sizeof(antiReplay.nonce))
			return TSPERR(TSS_E_BAD_PARAMETER);

		memcpy(antiReplay.nonce, pValidationData->rgbExternalData,
		       sizeof(antiReplay.nonce));
	}

	if (useAuthCert && !useAuthKey)
		return TSPERR(TSS_E_BAD_PARAMETER);

	/* Setup the auths */
	if (useAuthCert || useAuthKey) {
		result = Trspi_HashInit(&hashCtx, TSS_HASH_SHA1);
		result |= Trspi_Hash_UINT32(&hashCtx, TPM_ORD_CertifyKey);
		result |= Trspi_HashUpdate(&hashCtx, sizeof(antiReplay.nonce), antiReplay.nonce);
		if ((result |= Trspi_HashFinal(&hashCtx, digest.digest)))
			return result;
	}

	if (useAuthKey) {
		if ((result = secret_PerformAuth_OIAP(hKey, TPM_ORD_CertifyKey,
						      hPolicy, &digest,
						      &keyAuth)))
			return result;
	} else
		pKeyAuth = NULL;

	if (useAuthCert) {
		if ((result = secret_PerformAuth_OIAP(hCertifyingKey,
						      TPM_ORD_CertifyKey,
						      hCertPolicy, &digest,
						      &certAuth)))
			return result;
	} else
		pCertAuth = NULL;

	if ((result = TCSP_CertifyKey(tcsContext,
				     certifyTCSKeyHandle,
				     keyTCSKeyHandle,
				     antiReplay,
				     pCertAuth,
				     pKeyAuth,
				     &CertifyInfoSize,
				     &CertifyInfo,
				     &outDataSize,
				     &outData))) {
		if (useAuthKey)
			TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
		if (useAuthCert)
			TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);
		return result;
	}

	/* Validate auth */
	if (useAuthCert || useAuthKey) {
		result = Trspi_HashInit(&hashCtx, TSS_HASH_SHA1);
		result |= Trspi_Hash_UINT32(&hashCtx, result);
		result |= Trspi_Hash_UINT32(&hashCtx, TPM_ORD_CertifyKey);
		result |= Trspi_HashUpdate(&hashCtx, CertifyInfoSize, CertifyInfo);
		if ((result |= Trspi_HashFinal(&hashCtx, digest.digest)))
			return result;

		if (useAuthKey) {
			if ((result = obj_policy_validate_auth_oiap(hPolicy, &digest, &keyAuth))) {
				TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
				if (useAuthCert)
					TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);
				return result;
			}
		}
		if (useAuthCert) {
			if ((result = obj_policy_validate_auth_oiap(hCertPolicy, &digest, &certAuth))) {
				TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);
				if (useAuthKey)
					TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
				return result;
			}
		}
	}

	if (pValidationData == NULL) {
		if ((result = obj_rsakey_get_blob(hCertifyingKey,
							&keyDataSize, &keyData))) {
			if (useAuthKey)
				TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
			if (useAuthCert)
				TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);

			LogError("Error in calling GetAttribData internally");
			return TSPERR(TSS_E_INTERNAL_ERROR);
		}

		memset(&keyContainer, 0, sizeof(TCPA_KEY));

		offset = 0;
		if ((result = Trspi_UnloadBlob_KEY(&offset, keyData, &keyContainer)))
			return result;

		if ((result = Trspi_Hash(TSS_HASH_SHA1, CertifyInfoSize, CertifyInfo,
					 digest.digest))) {
			free_key_refs(&keyContainer);
			return result;
		}

		if ((result = Trspi_Verify(TSS_HASH_SHA1, digest.digest, 20,
					   keyContainer.pubKey.key,
					   keyContainer.pubKey.keyLength,
					   outData, outDataSize))) {
			if (useAuthKey)
				TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
			if (useAuthCert)
				TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);

			free_key_refs(&keyContainer);
			return TSPERR(TSS_E_VERIFICATION_FAILED);
		}
		free_key_refs(&keyContainer);
	} else {
		pValidationData->ulDataLength = CertifyInfoSize;
		pValidationData->rgbData = calloc_tspi(tspContext, CertifyInfoSize);
		if (pValidationData->rgbData == NULL) {
			LogError("malloc of %d bytes failed.", CertifyInfoSize);
			if (useAuthKey)
				TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
			if (useAuthCert)
				TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);

			return TSPERR(TSS_E_OUTOFMEMORY);
		}
		memcpy(pValidationData->rgbData, CertifyInfo, CertifyInfoSize);
		pValidationData->ulValidationDataLength = outDataSize;
		pValidationData->rgbValidationData = calloc_tspi(tspContext, outDataSize);
		if (pValidationData->rgbValidationData == NULL) {
			LogError("malloc of %d bytes failed.", outDataSize);
			if (useAuthKey)
				TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
			if (useAuthCert)
				TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);

			return TSPERR(TSS_E_OUTOFMEMORY);
		}
		memcpy(pValidationData->rgbValidationData, outData, outDataSize);
	}

	if (useAuthKey)
		TCSP_TerminateHandle(tcsContext, keyAuth.AuthHandle);
	if (useAuthCert)
		TCSP_TerminateHandle(tcsContext, certAuth.AuthHandle);

	return TSS_SUCCESS;
}

