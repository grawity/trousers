
/*
 * Licensed Materials - Property of IBM
 *
 * trousers - An open source TCG Software Stack
 *
 * (C) Copyright International Business Machines Corp. 2004, 2005
 *
 */


#ifndef _SPI_UTILS_H_
#define _SPI_UTILS_H_

#include <pthread.h>
#include <netinet/in.h> // for endian routines

struct key_mem_cache
{
	TCS_KEY_HANDLE tcs_handle;
	TSS_HKEY tsp_handle;
	UINT16 flags;
	UINT32 time_stamp;
	TSS_UUID uuid;
	TSS_UUID p_uuid;
	TCPA_KEY *blob;
	struct key_mem_cache *parent;
	struct key_mem_cache *next;
};

extern struct key_mem_cache *key_mem_cache_head;
extern pthread_mutex_t mem_cache_lock;

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define BOOL(x)		((x) == 0) ? FALSE : TRUE
#define INVBOOL(x)	((x) == 0) ? TRUE : FALSE

#define INCREMENT	1
#define DECREMENT	0

UINT32 UnicodeToArray(BYTE *, UNICODE *);
UINT32 ArrayToUnicode(BYTE *, UINT32, UNICODE *);
UINT32 StringToUnicodeArray(char *, BYTE *);

TSS_RESULT internal_GetRandomNonce(TCS_CONTEXT_HANDLE, TCPA_NONCE *);

void *calloc_tspi(TSS_HCONTEXT, UINT32);
TSS_RESULT free_tspi(TSS_HCONTEXT, void *);
TSS_RESULT add_mem_entry(TSS_HCONTEXT, void *);

/* secrets.c */

TSS_RESULT policy_UsesAuth(TSS_HPOLICY, TSS_BOOL *);

TSS_RESULT secret_PerformAuth_OIAP(TSS_HOBJECT, UINT32, TSS_HPOLICY, TCPA_DIGEST *,
				   TPM_AUTH *);
TSS_RESULT secret_PerformXOR_OSAP(TSS_HPOLICY, TSS_HPOLICY, TSS_HPOLICY, TSS_HOBJECT,
				  UINT16, UINT32, TCPA_ENCAUTH *, TCPA_ENCAUTH *,
				  BYTE *, TPM_AUTH *, TCPA_NONCE *);
TSS_RESULT secret_PerformAuth_OSAP(TSS_HOBJECT, UINT32, TSS_HPOLICY,
				   TSS_HPOLICY, TSS_HPOLICY, BYTE *,
				   TPM_AUTH *, BYTE *, TCPA_NONCE *);

TSS_RESULT secret_ValidateAuth_OSAP(TSS_HOBJECT, UINT32, TSS_HPOLICY,
				    TSS_HPOLICY, TSS_HPOLICY, BYTE *,
				    TPM_AUTH *, BYTE *, TCPA_NONCE *);

TSS_RESULT secret_TakeOwnership(TSS_HKEY, TSS_HTPM, TSS_HKEY, TPM_AUTH *,
				UINT32 *, BYTE *, UINT32 *, BYTE *);

#define next( x )	x = x->next

/* spi_utils.c */

UINT16 get_num_pcrs(TCS_CONTEXT_HANDLE);
void   free_key_refs(TCPA_KEY *);

#define UI_MAX_SECRET_STRING_LENGTH	256
#define UI_MAX_POPUP_STRING_LENGTH	256

#ifdef TSS_NO_GUI
#define DisplayPINWindow(a,b,c)			\
	do {					\
		*(b) = 0;			\
	} while (0)
#define DisplayNewPINWindow(a,b,c)			\
	do {					\
		*(b) = 0;			\
	} while (0)
#else
TSS_RESULT DisplayPINWindow(BYTE *, UINT32 *, BYTE *);
TSS_RESULT DisplayNewPINWindow(BYTE *, UINT32 *, BYTE *);
#endif

