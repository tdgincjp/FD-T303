#ifndef CALLBACKFUN_H
#define CALLBACKFUN_H

#ifdef	__cplusplus
extern "C" {
#endif
//PinEntry 
WORD ProcessRevTLV(USHORT tlvLength, BYTE *tlv);
WORD ProcessTLV_6F(USHORT tlvLength, BYTE *tlv);

WORD SelectApp(CANDIDATE_LIST *pCandidate, PDWORD pData);
WORD UserCertVerify(PCARDHOLDERCHECK pCardHoldInfo);
WORD ScrRW(BYTE *pbyBuffer, WORD *pwLen, int nPoint);
WORD ReadLogInfo(LPVOID lpVoid, WORD wPoint);
WORD DisplayMsg(BYTE byMsgIndex);
WORD VoiceReferral(BYTE *pbyBuf, BYTE *pbyLen, BYTE byOption);

WORD PINEntryProcess(BYTE byIsLastTry,BYTE *pbyAmount, BYTE byPanLen, BYTE *pbyPan, 
				 BYTE byType, BYTE *pbyPubKey, BYTE byPubKeyLen, 
				 DWORD dwExponents,BYTE *pbyChallenge,BYTE* pbyPinBlock);
WORD PINEntryPro(void* pPinEntry1);
WORD MultiCallBack(WORD wCBType, WPARAM wParam, LPARAM lParam);
#ifdef	__cplusplus
}
#endif
#endif
