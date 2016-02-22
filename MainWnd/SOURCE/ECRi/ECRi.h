#pragma once

// CECRi 
class CECRi
{
public:
	CECRi();   // standard constructor
	~CECRi();

public:
	char* m_Request;
	char m_Response[SZ_HOST_BUFF_MAX];
	int m_ResponseLength;

	int ParseEcrRequest(char *reqBuf);
	void FormatEcrResponse(int status, TRAN_TranDataRec *pRec);
	ECR_TranDataRec ECR_TRANREC;

private:
	BOOL Write(char *txt);

	int ECR2TxnCode(int EcrCode);
	inline void Skip(int num);
	inline void Back(int num);
	bool FieldEnd(char *data);
	int FieldLen(int maxLen);
	void GetInt(int &x, int size);
	void GetLongInt(long int &x, int size);
	inline void GetStr(char *str, int size);
	int GetTransactionCode();
	int GetTerminalId();
	int GetAmount();
	int GetInvoice();
	int ParseHostInit();
	int ParsePurchase();
	
	void Put(char c);
	void Put(char c, int count);
	void Put(const char *str);
	void Put(const char *str, int maxLength);
	void PutRightFilled(const char *str, int totalLength, char fill);
	void PutBCDAscii(const BYTE *buffer, int size);
};
