#include "StdAfx.h"
#include "MyButton.h"


//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CMyButton::CMyButton(void)
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CMyButton::~CMyButton(void)
{
}
BEGIN_MESSAGE_MAP(CMyButton, CWnd)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
//!	\Set button value
//!	\param	index - button id 
//!	\param	text - button text
//!	\param	bmp - button image
//-----------------------------------------------------------------------------
void  CMyButton::SetValue(int index , CString text,int bmp)
{
	m_index = index;
	m_text = text;
	m_bmp = bmp;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CMyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);		//Get device context object
	CRect rt;
	rt = lpDrawItemStruct->rcItem;		//Get button rectx

	UINT state = lpDrawItemStruct->itemState;	//Get state of the button
	if ( (state & ODS_SELECTED) )		// If it is pressed
	{
		dc.FillSolidRect(rt, RGB(255,255,255));		//Fill button with blue color
		dc.SetTextColor(RGB(0,0,0));		// Set the color of the caption to be yellow
	}
	else
	{
		dc.FillSolidRect(rt, RGB(0, 0, 0));		//Fill button with blue color

		dc.SetTextColor(RGB(255,255,255));		// Set the color of the caption to be yellow

	}

	CFont font;
	font.CreatePointFont(120, _T("MS Sans Serif"));
	CFont* def_font = dc.SelectObject(&font);
	if(m_index != -1)
	{
		CString m_strText;
		m_strText.Format(L"%d",m_index);
		dc.ExtTextOut(2, 2, ETO_CLIPPED, NULL, m_strText, wcslen(m_strText), NULL );
	}


	CFont font1;
	font1.CreatePointFont(150, _T("MS Sans Serif"));
	dc.SelectObject(&font1);
	CString strTemp;

	strTemp = m_text;

	CRect rt1(rt);
	rt1.left += 12;
	dc.DrawText(strTemp,rt1,DT_LEFT|DT_VCENTER|DT_SINGLELINE);		// Draw out the caption
	dc.SelectObject(def_font);
	font.DeleteObject();
	font1.DeleteObject();

	if(m_bmp != 0)
	{
		CBitmap bmp;
		bmp.LoadBitmap(m_bmp);//IDB_OK);
		CDC LCDdcMem;
		LCDdcMem.CreateCompatibleDC(&dc);
		LCDdcMem.SelectObject(&bmp);
		dc.BitBlt(rt.right-28,rt.top+(rt.Height()-24)/2,24,24,&LCDdcMem,0,0,SRCCOPY);
	}
	dc.Detach();
}