TSS_RESULT merge_key_hierarchies(TSS_HCONTEXT, UINT32, TSS_KM_KEYINFO *, UINT32, TSS_KM_KEYINFO *,
				 UINT32 *, TSS_KM_KEYINFO **);

int pin_mem(void *, size_t);
int unpin_mem(void *, size_t);


TSS_RESULT internal_GetMachineName(UNICODE *, int);
TSS_RESULT internal_GetCap(TSS_HCONTEXT, TSS_FLAG, UINT32, UINT32 *, BYTE **);

TSS_RESULT ConnectGuts(TSS_HCONTEXT, UNICODE *, TCS_CONTEXT_HANDLE);

/* For an unconnected context that wants to do PCR operations, assume that
 * the TPM has TSS_DEFAULT_NUM_PCRS pcrs */
#define TSS_DEFAULT_NUM_PCRS		16
#define TSS_LOCAL_RANDOM_DEVICE		"/dev/urandom"
#define TSS_LOCALHOST_STRING		"localhost"
TSS_RESULT get_local_random(TSS_HCONTEXT, UINT32, BYTE **);

#define AUTH_RETRY_NANOSECS	500000000
#define AUTH_RETRY_COUNT	5

#define endian32(x)	htonl(x)
#define endian16(x)	htons(x)

extern TSS_VERSION VERSION_1_1;

/* openssl.c */
#ifdef TSS_DEBUG
#define DEBUG_print_openssl_errors() \
	do { \
		ERR_load_crypto_strings(); \
		ERR_print_errors_fp(stderr); \
	} while (0)
#else
#define DEBUG_print_openssl_errors()
#endif


TSS_RESULT Init_AuthNonce(TCS_CONTEXT_HANDLE, TPM_AUTH *);
TSS_BOOL validateReturnAuth(BYTE *, BYTE *, TPM_AUTH *);
void HMAC_Auth(BYTE *, BYTE *, TPM_AUTH *);
TSS_RESULT OSAP_Calc(TCS_CONTEXT_HANDLE, UINT16, UINT32, BYTE *, BYTE *, BYTE *,
			TCPA_ENCAUTH *, TCPA_ENCAUTH *, BYTE *, TPM_AUTH *);

UINT16 Decode_UINT16(BYTE *);
void UINT32ToArray(UINT32, BYTE *);
void UINT16ToArray(UINT16, BYTE *);
UINT32 Decode_UINT32(BYTE *);

TSS_RESULT popup_GetSecret(UINT32, UINT32, BYTE *, void *);

TSS_BOOL check_flagset_collision(TSS_FLAG, UINT32);
TSS_RESULT get_tpm_flags(TCS_CONTEXT_HANDLE, TSS_HTPM, UINT32 *, UINT32 *);
TSS_RESULT calc_composite_from_object(TCPA_PCR_SELECTION *, TCPA_PCRVALUE *, TCPA_DIGEST *);

void LoadBlob_AUTH(UINT64 *, BYTE *, TPM_AUTH *);
void UnloadBlob_AUTH(UINT64 *, BYTE *, TPM_AUTH *);
void LoadBlob_LOADKEY_INFO(UINT64 *, BYTE *, TCS_LOADKEY_INFO *);
void UnloadBlob_LOADKEY_INFO(UINT64 *, BYTE *, TCS_LOADKEY_INFO *);


TSS_RESULT TSP_SetCapability(TCS_CONTEXT_HANDLE, TSS_HTPM, TSS_HPOLICY, TPM_CAPABILITY_AREA,
			     UINT32, TSS_BOOL);

TSS_RESULT TCS_CloseContext(TCS_CONTEXT_HANDLE);
TSS_RESULT TCS_OpenContext_RPC(BYTE *, UINT32 *, int);
TSS_RESULT TCS_GetCapability(TCS_CONTEXT_HANDLE, TCPA_CAPABILITY_AREA, UINT32, BYTE *,
                              UINT32 *, BYTE **);
