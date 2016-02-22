#pragma once

// CTagUtil
class CTagUtil
{
// Construction
public:
	CTagUtil();	// standard constructor

public:
	static int CTagUtil::AddToTlvBuff(char *Tag, USHORT TagLen, BYTE *pTagData, BYTE *pTargetBuff, USHORT MaxBuffLen);
	static int CTagUtil::ReadFromTlvBuff(BYTE *pTagBuff, char *Tag, BYTE *pTagData);
	static int CTagUtil::AccessDataInTlvBuff(BYTE *pTagBuff, char *Tag, BYTE **pData);
	static void CTagUtil::RemoveFromTlvBuff(BYTE *pTagBuff, char *Tag);
	static void CTagUtil::ClearTlvBuff(BYTE *pTagBuff);

private:
	USHORT GetUsedLenInTlvBuff(BYTE *pTagBuff);

};