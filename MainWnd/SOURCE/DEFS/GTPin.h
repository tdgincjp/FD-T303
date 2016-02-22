/*
;******************************************************************************
;*
;* 	FD Global Terminal module (GTPin.H)
;*
;* 	Copyright (c) XAC Corporation.All rights reserved.
;* 	
;*	Author: Gary li (gary_li@xacsz.com.cn)
;*	Created: 04/19/2011
;*	Revision: 06/28/2011
;*	
;******************************************************************************
*/

#ifndef GTPIN_H
#define GTPIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <saioxpd.h>


//
//	ZKA PIN PAD state
//
#ifndef PIN_ZKA_SM_S0
#define PIN_ZKA_SM_S0		0 //idle
#endif
//#define PIN_ZKA_SM_S1		1 //0200
//#define PIN_ZKA_SM_S2		2 //0400
//#define PIN_ZKA_SM_S3		3 //0401
#ifndef PIN_ZKA_SM_Y_NG
#define PIN_ZKA_SM_Y_NG		4 //ZKA key index mismatch state
#endif
//#define PIN_ZKA_SM_NO_KEY	5 //No ZKA initial key in PINpad

//
//	ZKA Mode
//
#define PIN_ZKA_9000		0 //9000 request
#define PIN_ZKA_9010		1 //9010 request

//
// MAC mode
//
#define PIN_MAC_Generation		0
#define PIN_MAC_Verification	1
// mac verify result
#define PIN_MAC_VERIFY_OK	0
#define PIN_MAC_VERIFY_NOK	1

//
// Data crypto mode
//
#define PIN_DEC_CBC	0
#define PIN_ENC_CBC	1
#define PIN_DEC_ECB	2
#define PIN_ENC_ECB	3
//
// error code
//
//custom
#define PIN_CHALLENGE_FAIL	0xE050        //Sign value mismatched between FEP(Host server) and EPP. 
//#define PIN_SEQ_MISMATCH	0xE052                //EPP Sequence mismatch 
//#define PIN_STATE_MISMATCH	0xE054        //Key index mismatch state, Need to Diag. 
//#define PIN_STATE_ERROR		0xE056                //Run with improper status ID in SM(status machine) 
//#define PIN_INSUFFICIENT_MEMORY	0xE058        //EPP out of memory for saving data(0x0009).
#ifndef PIN_CANCEL
#define PIN_CANCEL 0xE018  // The user has canceled the pin entry
#endif
#ifndef PIN_TIMEOUT
#define PIN_TIMEOUT 0xE019 // The pin entry is aborted due to timeout
#endif

//
// Poland message security mode
//
#define PIN_FDP_AUTH 1  // Mutual Authentication(crypto1/crypto2)
#define PIN_FDP_NEWKEK_CHK 2 // New Key Download(crypto3/crypto4)
#define PIN_FDP_NEWKEK_GO 3 // Mutual Authentication(crypto3)

//
//
//
#define PIN_FDP_DUKPT_KEY 0 //FDP DUKPT key for SPDH host
#define PIN_FDP_FGS_KEY 1 //FDP KEY_1 for FGS host mutual authentication
#define PIN_FDP_SSL_PWD 2 //FDP SSL PWD KEY
#define PIN_FDP_SSL_P12 3 //FDP SSL private key

//
// UK HOST macros
//
#define PIN_HOST_MASTERCARD_VISA 0 //MasterCard/Visa host
#define PIN_HOST_AMEX				1 //AMEX host
#define PIN_HOST_DINERS			2 //Diners host
#define PIN_HOST_DISCOVER		3 //Discover host
#define PIN_HOST_FDC				4 //FDC host
#define PIN_HOST_RFU 				5 //RFU host



//
//	SRED marcos
//

#define	SRED_FUN				0

//	Key slots
#define	SRED_HAK				0x60
#define	SRED_IAK				0x61
#define	SRED_RAK1				0x62

#define	SRED_RAK2				0x66
#define	SRED_MAK				0x67