TSS_RESULT TCSP_GetCapability(TCS_CONTEXT_HANDLE, TCPA_CAPABILITY_AREA,	UINT32, BYTE *, UINT32 *,
				BYTE **);
TSS_RESULT TCSP_SetCapability(TCS_CONTEXT_HANDLE, TCPA_CAPABILITY_AREA,	UINT32, BYTE *, UINT32,
				BYTE *, TPM_AUTH *);
TSS_RESULT TCSP_LoadKeyByBlob(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, UINT32, BYTE *, TPM_AUTH *,
                               TCS_KEY_HANDLE *, TCS_KEY_HANDLE *);
TSS_RESULT TCSP_LoadKeyByUUID(TCS_CONTEXT_HANDLE, TSS_UUID, TCS_LOADKEY_INFO *, TCS_KEY_HANDLE *);
TSS_RESULT TCS_GetRegisteredKeyBlob(TCS_CONTEXT_HANDLE, TSS_UUID, UINT32 *, BYTE **);
TSS_RESULT TCS_RegisterKey(TCS_CONTEXT_HANDLE, TSS_UUID, TSS_UUID, UINT32, BYTE *, UINT32, BYTE *);
TSS_RESULT TCSP_UnregisterKey(TCS_CONTEXT_HANDLE, TSS_UUID);
TSS_RESULT TCS_EnumRegisteredKeys(TCS_CONTEXT_HANDLE, TSS_UUID *, UINT32 *, TSS_KM_KEYINFO **);
TSS_RESULT TCSP_GetRegisteredKeyByPublicInfo(TCS_CONTEXT_HANDLE, TCPA_ALGORITHM_ID, UINT32,
                                              BYTE *, UINT32 *, BYTE **);
TSS_RESULT TCSP_ChangeAuth(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_PROTOCOL_ID, TCPA_ENCAUTH,
				TCPA_ENTITY_TYPE, UINT32, BYTE *, TPM_AUTH *, TPM_AUTH *,
	                        UINT32 *, BYTE **);
TSS_RESULT TCSP_ChangeAuthOwner(TCS_CONTEXT_HANDLE, TCPA_PROTOCOL_ID, TCPA_ENCAUTH, TCPA_ENTITY_TYPE,
                                 TPM_AUTH *);
TSS_RESULT TCSP_TerminateHandle(TCS_CONTEXT_HANDLE, TCS_AUTHHANDLE);
TSS_RESULT TCSP_GetRandom(TCS_CONTEXT_HANDLE, UINT32, BYTE **);
TSS_RESULT TCSP_ChangeAuthAsymStart(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_NONCE, UINT32, BYTE *,
                                     TPM_AUTH *, UINT32 *, BYTE **, UINT32 *, BYTE **, UINT32 *,
                                     BYTE **, TCS_KEY_HANDLE *);
TSS_RESULT TCSP_ChangeAuthAsymFinish(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCS_KEY_HANDLE,
					TCPA_ENTITY_TYPE, TCPA_HMAC, UINT32, BYTE *, UINT32,
					BYTE *, TPM_AUTH *, UINT32 *, BYTE **, TCPA_SALT_NONCE *,
					TCPA_DIGEST *);
