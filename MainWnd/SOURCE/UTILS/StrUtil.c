#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "..\\DEFS\\constant.h"
#include "StrUtil.h"
#define FS        0X1C        // File seperator

#pragma warning(disable: 4996)
#pragma warning(disable: 4142)
#pragma warning(disable: 4018)

/****************************************************************************/
void STR_TrimSpace(char *pStr)
{
	char *p = pStr;
	int len;
	
	while (*p++ == ' '); // remove the leading spaces
	
	strcpy(pStr, --p);

	if ((len = strlen(pStr)) > 0) // remonve the trailing spaces
	{
		pStr += len;
		while (*--pStr == ' ');
		*++pStr = '\0';
	}
}

/****************************************************************************/
void STR_TrimTrailingSpace(char *pStr)
{
	int len;
	
	if ((len = strlen(pStr)) > 0) // remove the trailing spaces
	{
		pStr += len;
		while (*--pStr == ' ');
		*++pStr = '\0';
	}
}

/******************************************************************************/
void STR_itoa(int input, int len, char *pOut)
{
	char *tmpPtr;
	char locBuf[21];
	int j;

	if (len >= 20)
		return;
	// no checking on the input number so caller has to be careful with this
	tmpPtr = &locBuf[len - 1];
	for (j = 1; j <= len; j++) // do len times
	{
		*tmpPtr-- = (input - input / 10 * 10) + 48;
		input = input / 10;
	}
	tmpPtr++;
	memcpy(pOut, tmpPtr, len);
	*(pOut + len) = 0;
}

/******************************************************************************/
void STR_ltoa(long input, int len, char *pOut)
{
	char *tmpPtr;
	char locBuf[21];
	int j;

//	assert(len < 20);
	if (len >= 20)
		return;
	// no checking on the input number so caller has to be careful with this
	tmpPtr = &locBuf[len - 1];
	for (j = 1; j <= len; j++) // do len times
	{
		*tmpPtr-- = (char)((input - input / 10 * 10) + 48);
		input = input / 10;
	}
	tmpPtr++;
	memcpy(pOut, tmpPtr, len);
	*(pOut + len) = 0;
}

/******************************************************************************/
BOOL STR_IsDecimal(char *data, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (!isdigit(*data++))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/******************************************************************************/
int STR_atoi(char *data, int len)
{
	char buff[21];
	int size = strlen(data)>2?len:strlen(data);
			
	if (size > sizeof(buff) - 1)
	{
		size = sizeof(buff) - 1;
	}
	
	memcpy(buff, data, size);
	buff[size] = 0;

	if (!STR_IsDecimal(buff, size))
	{
		return 0;
	}
	
	return atoi((char*) buff);
}


/******************************************************************************/
long STR_atol(char *data, int len)
{
	char buff[21];

	int size = strlen(data)>2?len:strlen(data);
			
	if (size > sizeof(buff) - 1)
	{
		size = sizeof(buff) - 1;
	}
	
	memcpy(buff, data, size);
	buff[size] = 0;

	if (!STR_IsDecimal(buff, size))
	{
		return 0;
	}
	return atol((char*) buff);
}

/******************************************************************************/
BYTE STR_AscHexToBin(BYTE ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return (ch - '0');

	if ((ch >= 'A') && (ch <= 'F'))
		return (ch - '7');

	if ((ch >= 'a') && (ch <= 'f'))
		return (ch - 'W');

	return 0;
}

/****************************************************************************/
int STR_PackData(BYTE *packedData, char *data, int len)
{
	int i, plen;

	plen = len / 2;
	for (i = 0; i < plen; i++, packedData++, data++) // pack 2 chrs into 1 BYTE 
	{
		*packedData = STR_AscHexToBin(*data) << 4;
		*packedData += STR_AscHexToBin(*++data);
	}

	if (len % 2) // if the length is not even, get the last one
	{
		*packedData = STR_AscHexToBin(*data) << 4;
		plen++;
	}

	return (plen); // return length of packed data
}

void BCDtoChar(BYTE* data,char* target, int length)
{
	int i;
	for(i=0;i<length;i++)
	{
		*(target+i) = (*(data+i*2)-0x30)*10 + *(data+i*2 +1)-0x30;
	}
}

/******************************************************************************/
BYTE STR_BintoAscHex(BYTE ch)
{
	if ((ch & 0x0f) <= 0x09)
		return (ch + '0');
	else
		return (ch + '7');
}

/******************************************************************************/
int STR_AscToint(char *data, USHORT len)
{
	BYTE strn[21];

	memcpy(strn, data, len);
	strn[len] = '\0';
	return atoi(strn);
}

/******************************************************************************/
USHORT STR_UnpackData(char *data, BYTE *packedData, int len)
{
	int n;
	for (n = 0; n < len; n++, packedData++)
	{
		*data++ = STR_BintoAscHex((char) ((*packedData >> 4) & 0x0f));
		*data++ = STR_BintoAscHex((char) (*packedData & 0x0f));
	}
	return (len*2); // return unpacked data length
}

/******************************************************************************/
void STR_TrimZero(char *pStr)
{
	char *p = pStr;
	while (*p++ == '0'); // remove the leading zeros
	strcpy(pStr, --p);
}

/******************************************************************************/
void STR_ToUpper(char *buff, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (islower(buff[i]) != 0)
			buff[i] -= 32; // convert to uppercase before save
	}
}

