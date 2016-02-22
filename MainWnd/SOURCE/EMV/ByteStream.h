
// ByteStream.h: interface for the CByteStream class.

#if !defined(AFX_BYTESTREAM_H__7B0E6B8B_36D7_42D0_AFA2_F6C5E43536B2__INCLUDED_)
#define AFX_BYTESTREAM_H__7B0E6B8B_36D7_42D0_AFA2_F6C5E43536B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #ifdef EMV_CLASS_BASE
// #define EMV_BASE __declspec(dllexport)
// #else
// #define EMV_BASE __declspec(dllimport)
// #endif

class CByteStream  
{
public:
	CByteStream()
	{
		m_pData = NULL;
		m_nSize = 0;
		m_nMaxSize = 0;
	}

	CByteStream(LPCSTR lpsz)
	{
		m_pData = NULL;
		m_nSize = 0;
		m_nMaxSize = 0;

		int nSrcLen = (lpsz != NULL ? strlen(lpsz) : 0);
		if (nSrcLen != 0)
		{
			Empty();
			Add(lpsz, nSrcLen);
		}
	}

	CByteStream(const CByteStream &ByteStream)
	{
		m_pData = NULL;
		m_nSize = 0;
		m_nMaxSize = 0;
		
		if (ByteStream.GetLength()>0)
			Add(ByteStream.GetBuffer(), ByteStream.GetLength());
	}
	
	virtual ~CByteStream()
	{
		Empty();
	}
	
	void Write(const void* lpBuf, UINT nLen)
	{
		if(nLen <= 0)
			return;
		
		int nIndex = 0;
		if(m_nSize > 0)
			nIndex = m_nSize;
		
		SetSize(m_nSize+nLen);
		memcpy(&m_pData[nIndex], lpBuf, nLen);
	}

	void Add(const void* lpBuf, int nLen)
	{
		if (nLen <= 0)
			return;
		
		int nIndex = 0;
		if (m_nSize > 0)
			nIndex = m_nSize;
		
		SetSize(m_nSize+nLen);
		memcpy(&m_pData[nIndex], lpBuf, nLen);
	}
	
	void AddTLV(int Tag, int Len, const void *Value)
	{
		BYTE temp[2];
		int len;
		
		if (Len <= 0)
			return;
		
		// Add Tag
		if (Tag >= 0x100)	// => 0x100 means at least 2 bytes
		{
			temp[0] = Tag / 0x100;
			temp[1] = Tag % 0x100;
			len = 2;
		}
		else
		{
			temp[0] = Tag % 0x100;
			len = 1;
		}
		Add(temp, len);
				
		// Add Length
		if (Len > 0x7F)
		{
			temp[0] = 0x81;
			temp[1] = Len;
			len = 2;
		}
		else
		{
			temp[0] = Len;
			len = 1;
		}
		Add(temp, len);

		// Add Value
		Add(Value, Len);
	}

	void AddCmdTLV(int Tag, int Len, const void *Value)
	{
		BYTE temp[2];
		int len;
		
		if (Len <= 0)
			return;
		
		// Add Tag
		if (Tag >= 0x100)	// => 0x100 means at least 2 bytes
		{
			temp[0] = Tag / 0x100;
			temp[1] = Tag % 0x100;
			len = 2;
		}
		else
		{
			temp[0] = Tag % 0x100;
			len = 1;
		}
		Add(temp, len);
				
		// Add Length
		if (Len > 0x100)
		{
			temp[0] = Tag / 0x100;
			temp[1] = Tag % 0x100;
		}
		else
		{
			temp[0] = 0;
			temp[1] = Len;
		}
		len = 2;
		Add(temp, len);

		// Add Value
		Add(Value, Len);
	}


	int Compare(PBYTE pData, int nLen)
	{
		return memcmp(pData, m_pData, nLen);
	}
	
	int Compare(LPSTR pStr)
	{
		return memcmp(pStr, m_pData, strlen(pStr));
	}
	int Compare(CByteStream *pData)
	{
		return Compare(pData->GetBuffer(), pData->GetLength());
	}
	void Empty()
	{
		SetSize(0);
	}
	BOOL IsEmpty()
	{
		if (m_nSize>0)
			return FALSE;
		else
			return TRUE;
	}
	int GetLength() const
	{
		return m_nSize;
	}	
	
	BYTE* GetBuffer() const
	{
		return m_pData;
	}
	
	// insertion operations
	CByteStream& operator<<(BYTE by)
	{
		BYTE byTemp[4] = {0};
		byTemp[0] = by;
		
		Add(byTemp, 1);
		
		return *this;
	}
	CByteStream& operator<<(char ch)
	{
		BYTE byTemp[4] = {0};
		byTemp[0] = ch;
		
		Add(byTemp, 1);
		
		return *this;
	}
	
