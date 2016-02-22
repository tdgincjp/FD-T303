// Transactions.h : header file
//

#pragma once
#include "afxwin.h"
#include "..\\host\\host.h"
#include "finan.h"
#include "admin.h"


// CECR dialog
class CTransaction
{	
public:
	CTransaction(void);
	~CTransaction(void);
	void DoTransaction(ECR_TranDataRec *pEcrRec);
	int GetStatus();
	TRAN_TranDataRec *GetTransactionData();
	BOOL GetRecieving();
	void SetCHost(CHost* host);

private:
	CFinan m_FINAN;
	CAdmin m_ADMIN;
	CONF_Data m_CONFIG;
	TRAN_TranDataRec m_TRREC;
	int m_status;
	
	void DoFinan(ECR_TranDataRec *pEcrRec);
	void DoAdmin(ECR_TranDataRec *pEcrRec);
	void DoLocal(ECR_TranDataRec *pEcrRec);


};
