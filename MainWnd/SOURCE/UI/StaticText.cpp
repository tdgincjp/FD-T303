#include "stdafx.h"
#include "StaticText.h"

CStaticText::CStaticText()
    : m_szCaption(_T(""))
	, m_szCaption1(_T(""))
    , m_color(RGB(0, 0, 0))
    , m_bkcolor(RGB(255,255,255))
    , m_nPointSize(100)
    , m_lpszFaceName(_T("Arial"))
    , m_bChangeFont(false)
, m_nFormat(DT_LEFT)
{ 
	m_nBtType = 0;
}
CStaticText::~CStaticText()
{
}

BEGIN_MESSAGE_MAP(CStaticText, CStatic)
//{{AFX_MSG_MAP(CStaticText)
ON_WM_PAINT()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CStaticText message handlers
//-----------------------------------------------------------------------------
//!	\CStaticText class virtual function
//-----------------------------------------------------------------------------
void CStaticText::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rc;
	GetClientRect(&rc);
	dc.FillSolidRect(rc, RGB( 255,255,255));

	int iTriSpace = rc.Height()/2;
	if ( m_nBtType == 1)
	{
		CPen newpen;
		newpen.CreatePen(PS_SOLID,1,m_bkcolor);
		CPen *oldpen = dc.SelectObject(&newpen);

		int iTemp = iTriSpace;
		for (int i= 1;i<iTriSpace; i++)
		{
			dc.MoveTo(i, iTemp-i);
			dc.LineTo(i, iTemp+i);
		}

		dc.SelectObject(oldpen);
		rc.left += iTriSpace;
	}
	else if( m_nBtType == 2)
	{
		CPen newpen;
		newpen.CreatePen(PS_SOLID,1,m_bkcolor);
		CPen *oldpen = dc.SelectObject(&newpen);

		int iTemp = rc.Width()-iTriSpace;
		for (int i= 0;i<iTriSpace; i++)
		{
			dc.MoveTo(iTemp+i, i);
			dc.LineTo(iTemp+i, rc.Height()-i);
		}

		dc.SelectObject(oldpen);

		m_nFormat = DT_RIGHT;
		rc.right -= iTriSpace;
	}

	dc.FillSolidRect(rc, m_bkcolor);

	CFont font;
	CFont* pOldFont;
	int PointSize = m_nPointSize;
	CSize sz;
	do
	{
		font.CreatePointFont(PointSize, m_lpszFaceName, NULL);
		pOldFont = dc.SelectObject(&font);

		sz=dc.GetTextExtent(m_szCaption);
		if (sz.cx > (rc.right - rc.left))
		{
			PointSize -= 20;
			font.DeleteObject();
		}
		else
			break;
	}while(TRUE);
	dc.SetTextColor(m_color);
	dc.SetBkColor(m_bkcolor); 

	if( m_nBtType > 0)
		rc.top += 6;

	dc.DrawText(m_szCaption, &rc, m_nFormat);
	if (m_szCaption1 != L"")
		dc.DrawText(m_szCaption1, &rc, m_nFormat==DT_LEFT?DT_RIGHT:DT_LEFT);
	
	dc.SelectObject(pOldFont);
	font.DeleteObject();
}

//-----------------------------------------------------------------------------
//!	\Set Caption string
//!	\param	szCaption - caption string 
//-----------------------------------------------------------------------------
void CStaticText::SetCaption(CString szCaption)
{
	if(m_szCaption == szCaption && m_szCaption1==L"")
		return;
 
    m_szCaption = szCaption;
	Invalidate(TRUE);
}

//-----------------------------------------------------------------------------
//!	\Set color
//!	\param	color - caption string color
//!	\param	bkcolor - back ground color
//-----------------------------------------------------------------------------
void CStaticText::SetColor(COLORREF color /* = RGB(0, 0, 0)*/,COLORREF bkcolor )
{
    m_color = color;
	m_bkcolor = bkcolor;
}

//-----------------------------------------------------------------------------
//!	\Set Caption string Font and size
//!	\param	bChangeFont - font 
//!	\param	nPointSize - font size 
//!	\param	lpszFaceName - caption string face name
//-----------------------------------------------------------------------------
void CStaticText::SetPointFont(bool bChangeFont, int nPointSize, LPCTSTR lpszFaceName)
{
    m_bChangeFont = bChangeFont;
	m_nPointSize = nPointSize;
	m_lpszFaceName = lpszFaceName;
}