#define READER_BKLK				0x30
#define	READER_IAK				0x4C
#define	READER_RAK				0x4B

#define READER_MODE_TDES		0x00
#define READER_MODE_AES			0x01



//MAC authenticate type
#define XAC_CLIENT_AUTH			1
#define XAC_HOST_AUTH			2
#define XAC_GENERATE_SK			3

//MAC aythenticate algorithm
#define XAC_METHOD_TDES			1
#define XAC_METHOD_RSA			2
#define XAC_METHOD_PWD			3

//Account management mode
#define SRED_OPTION			0
#define SRED_ENABLE			1

//Additional error code&definations.
#ifndef XPD_PROTOCOL_VNG2
#define XPD_PROTOCOL_VNG2 		0x06	
#endif

#ifndef PIN_OPERATION_FAILED
#define	PIN_OPERATION_FAILED	0xE007
#endif

#ifndef XPD_OPERATION_FAILED
#define XPD_OPERATION_FAILED 		0xE040
#endif

//Admin id
#define ADMIN_A	'A'
#define ADMIN_B	'B'

//Type of change process
#define TYPE_ENCRYPTED	0

//The status of password change process
#define PWD_CHANGE_SUCESS	0x00
#define PWD_CHANGE_FAILED	0x01
#define PWD_CHANGE_STEP	0x02

//Type of the log
#define TYPE_CHANGE_PASSWORD	0x00
#define TYPE_INSTALLATION	0x01

//KCV mode
#define KCV_LEN_2	2


//
//  FME macro
//
#define PIN_ENCRYPT_FAIL	0xE060 // TDES or AES encryption fail
#define PIN_DECRYPT_FAIL	0xE061 // TDES or AES decryption fail
//#define PIN_CHALLENGE_FAIL	0xE050 // //Sign value mismatched.

//
// GT PIN API - common function
//
extern DWORD GTPinOpen(void);
extern DWORD GTPinClose(void);
extern DWORD GTPinSetData(BYTE, BYTE *, DWORD);
extern DWORD GTPinGetData(BYTE, BYTE *, DWORD);
extern DWORD GTPinGetInfo(BYTE *);

//
//	ZKAPINpad API routines 
//
extern DWORD ZKAPinOpen(void);
extern DWORD ZKAPinClose(void);
extern DWORD ZKAPinYInit(BYTE, BYTE *, BYTE *);
extern DWORD ZKAPinGetStatus(BYTE *);
extern DWORD ZKAPinSetData(BYTE, BYTE *, DWORD);
extern DWORD ZKAPinGetData(BYTE, BYTE *, DWORD);
extern DWORD ZKAPinMac(BYTE, BYTE *, DWORD, BYTE *);
extern DWORD ZKAPinEntry(BYTE, BYTE, DWORD, BOOL, void (*pfnInputUI)(void), BYTE *);
extern DWORD ZKAPinOffLineVerify(BOOL, BYTE *, BYTE, DWORD, BYTE *, BYTE, BYTE, DWORD,
						  BOOL, void (*pfnInputUI)(void));
extern DWORD ZKAPinDataCrypto(BYTE, BYTE *, DWORD, BYTE *);
extern DWORD ZKAPinGetInfo(BYTE *);
extern DWORD ZKAPinSetYStatus(void);
extern DWORD ZKAPinBhCancel(void);
extern DWORD ZKAPinBeforehandEntry(BYTE,BYTE,DWORD,BOOL,BOOL,void (*pfnInputUI)(void));
//extern DWORD ZKAPinBeforehandEntry(BYTE,BYTE,DWORD,BOOL,void (*pfnInputUI)(void));
extern DWORD ZKAPinBhComputeEPB(BYTE *);
extern DWORD ZKAPinBhOfflineVerify(BOOL,BYTE *,BYTE,DWORD,BYTE *);

//Poland
extern DWORD FDPFGSAuth(BYTE, BYTE *,BYTE, BYTE *, BYTE *);
extern DWORD FDPSSLGetPWD(BYTE *, DWORD, BYTE *);
extern DWORD FDPRLK(DWORD, BYTE *, DWORD);

