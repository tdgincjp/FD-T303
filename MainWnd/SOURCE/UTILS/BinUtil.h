#ifndef DATABINRANGE_H
#define	DATABINRANGE_H

#ifdef	__cplusplus
extern "C" {
#endif


#define SZ_BIN_CARD_RANGE                         11
#define SZ_BIN_NAME                               16
#define SZ_BIN_CARD_SERVICE_TYPE                  2
#define SZ_BIN_REFUND_LIMIT                       10
#define SZ_BIN_SURCHARGE_LIMIT                    9
#define SZ_BIN_RID                                10
#define SZ_BIN_AID                                16
#define SZ_ACTIVE_BIN_RANGE                       11

#define SZ_BIN_CTLS_EMV_CVM_LIMIT                 6
#define SZ_BIN_CTLS_EMV_TXN_LIMIT                 6
#define SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT            4
#define SZ_BIN_CTLS_EMV_OFFLINE_FLOOR_LIMIT       6

#define SZ_EMV_KEY_RID                            5
#define SZ_EMV_KEY_SERVICE_TYPE                   2
#define SZ_EMV_KEY_INDEX                          1
#define SZ_EMV_KEY_HASH_ALGORITHM_INDICATOR       1
#define SZ_EMV_KEY_SIGNATURE_ALGORITHM_INDICATOR  1
#define SZ_EMV_KEY_MODULUS                        248
#define SZ_EMV_KEY_EXPONENT                       3
#define SZ_EMV_KEY_CHECKSUM                       20
#define SZ_EMV_KEY_EXPIRY_DATE                    2
#define SZ_EMV_TAC                                5
#define SZ_EMV_APPLICATION_NUMBER                 4
#define SZ_EMV_THRESHOLD_VALUE                    4
#define SZ_EMV_MAX_DOL                            252
#define SZ_EMV_INTERAC_RRL                        12
#define SZ_EMV_INTERAC_TOS                        2
#define SZ_EMV_INTERAC_TTI                        3
#define SZ_EMV_VISA_TTQ                           4
#define SZ_APPLICATION_LABEL                      20
 

#define CTLS_MSR_PROGRAM_ENABLED                  0x02
#define CTLS_EMV_PROGRAM_ENABLED                  0x01

typedef struct 
{
  char serviceType[SZ_BIN_CARD_SERVICE_TYPE+1];
  char low[SZ_BIN_CARD_RANGE+1];
  char high[SZ_BIN_CARD_RANGE+1];
  int minLen;
  int maxLen;
  char name[SZ_BIN_NAME+1];
} sBinRangeData;

#define BIN_MAX_NUMBER_RECORDS   30 

typedef struct
{
	char serviceType[SZ_BIN_CARD_SERVICE_TYPE+1];
	char surchargeLimit[SZ_BIN_SURCHARGE_LIMIT+1];
	BYTE tranProfile;
} sServiceData;

typedef struct
{
	char serviceType[SZ_BIN_CARD_SERVICE_TYPE+1];
	BOOL hasData;
  USHORT defaultTDOLLength;
  BYTE defaultTDOL[SZ_EMV_MAX_DOL];
  USHORT defaultDDOLLength;
  BYTE defaultDDOL[SZ_EMV_MAX_DOL];
  BYTE emvAppVersion[SZ_EMV_APPLICATION_NUMBER];
  BYTE msdAppVersion[SZ_EMV_APPLICATION_NUMBER];
  BYTE ctlsCvmLimit[SZ_BIN_CTLS_EMV_CVM_LIMIT];
  BYTE ctlsTxnLimit[SZ_BIN_CTLS_EMV_TXN_LIMIT];
  BYTE interactTOS[SZ_EMV_INTERAC_TOS];
  char interactRRL[SZ_EMV_INTERAC_RRL+1];
  BYTE visaTTQ[SZ_EMV_VISA_TTQ];
  BYTE interactTTI[SZ_EMV_INTERAC_TTI];  // hard coded, not from host
} sCtlsParams;

typedef struct
{
	char serviceType[SZ_BIN_CARD_SERVICE_TYPE+1];
	char asciiAID[SZ_BIN_AID*2+1];
	BYTE AID[SZ_BIN_AID];
  USHORT AIDLength;
  BYTE emvFlag;
  BOOL fallbackAllowed;
  BYTE emvOfflineFloorLimit[SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT];
  BYTE ctlsOfflineFloorLimit[SZ_BIN_CTLS_EMV_OFFLINE_FLOOR_LIMIT];
	BYTE TACDefault[SZ_EMV_TAC];
	BYTE TACDenial[SZ_EMV_TAC];
	BYTE TACOnline[SZ_EMV_TAC];
	BYTE ctlsTACDefault[SZ_EMV_TAC];
	BYTE ctlsTACDenial[SZ_EMV_TAC];
	BYTE ctlsTACOnline[SZ_EMV_TAC];
  BYTE MTI;
  BYTE appVersion[SZ_EMV_APPLICATION_NUMBER];
  BYTE thresholdValue[SZ_EMV_THRESHOLD_VALUE];
  BYTE targetPercentage;
  BYTE maximumTargetPercentage;
  USHORT defaultDDOLLength;
  BYTE defaultDDOL[SZ_EMV_MAX_DOL];
  USHORT defaultTDOLLength;
  BYTE defaultTDOL[SZ_EMV_MAX_DOL];
  BYTE applicationSelectionIndicator;
} sEmvData; // did k format 1

typedef struct
{
  BYTE RID[SZ_BIN_RID];
  BYTE index;
  USHORT modulusLength;
  BYTE modulus[SZ_EMV_KEY_MODULUS]; // binary format
  USHORT checksumLength;
  BYTE checksum[SZ_EMV_KEY_CHECKSUM];
  USHORT exponentLength;
  BYTE exponent[SZ_EMV_KEY_EXPONENT]; // in ASCII Hex
  BYTE hashAlgorithmIndicator;
  BYTE signatureAlgorithmIndicator;
} sEmvKeyData;  // did k format 2

BOOL BIN_SearchRecord(char *account, char *servType, sBinRangeData *pRec);
BOOL BIN_GetCardName(char *servType, char *cardName);


#ifdef	__cplusplus
}
#endif

#endif	/* DATABINRANGE_H */
