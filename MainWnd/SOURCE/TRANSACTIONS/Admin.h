#pragma once

class CAdmin
{
	public:
		CAdmin(void);
		~CAdmin(void);
		int GetComStatus();
		int GetTranStatus();
		int DoTransaction(ECR_TranDataRec *pEcrTranRec);
		void GetTranRec(TRAN_TranDataRec *pRec);
		void SetInitStatus(int status);
		int GetInitStatus();
		CHost* m_HOST;
	
	private:
		TRAN_TranDataRec m_TRREC;
		CONF_Data m_CONFIG;
		int m_CustDialogTimer;
				
	private:
		void ClearScreen();
		void Display(int index1, int index2, int index3, int index4, 
																		int index5, int index6, int index7);
		void SetTermLanguage();
		void SetTranTitle();
		void FillTransRec(ECR_TranDataRec *pEcrTranRec);
		int DoHostInit();
		int DoKeyExchange();
		int DoCloseBatch();
		void DoHost();
};
