#include "StdAfx.h"
#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"
#include "..\\UI\\language.h"
#include "..\\UI\\display.h"
#include "..\\utils\\BinUtil.h"
#include "..\\utils\\StrUtil.h"
#include "..\\utils\\Util.h"
#include "..\\data\\DataFile.h"
//#include "..\\data\\DBFile.h"
#include "..\\data\\RecordFile.h"
#include "..\\utils\\FolderCtrl.h"
#include "..\\host\\host.h"
#include "..\\EMV\\EMV.h"
#include "Admin.h"

/******************************************************************************/
CAdmin::CAdmin(void)
{
}

/******************************************************************************/
CAdmin::~CAdmin(void)
{
}

/******************************************************************************/
void CAdmin::ClearScreen()
{
	CDisplay::ClearScreen();
}

/******************************************************************************/
void CAdmin::Display(int index1, int index2, int index3, int index4, 
				int index5, int index6, int index7)
{
	CDisplay::SetText(1, index1);
	CDisplay::SetText(2, index2);
	CDisplay::SetText(3, index3);
	CDisplay::SetText(4, index4);
	CDisplay::SetText(5, index5);
	CDisplay::SetText(6, index6);
	CDisplay::SetText(7, index7);
	CDisplay::Display();	
}

/******************************************************************************/
int CAdmin::DoTransaction(ECR_TranDataRec *pEcrTranRec)
{
	int status;
	DWORD size;
	
	assert(CDataFile::Read(FID_MERCH_CFG_TERM_ID, (BYTE*) m_CONFIG.TermID, &size));
	assert(CDataFile::Read(FID_CFG_DEVICE_MODEL, (BYTE*) m_CONFIG.DeviceModel, &size));
	m_CONFIG.EMVEnabled = TRUE;

	// Fill m_TRREC with data from ECR record
	FillTransRec(pEcrTranRec);
	m_TRREC.HostTranCode = m_TRREC.TranCode;
	
	switch(m_TRREC.TranCode)
	{
	case TRAN_HOST_INIT:
		status = DoHostInit();
		break;
	case TRAN_KEY_EXCHANGE:
		status = DoKeyExchange();
		break;
	case TRAN_CLOSE_BATCH:
		status = DoCloseBatch();
		break;
	default:
		assert(FALSE);
	}
	
	return status;
}

/******************************************************************************/
int CAdmin::DoHostInit()
{
	// clear host download data
	CRecordFile HostFile;
	HostFile.Delete(SERVICE_DATA_FILE);
	HostFile.Delete(EMV_DATA_FILE);
	HostFile.Delete(CTLS_DATA_FILE);
	CDataFile::Delete(FID_TERM_MAC_REQ_FIDS);
	CDataFile::Delete(FID_TERM_MAC_RSP_FIDS);
	CDataFile::Delete(FID_MERCH_CFG_ADDRESS);
	CDataFile::Delete(FID_MERCH_CFG_CITY_PROV);
	CDataFile::Delete(FID_MERCH_CFG_NAME);	
	CDataFile::Delete(FID_TERM_ALLOWED_TRANS);
	CFolderCtrl::DeleteDir(EMV_KEY_DATA_DIR);
	SetInitStatus(STS_DLL_DOWNLOAD_REQUIRED);
	
	// Host communication	
	DoHost();
	
	// set download status
	if (m_HOST->GetCommStatus() == ST_OK)
	{
		if (m_HOST->GetTranStatus() == ST_APPROVED)
		{
			AP_FILE_HANDLER fh;
			if (HostFile.Open(EMV_DATA_FILE, &fh))	// EMV download
			{
				HostFile.CloseFile(&fh);
				CEMV EMV;
				BOOL enable;
				if (EMV.Init())
				{
					enable = TRUE;
				}
				else
				{
					enable = FALSE;
				}
				CDataFile::Save(FID_TERM_EMV_ENABLE_FLG, (BYTE *) &enable, sizeof(BOOL));
			}	
			SetInitStatus(STS_DLL_OK);
			CDisplay::ShowStatus(CLanguage::IDX_HOST_INIT_DONE);
		}
	}
	return 0;
}

/******************************************************************************/
int CAdmin::DoKeyExchange()
{
	// clear keys
	CDataFile::Delete(FID_TERM_PIN_KEY);
	CDataFile::Delete(FID_TERM_MAC_KEY);
	SetInitStatus(STS_DLL_NO_KEYS);

	// Host communication	
	DoHost();
	
	// set download status
	if (m_HOST->GetCommStatus() == ST_OK)
	{
		if (m_HOST->GetTranStatus() == ST_APPROVED)
		{
			SetInitStatus(STS_DLL_OK);
			CDisplay::ShowStatus(CLanguage::IDX_HOST_INIT_DONE);
		}
		else
		{
			CDisplay::ShowStatus(CLanguage::IDX_ERROR);
		}
	}
	else
	{
		CDisplay::ShowStatus(CLanguage::IDX_ERROR);
	}
	return 0;
}

/******************************************************************************/
int CAdmin::DoCloseBatch()
{
	DoHost();
	return 0;
}

/******************************************************************************/
void CAdmin::DoHost()
{
	m_HOST->Start(&m_TRREC, &m_CONFIG);
}

/******************************************************************************/
int CAdmin::GetComStatus()
{
	return m_TRREC.ComStatus;
}

/******************************************************************************/
int CAdmin::GetTranStatus()
{
	return m_TRREC.TranStatus;
}

/******************************************************************************/
void CAdmin::GetTranRec(TRAN_TranDataRec *pRec)
{
	memcpy(pRec, &m_TRREC, sizeof(TRAN_TranDataRec));
}

/******************************************************************************/
void CAdmin::SetTermLanguage()
{
	int termLang;
	DWORD size;
	assert(CDataFile::Read(FID_CFG_LANGUAGE, (BYTE *)&termLang, &size));
	CLanguage::SetLanguage(termLang);
}

/******************************************************************************/
void CAdmin::SetTranTitle()
{
	switch (m_TRREC.TranCode)
	{
	case TRAN_HOST_INIT:
		m_TRREC.Title = CLanguage::IDX_HOST_INIT;
		break;
	case TRAN_KEY_EXCHANGE:
		m_TRREC.Title = CLanguage::IDX_KEY_EXCHANGE;
		break;
	case TRAN_CLOSE_BATCH:
		m_TRREC.Title = CLanguage::IDX_CLOSE_BATCH;
		break;
	default:
		assert(FALSE);
	}
}

/******************************************************************************/
void CAdmin::FillTransRec(ECR_TranDataRec *pEcrTranRec)
{
	m_TRREC.TranCode = pEcrTranRec->TranCode;
	SetTranTitle();

	// Set Terminal Language
	SetTermLanguage();
}

/******************************************************************************/
void CAdmin::SetInitStatus(int status)
{
	assert(CDataFile::Save(FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, sizeof(int)));
}

/******************************************************************************/
int CAdmin::GetInitStatus()
{
	DWORD size = sizeof(int);
	int status;

	assert(CDataFile::Read(FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, &size));

	return status;
}


