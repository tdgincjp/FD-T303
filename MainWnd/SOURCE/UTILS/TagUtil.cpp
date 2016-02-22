#include "stdafx.h"
#include <stdio.h>
#include "TagUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable: 4996)

#define assure(x,y) {long r; r = (x); if (!(r y)) {assert(FALSE);}}

CTagUtil::CTagUtil()
{
}

int CTagUtil::AddToTlvBuff(char *Tag, USHORT TagLen, BYTE *pTagData, BYTE *pTargetBuff, USHORT MaxBuffLen)
{
	BYTE *p;
	USHORT currLen;

	p = pTargetBuff;
	currLen = (*p++) * 256;
	currLen += *p++;

	assure(TagLen+4 + currLen+2, < MaxBuffLen);
	if (TagLen)
	{
		p += currLen;
		*p++ = Tag[0];
		*p++ = Tag[1];
		*p++ = TagLen / 256;
		*p++ = TagLen % 256;
		memcpy(p, pTagData, TagLen);

		currLen += TagLen + 4;
		*pTargetBuff++ = currLen / 256;
		*pTargetBuff = currLen % 256;
	}
	return currLen;
}

/******************************************************************************/
int CTagUtil::ReadFromTlvBuff(BYTE *pTagBuff, char *Tag, BYTE *pTagData)
{
	int len;
	BYTE *p;

	len = AccessDataInTlvBuff(pTagBuff, Tag, &p);

	if (len)
	{
		memcpy(pTagData, p, len);
		pTagData[len] = 0;					//JC Null terminate Tag Data May 13/15
	}

	return len;
}

/******************************************************************************/
int CTagUtil::AccessDataInTlvBuff(BYTE *pTagBuff, char *Tag, BYTE **pData)
{
	USHORT totalLen, dataLen;
	char dataTag[3];
	BYTE *p;

	memset(dataTag, 0, sizeof(dataTag));
	p = pTagBuff;
	totalLen = (*p++) * 256;
	totalLen += *p++;

	while (totalLen)
	{
		dataTag[0] = *p++;
		dataTag[1] = *p++;
		dataLen = (*p++) * 256;
		dataLen += *p++;
		assure(totalLen, >= (dataLen+4));
		if (strcmp(Tag, dataTag) == 0)
		{
			*pData = p;
			return dataLen;
		}
		p += dataLen;
		totalLen -= (dataLen + 4);
	}

	*pData = NULL;
	return 0;
}

/******************************************************************************/
void CTagUtil::RemoveFromTlvBuff(BYTE *pTagBuff, char *Tag)
{
	USHORT totalLen, dataLen, remainingLen;
	BYTE *p;

	p = pTagBuff;
	totalLen = (*p++) * 256;
	totalLen += *p++;

	dataLen = AccessDataInTlvBuff(pTagBuff, Tag, &p);

	if (dataLen)
	{
		p -= 4; // point to the Tag
		dataLen += 4;

		remainingLen = totalLen - (p - pTagBuff - 2) - dataLen;
		if (remainingLen)
		{
			memcpy(p, p + dataLen, remainingLen);
		}
		totalLen -= dataLen;
		*pTagBuff++ = totalLen / 256;
		*pTagBuff = totalLen % 256;
	}
}

/******************************************************************************/
void CTagUtil::ClearTlvBuff(BYTE *pTagBuff)
{
	memset(pTagBuff, 0, 2);
}

/******************************************************************************/
USHORT CTagUtil::GetUsedLenInTlvBuff(BYTE *pTagBuff)
{
	USHORT totalLen;
	totalLen = (*pTagBuff++) * 256;
	totalLen += *pTagBuff + 2;
	return totalLen;
}



