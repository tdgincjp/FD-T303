
#pragma once
class CStaticText : public CStatic
{
// Construction
public:
	CStaticText();
    virtual ~CStaticText();
// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CStaticText)
//}}AFX_VIRTUAL
// Implementation
public:
	void SetPointFont(bool bChangeFont = true, int nPointSize = 100,
	LPCTSTR lpszFaceName = TEXT("Microsoft Sans Serif"));
	void SetColor(COLORREF color = RGB(0, 0, 0),COLORREF bkcolor = RGB(255,255,255) );
	void SetCaption(CString szCaption);
	bool m_bChangeFont;
	int m_nPointSize;

	int m_nBtType;  //0-normal, 1- left button, 2- right button
	UINT m_nFormat;  
	LPCTSTR m_lpszFaceName; 
	COLORREF m_color; 
	COLORREF m_bkcolor; 
	 CString m_szCaption;
	 CString m_szCaption1;
// Generated message map functions
protected:
	//{{AFX_MSG(CStaticText)
	afx_msg void OnPaint();
//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
 

