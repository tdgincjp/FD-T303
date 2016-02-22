#ifndef EMVAPI_H
#define EMVAPI_H

#include "Common.h"

#ifdef __cplusplus

#define EMV_API extern "C" __declspec (dllexport)

#else

#define EMV_API __declspec (dllimport)

#endif


EMV_API WORD EmvStart(PTXNCONDITION pTxnCondition);
//1 AppSel
EMV_API WORD EmvApplicationSelection();
//2 InitApp
EMV_API WORD EmvInitiateApplication();
//3 Read App Data
EMV_API WORD EmvReadApplicationData();
//4 Offline Data Authentication
EMV_API WORD EmvOfflineDataAuthentication();
//5 Process Restriction
EMV_API WORD EmvProcessingRestrictions();
//6 Cardholder Verification
EMV_API WORD EmvCardholderVerification();
//7 Terminal Risk Management
EMV_API WORD EmvTerminalRiskManagement();
//8 Terminal action analysis
EMV_API WORD EmvTerminalActionAnalysis(BYTE *pbyResult, BYTE byForceAAC=FALSE);
//9 Online Processing
EMV_API WORD EmvOnlineProcessing(BYTE byOnlineStatus, PAUTHRESPONSE pARPCResponse);
// //10 Completion
EMV_API WORD EmvCompletion(BYTE *pTransResult, BYTE byForceAAC=FALSE);
EMV_API WORD EmvStop();

//Get Data Capture type
EMV_API WORD EmvGetDataCaptureType(BYTE *pbyDCType);
//Get Version API&Kernel
EMV_API WORD EmvGetApiVersion(BYTE *pbyVer, BYTE byLen);
EMV_API WORD EmvGetKernelVersion(BYTE *pbyVer, BYTE byLen);
//Calculate Check sum 
EMV_API WORD CalCheckSum();

EMV_API WORD EmvSetIcsValue(DWORD dwType, BYTE byValue);
EMV_API WORD EmvEditTransactionType(WORD wType);
EMV_API WORD EmvGetAidUserData(BYTE byAidIndex, BYTE *pbyTag, BYTE *pbyBuf, WORD *pwBufLen);

//Data Storage
EMV_API WORD EmvGetParam(BYTE *pbyTags, WORD wTagLen, BYTE *pbyTLV, WORD *pwTLVLen);
EMV_API WORD EmvSetParam(BYTE *pbyTLV, WORD wTLVLen);
// Get specific profile data
EMV_API WORD EmvGetProfile(BYTE byType, WORD wIndex, BYTE *pbyBuffer, WORD *pwBufLen);
// Set profile data (add/overwrite)
EMV_API WORD EmvSetProfile(BYTE byType, WORD *pwIndexRet, BYTE *pbyData, WORD wDataLength);
// Get the index of first profile and number of profiles, 0xFFFF for no available profiles
EMV_API WORD EmvGetFirstProfile( BYTE byType, WORD *pwFileIndex );
// Get the next available profile, 0xFFFF for no available profiles
EMV_API WORD EmvGetNextProfile( WORD *pwFileIndex );
// Delete specific profile
EMV_API WORD EmvDelProfile (BYTE byType, WORD wFileIndex );

//CallBack reg
EMV_API void CB_EmvCallbackReg(WORD (*pFun)(WORD wCBType, WPARAM wParam, LPARAM lParam));

#endif
