#pragma once

// CPinInput dialog
#include "..\\..\\MainWnd.h"

class CPinInput : public CDialog
{
	DECLARE_DYNAMIC(CPinInput)

public:
	CPinInput(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPinInput();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	enum { IDD = IDD_INPUT_KEY };

	BOOL m_bCheckCardRemoved;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};