TSS_RESULT TCSP_GetPubKey(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_CreateWrapKey(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_ENCAUTH, TCPA_ENCAUTH,
				UINT32, BYTE *, UINT32 *, BYTE **, TPM_AUTH *);
TSS_RESULT TCSP_CertifyKey(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCS_KEY_HANDLE, TCPA_NONCE, TPM_AUTH *,
				TPM_AUTH *, UINT32 *, BYTE **, UINT32 *, BYTE **);
TSS_RESULT TCSP_CreateMigrationBlob(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_MIGRATE_SCHEME, UINT32,
					BYTE *, UINT32, BYTE *, TPM_AUTH *, TPM_AUTH *, UINT32 *,
					BYTE **, UINT32 *, BYTE **);
TSS_RESULT TCSP_ConvertMigrationBlob(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, UINT32, BYTE *, UINT32,
				     BYTE *, TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_PcrRead(TCS_CONTEXT_HANDLE, TCPA_PCRINDEX, TCPA_PCRVALUE *);
TSS_RESULT TCSP_OSAP(TCS_CONTEXT_HANDLE, TCPA_ENTITY_TYPE, UINT32, TCPA_NONCE, TCS_AUTHHANDLE *,
			TCPA_NONCE *, TCPA_NONCE *);
TSS_RESULT TCSP_GetCapabilityOwner(TCS_CONTEXT_HANDLE, TPM_AUTH *, TCPA_VERSION *, UINT32 *, UINT32 *);
TSS_RESULT TCSP_OIAP(TCS_CONTEXT_HANDLE, TCS_AUTHHANDLE *, TCPA_NONCE *);
TSS_RESULT TCSP_Seal(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_ENCAUTH, UINT32, BYTE *, UINT32, BYTE *,
                                       TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_Unseal(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, UINT32, BYTE *, TPM_AUTH *, TPM_AUTH *,
                                         UINT32 *, BYTE **);
TSS_RESULT TCSP_UnBind(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, UINT32, BYTE *, TPM_AUTH *, UINT32 *,
                                         BYTE **);
TSS_RESULT TCSP_Sign(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, UINT32, BYTE *, TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_CreateEndorsementKeyPair(TCS_CONTEXT_HANDLE, TCPA_NONCE, UINT32, BYTE *, UINT32 *,
						BYTE **, TCPA_DIGEST *);
TSS_RESULT TCSP_ReadPubek(TCS_CONTEXT_HANDLE, TCPA_NONCE, UINT32 *, BYTE **, TCPA_DIGEST *);
TSS_RESULT TCSP_OwnerReadPubek(TCS_CONTEXT_HANDLE, TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_TakeOwnership(TCS_CONTEXT_HANDLE, UINT16, UINT32, BYTE *, UINT32, BYTE *, UINT32,
				BYTE *, TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_MakeIdentity(TCS_CONTEXT_HANDLE, TCPA_ENCAUTH, TCPA_CHOSENID_HASH, UINT32, BYTE *,
				TPM_AUTH *, TPM_AUTH *, UINT32 *, BYTE **, UINT32 *, BYTE **, UINT32 *,
				BYTE **, UINT32 *, BYTE **, UINT32 *, BYTE **);
TSS_RESULT TCSP_ActivateTPMIdentity(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, UINT32, BYTE *, TPM_AUTH *,
					TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_OwnerClear(TCS_CONTEXT_HANDLE, TPM_AUTH *);
TSS_RESULT TCSP_DisableOwnerClear(TCS_CONTEXT_HANDLE, TPM_AUTH *);
TSS_RESULT TCSP_ForceClear(TCS_CONTEXT_HANDLE);
TSS_RESULT TCSP_DisableForceClear(TCS_CONTEXT_HANDLE);
TSS_RESULT TCSP_PhysicalDisable(TCS_CONTEXT_HANDLE);
TSS_RESULT TCSP_PhysicalEnable(TCS_CONTEXT_HANDLE);
TSS_RESULT TCSP_PhysicalSetDeactivated(TCS_CONTEXT_HANDLE, TSS_BOOL);
TSS_RESULT TCSP_PhysicalPresence(TCS_CONTEXT_HANDLE, TCPA_PHYSICAL_PRESENCE);
TSS_RESULT TCSP_SetTempDeactivated(TCS_CONTEXT_HANDLE);
TSS_RESULT TCSP_OwnerSetDisable(TCS_CONTEXT_HANDLE, TSS_BOOL, TPM_AUTH *);
TSS_RESULT TCSP_ResetLockValue(TCS_CONTEXT_HANDLE, TPM_AUTH *);
TSS_RESULT TCSP_SetOwnerInstall(TCS_CONTEXT_HANDLE, TSS_BOOL);
TSS_RESULT TCSP_DisablePubekRead(TCS_CONTEXT_HANDLE, TPM_AUTH *);
TSS_RESULT TCSP_SelfTestFull(TCS_CONTEXT_HANDLE);
TSS_RESULT TCSP_CertifySelfTest(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_NONCE, TPM_AUTH *, UINT32 *,
				BYTE **);
TSS_RESULT TCSP_GetTestResult(TCS_CONTEXT_HANDLE, UINT32 *, BYTE **);
TSS_RESULT TCSP_StirRandom(TCS_CONTEXT_HANDLE, UINT32, BYTE *);
TSS_RESULT TCSP_AuthorizeMigrationKey(TCS_CONTEXT_HANDLE, TCPA_MIGRATE_SCHEME, UINT32, BYTE *,
					TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCS_GetPcrEvent(TCS_CONTEXT_HANDLE, UINT32, UINT32 *, TSS_PCR_EVENT **);
TSS_RESULT TCS_GetPcrEventsByPcr(TCS_CONTEXT_HANDLE, UINT32, UINT32, UINT32 *, TSS_PCR_EVENT **);
TSS_RESULT TCS_GetPcrEventLog(TCS_CONTEXT_HANDLE, UINT32 *, TSS_PCR_EVENT **);
TSS_RESULT TCSP_Quote(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE, TCPA_NONCE, UINT32, BYTE *, TPM_AUTH *,
			UINT32 *, BYTE **, UINT32 *, BYTE **);
TSS_RESULT TCSP_Extend(TCS_CONTEXT_HANDLE, TCPA_PCRINDEX, TCPA_DIGEST, TCPA_PCRVALUE *);
TSS_RESULT TCSP_DirWriteAuth(TCS_CONTEXT_HANDLE, TCPA_DIRINDEX, TCPA_DIRVALUE, TPM_AUTH *);
TSS_RESULT TCSP_DirRead(TCS_CONTEXT_HANDLE, TCPA_DIRINDEX, TCPA_DIRVALUE *);
TSS_RESULT TCS_LogPcrEvent(TCS_CONTEXT_HANDLE, TSS_PCR_EVENT, UINT32 *);
TSS_RESULT TCSP_EvictKey(TCS_CONTEXT_HANDLE, TCS_KEY_HANDLE);
TSS_RESULT TCSP_CreateMaintenanceArchive(TCS_CONTEXT_HANDLE, TSS_BOOL, TPM_AUTH *, UINT32 *, BYTE **, UINT32 *, BYTE **);
TSS_RESULT TCSP_KillMaintenanceFeature(TCS_CONTEXT_HANDLE, TPM_AUTH *);
TSS_RESULT TCSP_LoadMaintenanceArchive(TCS_CONTEXT_HANDLE, UINT32, BYTE *, TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_LoadManuMaintPub(TCS_CONTEXT_HANDLE, TCPA_NONCE, UINT32, BYTE *, TCPA_DIGEST *);
TSS_RESULT TCSP_ReadManuMaintPub(TCS_CONTEXT_HANDLE, TCPA_NONCE, TCPA_DIGEST *);
TSS_RESULT TCSP_DaaJoin(TCS_CONTEXT_HANDLE,  TSS_HDAA, BYTE, UINT32, BYTE *, UINT32, BYTE *,
			TPM_AUTH *, UINT32 *, BYTE **);
TSS_RESULT TCSP_DaaSign(TCS_CONTEXT_HANDLE,  TSS_HDAA, BYTE, UINT32, BYTE *, UINT32, BYTE *,
			TPM_AUTH *, UINT32 *, BYTE **);

#endif
