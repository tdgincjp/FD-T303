#pragma once
#include "afxwin.h"

class CMyButton :
	public CButton
{
private:
	int m_index;
	int m_bmp;

	int m_iTime;

public:
	CMyButton(void);
	virtual ~CMyButton(void);

	CString m_text;

	void SetValue(int index , CString text,int bmp=0);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
};
