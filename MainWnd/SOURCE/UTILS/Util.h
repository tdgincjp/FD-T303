/* 
 * File:   Utils.h
 * Author: tan
 *
 * Created on June 11, 2012, 9:57 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <Windows.h>

#ifdef	__cplusplus
extern "C" {
#endif

BYTE UTIL_CalculateLrc(BYTE *buff, USHORT len);
void UTIL_FormatAmount(char *i_amount, char *o_amount, char sign);
void UTIL_FormatAmountWithoutComma(char *i_amount, char *o_amount, char sign);
void UTIL_AddAmount(char *amount1, char *amount2);
INT UTIL_CompareAmount(char *amount1, char *amount2);
BOOL CheckMod10(char *account, USHORT len);

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