/******************************************************************************/
BOOL STR_IsHex(char *hexStr, USHORT len)
{
	USHORT n;
	BYTE *p;

	p = hexStr;
	for (n = len; n > 0; n--)
	{
		if ((*p < '0' || *p > '9') && (*p < 'A' || *p > 'F'))
		{
			return FALSE;
		}
		p++;
	}
	return TRUE;
}

/******************************************************************************/
BOOL STR_IsNumeric(char *pStr, int len)
{
	USHORT n;
	char *p;
	int size = (strlen(pStr)>len)?len:strlen(pStr);
	

	p = pStr;
	for (n = size; n > 0; n--)
	{
		if (*p < '0' || *p > '9')
		{
			return FALSE;
		}
		p++;
	}
	return TRUE;
}

/******************************************************************************/
BOOL STR_IsUpper(char *data, USHORT len)
{
	USHORT n;
	BYTE *p;

	p = data;
	for (n = len; n > 0; n--)
	{
		if (*p < 'A' || *p > 'Z')
		{
			return FALSE;
		}
		p++;
	}
	return TRUE;
}

/******************************************************************************/
BOOL STR_IsSpaces(char *data, USHORT len)
{
	while (len-- > 0)
	{
		if (*data++ != ' ')
		{
			return FALSE;
		}
	}
	return TRUE;
}

/******************************************************************************/
USHORT STR_LongToBinString(ULONG num, USHORT outLen, BYTE *pOut)
{
	int i;
  memset(pOut, 0, outLen);
  for (i=outLen-1; i>=0 && num; i--)
  {
    pOut[i] = num%256;
    num = num/256;
  }
  return outLen;
}

/******************************************************************************/
USHORT STR_LongToBCDString(ULONG num, USHORT outLen, BYTE *pOut)
{
	int i;
  memset(pOut, 0, outLen);
  for (i=outLen-1; i>=0 && num; i--)
  {
    pOut[i] = num%10;
    num = num/10;
    pOut[i] += (num%10) << 4;
    num = num/10;
  }
  return outLen;
}

/******************************************************************************/
BOOL STR_IsHexOrSpaces(char *data, USHORT len)
{
	return STR_IsSpaces(data, len) || STR_IsHex(data, len);
}

/******************************************************************************/
BOOL STR_IsNumOrSpaces(char *data, USHORT len)
{
	return STR_IsSpaces(data, len) || STR_IsDecimal(data,	len);
}

/******************************************************************************/
BOOL STR_IsFlag(char* buff, USHORT len, char flag)
{
	int i;
	for (i = 0; i < len; i++)
	{	
		if (buff[i] == flag)
			return TRUE;
	}
	
	return FALSE;
}

/******************************************************************************/
int STR_FSCount(char* buff, USHORT len)
{
	USHORT i, count = 0;

	for (i = 0; i < len; i++)
	{	
		if (buff[i] == FS)
			++count;
	}	
	return count;
}

/*******************************************************************************/
void STR_BCDToNumeric(BYTE *bcdString, char *numericString, USHORT length)
{
	BYTE d;
	BOOL high;
	USHORT n;
	BYTE *p;
	char *q;

	high = !(length & 1);
	n = length;
	p = bcdString;
	q = numericString;
	while (n > 0)
	{
		if (high)
		{
			d = *p >> 4;
		}
		else
		{
			d = *p & 0xf;
			p++;
		}
		*q = '0' + d;
		q++;
		high = !high;
		n--;
	}
}

/*******************************************************************************/
void STR_NumericToBCD(char *numericString, BYTE *bcdString, USHORT length)
{
	BYTE c;
	BOOL high;
	USHORT n;
	char *p;
	BYTE *q;

	high = !(length & 1);
	p = numericString;
	q = bcdString;
	if (!high && length > 0)
	{
		*q = 0;
	}

	for (n = length; n > 0; n--)
	{
		c = *p - '0';
		if (high)
		{
			*q = c << 4;
		}
		else
		{
			*q |= c;
			q++;
		}
		high = !high;
		p++;
	}
}