	CByteStream& operator<<(LPCSTR psz)
	{
		Add(psz, strlen(psz));
		
		return *this;
	}

	CByteStream& operator<<(const CByteStream &ByteStream)
	{
		Add(ByteStream.GetBuffer(), ByteStream.GetLength());

		return *this;
	}

	CByteStream& operator<<(DWORD dw)
	{
		PDWORD pDW = NULL;
		
		int nIndex = 0;
		if (m_nSize > 0)
			nIndex = m_nSize;
		
		SetSize(m_nSize + sizeof(dw));
		
		pDW = (PDWORD)(&m_pData[nIndex]);
		*pDW = dw;
		
		return *this;
	}	
	
	CByteStream& operator=(const CByteStream &ByteStream)
	{		
		Empty();
		Add(ByteStream.GetBuffer(), ByteStream.GetLength());
		
		return *this;
	}

	CByteStream& operator=(LPCSTR psz)
	{
		Empty();
		Add(psz, strlen(psz));
		
		return *this;
	}
	
	CByteStream& operator+=(const CByteStream &ByteStream)
	{		
		Add(ByteStream.GetBuffer(), ByteStream.GetLength());

		return *this;
	}

	//	void operator<<(WORD w);
	//	void operator<<(LONG l);
	//	void operator<<(DWORD dw);
	//	void operator<<(float f);
	//	void operator<<(double d);
	//
	//	void operator<<(int i);
	//	void operator<<(short w);
	//	void operator<<(unsigned u);
	
	//	// extraction operations
	//	CByteStream& operator>>(BYTE& by);
	//	CByteStream& operator>>(WORD& w);
	//	CByteStream& operator>>(DWORD& dw);
	//	CByteStream& operator>>(LONG& l);
	//	CByteStream& operator>>(float& f);
	//	CByteStream& operator>>(double& d);
	//
	//	CByteStream& operator>>(int& i);
	//	CByteStream& operator>>(short& w);
	//	CByteStream& operator>>(char& ch);
	//	CByteStream& operator>>(unsigned& u);
	
protected:
	BYTE* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;
	
	void SetSize(int nNewSize)
	{
		if (nNewSize < 0)
			return;
		
		int m_nGrowBy = 0;
		
		if (nNewSize == 0)
		{
			// shrink to nothing
			if (m_pData!=NULL)
				delete[] (BYTE*)m_pData;
			m_pData = NULL;
			m_nSize = m_nMaxSize = 0;
		}
		else if (m_pData == NULL)
		{
			// create one with exact size
#ifdef SIZE_T_MAX
			ASSERT(nNewSize <= SIZE_T_MAX/sizeof(BYTE));    // no overflow
#endif
			m_pData = (BYTE*) new BYTE[nNewSize * sizeof(BYTE)];
			
			memset(m_pData, 0, nNewSize * sizeof(BYTE));  // zero fill
			
			m_nSize = m_nMaxSize = nNewSize;
		}
		else if (nNewSize <= m_nMaxSize)
		{
			// it fits
			if (nNewSize > m_nSize)
			{
				// initialize the new elements
				
				memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(BYTE));
				
			}
			
			m_nSize = nNewSize;
		}
		else
		{
			// otherwise, grow array
			int nGrowBy = m_nGrowBy;
			if (nGrowBy == 0)
			{
				// heuristically determine growth when nGrowBy == 0
				//  (this avoids heap fragmentation in many situations)
				nGrowBy = min(1024, max(4, m_nSize / 8));
			}
			int nNewMax;
			if (nNewSize < m_nMaxSize + nGrowBy)
				nNewMax = m_nMaxSize + nGrowBy;  // granularity
			else
				nNewMax = nNewSize;  // no slush
			
			ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
			ASSERT(nNewMax <= SIZE_T_MAX/sizeof(BYTE)); // no overflow
#endif
			BYTE* pNewData = (BYTE*) new BYTE[nNewMax * sizeof(BYTE)];
			
			// copy new data from old
			// WCE_FCTN(memcpy)(pNewData, m_pData, m_nSize * sizeof(BYTE));
			memcpy(pNewData, m_pData, m_nSize * sizeof(BYTE));
			
			// construct remaining elements
			ASSERT(nNewSize > m_nSize);
			
			memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(BYTE));
			
			
			// get rid of old stuff (note: no destructors called)
			delete[] (BYTE*)m_pData;
			m_pData = pNewData;
			m_nSize = nNewSize;
			m_nMaxSize = nNewMax;
		}
	}	
};

#endif // !defined(AFX_BYTESTREAM_H__7B0E6B8B_36D7_42D0_AFA2_F6C5E43536B2__INCLUDED_)
