#include "stdafx.h"
#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"
#include "..\\Host\\Host.h"
#include "Transactions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*************************************************************************/
CTransaction::CTransaction()
{

}

/*************************************************************************/
CTransaction::~CTransaction()
{

}

/*************************************************************************/
void CTransaction::DoTransaction(ECR_TranDataRec *pEcrRec)
{
	// first read config data into m_CONFIG
	
	// check transaction code for Financial or Admin
	if (pEcrRec->TranCode > FUNCTION_FINAN_BEGIN &&
			pEcrRec->TranCode < FUNCTION_FINAN_END)
	{
		DoFinan(pEcrRec);
	}
	else if (pEcrRec->TranCode > FUNCTION_ADMIN_BEGIN &&
					 pEcrRec->TranCode < FUNCTION_ADMIN_END)
	{
		DoAdmin(pEcrRec);
	}
	else	// local functions
	{
		DoLocal(pEcrRec);
	}
}
	
/*************************************************************************/
void CTransaction::DoFinan(ECR_TranDataRec *pEcrRec)
{
	BYTE retry = m_FINAN.DoTransaction(pEcrRec);
	m_status = m_FINAN.GetComStatus();
	if (m_status == ST_OK)
	{
		m_status = m_FINAN.GetTranStatus();
	}
	m_FINAN.GetTranRec(&m_TRREC);
}

/*************************************************************************/
void CTransaction::DoAdmin(ECR_TranDataRec *pEcrRec)
{
	BYTE status = m_ADMIN.DoTransaction(pEcrRec);
	m_status = m_ADMIN.GetComStatus();
	if (m_status == ST_OK)
	{
		m_status = m_ADMIN.GetTranStatus();
	}
	m_ADMIN.GetTranRec(&m_TRREC);
}

/*************************************************************************/
void CTransaction::DoLocal(ECR_TranDataRec *pEcrRec)
{
	//m_status = m_LOCAL.DoTransaction(pEcrRec);
	//GetTranRec(&m_TRREC);
}

/*************************************************************************/
int CTransaction::GetStatus()
{
	return m_status;
}

/*************************************************************************/
TRAN_TranDataRec *CTransaction::GetTransactionData()
{
	return &m_TRREC;
}

void CTransaction::SetCHost(CHost* host)
{
	m_FINAN.m_HOST = host;
	m_ADMIN.m_HOST = host;
}
	
