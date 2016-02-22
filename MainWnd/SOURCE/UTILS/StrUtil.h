/* 
 * File:   StrUtil.h
 * Author: tan
 *
 * Created on June 5, 2012, 1:31 PM
 */

#ifndef STRUTIL_H
#define	STRUTIL_H

#include <Windows.h>


#ifdef	__cplusplus
extern "C" {
#endif

#define PAD_LEADING         0	
#define PAD_TRAILING        1

void STR_TrimSpace(char *pStr);
void STR_TrimTrailingSpace(char *pStr);
void STR_itoa(int input, int len, char *pOut);
void STR_ltoa(long input, int len, char *pOut);
BOOL STR_IsDecimal(char *data, int len);
int STR_atoi(char *data, int len);
long STR_atol(char *data, int len);
BYTE STR_AscHexToBin(BYTE ch);
int STR_PackData(BYTE *packedData, char *data, int len);
char STR_BintoAscHex(BYTE ch);
int STR_AscToint(char *data, USHORT len);
USHORT STR_UnpackData(char *data, BYTE *packedData, int len);
void STR_TrimZero(char *pStr);
void STR_ToUpper(char *buff, int len);
BOOL STR_IsHex(char *hexStr, USHORT len);
BOOL STR_IsNumeric(char *pStr, int len);
BOOL STR_IsUpper(char *data, USHORT len);
BOOL STR_IsSpaces(char *data, USHORT len);
USHORT STR_LongToBinString(ULONG num, USHORT outLen, BYTE *pOut);
USHORT STR_LongToBCDString(ULONG num, USHORT outLen, BYTE *pOut);
BOOL STR_IsHexOrSpaces(char *data, USHORT len);
BOOL STR_IsNumOrSpaces(char *data, USHORT len);
BOOL STR_IsFlag(char* buff, USHORT len, char flag);
int STR_FSCount(char* buff, USHORT len);
void STR_BCDToNumeric(BYTE *bcdString, char *numericString, USHORT length);
void STR_NumericToBCD(char *numericString, BYTE *bcdString, USHORT length);
ULONG STR_BCDUintToULONG(BYTE value);
BOOL STR_IsAlpha(char *pString, USHORT Length);
BOOL STR_IsAlphanumeric(char *pString, USHORT Length);
BOOL STR_IsAlphanumericOrSpaces(char *pString, USHORT Length);
BOOL STR_IsANS(char *pString, USHORT Length);
BOOL STR_IsAlphanumeric(char *pString, USHORT Length);
BOOL STR_IsAlphanumericOrSpaces(char *pString, USHORT Length);
BOOL STR_IsAlphanumericNoSpecialChars(char *pString, USHORT Length);
void STR_Shift(char *buf, int len);
void STR_Pad(char *buf, char padchar, int len, char padtype);

void BCDtoChar(BYTE* data,char* target, int length);



#ifdef	__cplusplus
}
#endif

#endif	/* STRUTIL_H */