//
//---------------------UKPA subroutines declaration------------------
DWORD UKSelectHost(BYTE HostID);	//new
DWORD UKResetKeyRegister(void);
DWORD UKDeriveMacKey(BYTE *KeyData);
DWORD UKGenerateMac(BYTE algorithm,
					BYTE * message,
					DWORD  msgLen,
					BYTE * oReqMAC,
					BYTE * oReqMACResidue);
DWORD UKCheckMac(BYTE algorithm,
				 BYTE * message,
				 DWORD  msgLen,
				 BYTE * iAuthParam,
				 BYTE * iRespMAC,
				 BYTE * oReqMACResidue);
DWORD UKUpdateKeyRegister(BYTE *KeyData);



//
//---------------------SRED subroutines declaration------------------
DWORD SREDGetInfo(BYTE *info);
DWORD SREDSelectKey(BYTE funid,	BYTE keyid);
DWORD SREDSetPANMASK (BYTE devid,BYTE leadinglen,BYTE tailinglen,BYTE replacechar);
DWORD SREDSetFormat (BYTE devid,BYTE algorithm,BYTE sslrc,WORD len,BYTE *buffer);
DWORD SREDPANEntry(BYTE *pan, DWORD len);
DWORD SREDGetAccount(BYTE *pan,DWORD *pan_len);

DWORD SREDEnable(BYTE enflag,BYTE *cmdstring,BYTE reserved);
DWORD SREDLoadKey(BYTE keyid, BYTE* pbyeKey, BYTE byLen, BYTE* byKCV);

//----------------------------------------
DWORD SMDeviceStatus(BYTE *status);
DWORD SMEPPAuth(BYTE type,
				 BYTE keyid,
				 BYTE method,
				 BYTE flag,
				 BYTE *ibuffer,
				 WORD ilen,
				 BYTE *obuffer,
				 WORD *olen);
DWORD SMReaderAuth(BYTE type,
					BYTE keyid,
					BYTE method,
					BYTE *ibuffer,
					WORD ilen,
					BYTE *obuffer,
					WORD *olen);
DWORD SMReaderLoadKey(BYTE keyid,
					BYTE *ibuffer,
					BYTE *iKCV);
DWORD SMReaderLoadKeyEx(BYTE keyid,
						BYTE *ibuffer,
						BYTE *iKCV,
						DWORD dwMode);
DWORD SMReaderGetKCV(BYTE keyid, BYTE* oKCV);
DWORD SMReaderGetInfo(BYTE *buf, DWORD *plen);					

DWORD SMReaderGenericIO(DWORD timeout,
					BYTE *buffer,
					DWORD *plen);
DWORD SMEPPUpdatePWD(BYTE adminid,
					 BYTE type,
					 BYTE *ibuffer,
					 WORD ilen,
					 BYTE *obuffer
					 );
DWORD SMReaderUpdatePWD(BYTE adminid, 
						BYTE type,
						BYTE *ibuffer,
						WORD ilen,
						BYTE *obuffer
						);
DWORD SMEPPGetLog(BYTE type,
				  DWORD* olen,
				  BYTE *obuffer
				  );
DWORD SMReaderGetLog(BYTE type,
					 DWORD* olen,
					 BYTE* obuffer
					 );
DWORD SREDSetKCVMode(BYTE kcvLen);


// FME API subroutines
DWORD FMEAuth (BYTE devID,
			   VOID *altconfig,
			   DWORD baudrate,
			   BYTE protocol);
DWORD FMEEncryptCmd (BYTE *ibuf,
					 WORD ilen,
					 BYTE *obuf,
					 WORD *olen);
DWORD FMEDecryptResp (BYTE *ibuf,
					  WORD ilen,
					  BYTE *obuf,
					  WORD *olen);


#ifdef __cplusplus
}
#endif

#endif // GTPIN_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////