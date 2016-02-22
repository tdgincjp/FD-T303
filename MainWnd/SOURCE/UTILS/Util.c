#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include "SaioBase.h"
#include "util.h"
#include "..\\defs\\constant.h"

#pragma warning(disable: 4996)

/******************************************************************************/
static BOOL IsStrDigit(char *str)
{
	USHORT i;

	for (i = 0; i < strlen(str); i++)
	{	
		if (str[i] < '0' || str[i] > '9')
			return FALSE;
	}
	
	return TRUE;
}

/******************************************************************************/
BYTE UTIL_CalculateLrc(BYTE *buff, USHORT len)
{
	USHORT i;
	BYTE lrc;

	lrc = 0;
	for (i = 0; i < len; i++)
	{
		lrc ^= buff[i];
	}
	
	return lrc;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    UTIL_CompareAmount                                             */
/*                                                                            */
/* FUNCTION:    compare amount 1 and amount 2                                 */
/*                                                                            */
/*      ARGS:       amount1 - amount 1                                        */
/*                              amount2 - amount 2                            */
/*                                                                            */
/* RETURNS:     1 -- amount 1 > amount 2                                      */
/*              0 -- amount 1 = amount 2                                      */
/*             -1 -- amount 1 < amount 2                                      */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int UTIL_CompareAmount(char *amount1, char *amount2)
{
	long amt1, amt2;

	amt1 = atol(amount1);
	amt2 = atol(amount2);

	if (amt1 > amt2)
		return 1;
	else if (amt1 == amt2)
		return 0;
	else
		return -1;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    UTIL_FormatAmount                                              */
/*                                                                            */
/* FUNCTION:    format amount                                                 */
/*                                                                            */
/*      ARGS:                                                                 */
/*                                                                            */
/* RETURNS:                                                                   */
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void UTIL_FormatAmount(char *i_amount, char *o_amount, char sign)
{
	int len;

	*o_amount = '\0';
	if (!IsStrDigit(i_amount))
		return;

	*o_amount++ = sign;

	while (*i_amount++ == '0'); // remove the leading zeros
	i_amount--;

	len = strlen(i_amount);
	if (len > 8)
	{
		len = len - 8;
		memcpy(o_amount, i_amount, len);
		o_amount += len;
		i_amount += len;
		*o_amount++ = ',';
	}

	len = strlen(i_amount);
	if (len > 5)
	{
		len = len - 5;
		memcpy(o_amount, i_amount, len);
		o_amount += len;
		i_amount += len;
		*o_amount++ = ',';
	}

	len = strlen(i_amount);
	if (len > 2)
	{
		memcpy(o_amount, i_amount, len - 2);
		o_amount += len - 2;
		*o_amount++ = '.';
		strcpy(o_amount, i_amount + len - 2);
	}
	else
	{
		strcpy(o_amount, "0.00");
		strcpy(o_amount + 4 - len, i_amount);
	}
}

/******************************************************************************/
void UTIL_FormatAmountWithoutComma(char *i_amount, char *o_amount, char sign)
{
	int len;

	*o_amount = '\0';
	if (!IsStrDigit(i_amount))
		return;

	*o_amount++ = sign;

	while (*i_amount++ == '0')
		; /* remove the leading zeros */
	i_amount--;

	len = strlen(i_amount);
	if (len > 2)
	{
		memcpy(o_amount, i_amount, len - 2);
		o_amount += len - 2;
		*o_amount++ = '.';
		strcpy(o_amount, i_amount + len - 2);
	}
	else
	{
		strcpy(o_amount, "0.00");
		strcpy(o_amount + 4 - len, i_amount);
	}
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    UTIL_AddAmount                                                */
/*                                                                            */
/* FUNCTION:    amount 2 = amount 1 + amount 2                                */
/*                                                                            */
/* ARGS:        amount1 - amount 1                                            */
/*              amount2 - amount 2                                            */
/*                                                                            */
/* RETURNS:                                                                   */
/*                                                                            */
/* NOTES:     must ensure that amount2 is large enough. (SZ_TOTAL)            */
/*                                                                            */
/******************************************************************************/
void UTIL_AddAmount(char *amount1, char *amount2)
{
	char total[SZ_TOTAL + 1];
	long sum;
	int i;

	if (!IsStrDigit(amount1) || !IsStrDigit(amount2))
		return;

	sum = atoi(amount1) + atoi(amount2);

	memset(total, '0', SZ_AMOUNT);
	total[SZ_TOTAL] = '\0';

	for (i = SZ_TOTAL - 1; i >= 0; i--)
	{
		total[i] = (char) (sum % 10 + '0');
		sum = sum / 10;
		if (sum <= 0)
			break;
	}

	strcpy(amount2, &total[i]);
}

/******************************************************************************/
BOOL CheckMod10(char *account, USHORT len)
{
	char *pAccount;
	USHORT n, mod;

	pAccount = account + len - 1;			// pointer to the check digit
	mod = 0;

	while (pAccount >= account)
	{
		mod += ((*pAccount--) & 0x0F);		// add the odd digit

		if (pAccount >= account)
		{
			n = 2 * ((*pAccount--) & 0x0F); // double the even digit
			mod += n/10 + n%10;							// add individual digits
		}
	}

	if (mod %= 10)	// get unit digit
		return FALSE; // not 0, error
	else
		return TRUE;	// 0, check ok
}