/******************************************************************************/
ULONG STR_BCDUintToULONG(BYTE value)
{
	return (ULONG)(10 * (value >> 4)) + (ULONG)(value & 0xf);
}

/******************************************************************************/
BOOL STR_IsAlpha(char *pString, USHORT Length)
{
	int Index;

	/* skip white space */
	for (Index = 0; Index < Length; Index++)
	{
		if (*pString == ' ')
			pString++;
		else
			break;
	}

	for(;Index < Length; Index++)
	{
		if (!isalpha(*pString++))
		return FALSE;
	}
	return TRUE;
}

/******************************************************************************/
BOOL STR_IsAlphanumeric(char *pString, USHORT Length)
{
	int Index;

	/* skip white space */
	for (Index = 0; Index < Length; Index++)
	{
		if (*pString == ' ')
			pString++;
		else
			break;
	}

	for(;Index < Length; Index++)
	{
		if (!isalnum(*pString++))
		return FALSE;
	}
	return TRUE;
}

/******************************************************************************/
BOOL STR_IsAlphanumericOrSpaces(char *pString, USHORT Length)
{
	int Index;
	for (Index=0; Index<Length; Index++)
	{
		if (*pString != ' ')
		{
			if (!isalnum(*pString))
			{	
				return FALSE;
			}	
		}
		pString++;
	}
	return TRUE;
}

/******************************************************************************/
BOOL STR_IsANS(char *pString, USHORT Length)
{
	int Index;
	for (Index=0; Index<Length; Index++)
	{
		if (!isprint(*pString))
		{	
			return FALSE;
		}
		pString++;
	}
	return TRUE;
}



/******************************************************************************/
BOOL STR_IsAlphanumericNoSpecialChars(char *pString, USHORT Length)
{
	int Index;

	/* skip white space */
	for (Index = 0; Index < Length; Index++)
	{
		char a = *pString++;
		if (a < 0x20 || a > 0x7E)
			return FALSE;
	}

	return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* RTN NAME:   STR_Shift                                                      */
/*                                                                            */
/* FUNCTION:   Shifts a string to right in the buffer.                        */
/*                                                                            */
/* CALL:       void STR_Shift(char *buf, int len)                             */
/*                                                                            */
/* ARGS:       char *buf        - pointer to the string to operate on.        */
/*             int  len         - number of bytes to be shifted.              */
/*                                                                            */
/* RETURN:     NONE                                                           */
/*                                                                            */
/******************************************************************************/
void STR_Shift(char *buf, int len)
{
	char *gpStrPtr;
	int giLen = strlen(buf); /* length of the string */
	buf += giLen; /* source copy pointer */
	gpStrPtr = buf + len;

	while (giLen-- >= 0)
		*(gpStrPtr--) = *(buf--);
}


/******************************************************************************/
/*                                                                            */
/* RTN NAME:   STR_Pad                                                        */
/*                                                                            */
/* FUNCTION:   Pads a string with a given character to the specified length.  */
/*             Padding may be specified to be either "leading" or "trailing". */
/*             Works only on null-terminated strings.                         */
/*                                                                            */
/* CALL:       void STR_Pad(char *buf, char padchar, int len, int padtype)    */
/*                                                                            */
/* ARGS:       char *buf        - Pointer to the string to operate on.        */
/*             char padchar     - character to use when padding.              */
/*             int  len         - Length to pad string to.                    */
/*             int  padtype     - Padding Type  (PAD_LEADING | PAD_TRAILING) */
/*                                                                            */
/* RETURN:     NONE                                                           */
/*                                                                            */
/* NOTES:      1.  Move all characters in the destination string from the     */
/*                 insertion point on forward enough characters to make room  */
/*                 for the insertion string.  Append NULL terminator.         */
/*                                                                            */
/*             2.  Copy each character in the insertion string to it's proper */
/*                 position within the destination string.                    */
/*                                                                            */
/******************************************************************************/
void STR_Pad(char *buf, char padchar, int len, char padtype)
{
	int giLen = strlen(buf); /* Old length of string buffer  */
	int giPadLen = len - giLen; /* number of chars to be padded */

	if (giPadLen <= 0) /* If no padding required then exit */
		return;

	/* Determine padding point:  start or end of string */
	if (padtype == PAD_LEADING) /* pad at the begging */
	{
		STR_Shift(buf, giPadLen); /* make room for sub-string */
	}
	else
	{
		buf += giLen; /* pad at the end */
		*(buf + giPadLen) = '\0'; /* insert NULL terminator */
	}

	memset(buf, padchar, giPadLen); /* pad the character */
}
