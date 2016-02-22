#ifndef COMMON_H
#define COMMON_H

//Trans Type
#define EMV_TRANSTYPE_CASH					0x0001
#define EMV_TRANSTYPE_GOODS					0x0002
#define EMV_TRANSTYPE_SERVICES				0x0004
#define EMV_TRANSTYPE_CASHBACK				0x0008
//RFU
#define EMV_TRANSTYPE_GOODS_CASHBACK		0x0010
#define EMV_TRANSTYPE_SERVICES_CASHBACK		0x0020
#define EMV_TRANSTYPE_INQUIRY				0x0040
#define EMV_TRANSTYPE_TRANSFER				0x0080
#define EMV_TRANSTYPE_PAYMENT				0x0100
#define EMV_TRANSTYPE_ADMINISTRATIVE		0x0200
#define EMV_TRANSTYPE_CASH_DEPOSIT			0x0400

// Account Type
#define AccountType_Default		0x00
#define AccountType_Saving		0x10
#define AccountType_Debit		0x20
#define AccountType_Credit		0x30

//////////////////////////////////////////////////////////////////////////
//Call Back function type.

#define  CB_SCRSEND			1		//Only Send		wParam = byBuf, lParam = &wLen in
#define  CB_SCRREV			2		//Only Receive  wParam = byBuf, lParam = &wLen in/out
#define  CB_DEBUGINFO		3		//Degug infor export.wParam = LPSTR, lParam = NULL
#define  CB_DISPLAYMSG		4		//Display msg information wParam = byMSG, lParam:RFU
#define  CB_SELECTAPP		5		//Selecgt App Aid, [in] wParam = PCANDIDATE_LIST, 
									//[in]lParam = PDWORD, MAKELPARAM(LOW, HIGH) LOW= The number of retries, HIGH=candidate applications number.
									//[out]lParam = PDWORD, Selected Aid index.
#define  CB_TRANAMOUNT		6		//Read transcation amount sum, wParam = PTRANSRECORD, lParam:RFU
#define  CB_EXCEPTION		7		//Read Exception black list, wParam = PEXCEPTIONFILE, lParam:RFU
#define	 CB_VOICEREFERRAL	8		//offer a chance to attendant to call bank, wParam = PAN Buffer,lParam = panlen.
#define	 CB_PINENTRY		9		//Pin Entry wParam = PPINENTRY, lParam: RFU

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//PinEntry 
typedef struct tag_EpbInfo
{
	tag_EpbInfo()
	{
		byIsLastTry = 0;
		memset(byAmount, 0, 6);
		memset(szPan, 0, 20);
		byPinType = 0;
		memset(byPubkey, 0, 256);
		byPubKeyLen = 0;
		dwExponents = 0;
		memset(byChallenge, 0, 8);
		memset(byPinBlock, 0, 8);
		
	}
	BYTE	byIsLastTry;
	BYTE	byAmount[6];
	char	szPan[20];
 	BYTE	byPinType;
	BYTE	byPubkey[256];
	BYTE	byPubKeyLen;
	DWORD	dwExponents;
 	BYTE	byChallenge[8];
	BYTE	byPinBlock[8];
}EPBINFO, *PEPBINFO;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
typedef struct tag_TxnCondition
{
	tag_TxnCondition()
	{
		memset(byCashback, 0, 6);
		memset(byTransAmount, 0, 6);

		byForceAccept = 0;
		byForceOnline = 0;
		wTxnType = EMV_TRANSTYPE_GOODS;
		byAccountType = AccountType_Saving;
		byTxnTypeCode = 0;
		wTxnSN = 1;
		byDebug = 0;
	}
	
	BYTE byCashback[6];         // 81
	BYTE byTransAmount[6];
	BYTE byForceOnline;
	BYTE byForceAccept;
	WORD wTxnType;				// 9C Transaction Type
	BYTE byTxnTypeCode;			// 9C Transaction Type Code
	BYTE byAccountType;			// 5F57
	WORD wTxnSN;				// Transaction Sequence Number; 9F41
	BYTE byDebug;				//zore not output debug info. nozore export debug info.
}TXNCONDITION, *PTXNCONDITION;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
typedef struct tag_AuthResponse
{
	tag_AuthResponse()
	{
		memset(byAcquirerIdentifier, 0, 6);			// 9F01
		memset(byAmountAuthorized, 0, 6);				// 81
		memset(byIssuerScript, 0, 261);				// 86 - optional data, var len, left justified
		memset(byTermIdentifier, 0, 8);				// 9F1C 
		memset(byTransDate, 0, 3);					// 9A
		memset(byTransTime, 0, 3);					// 9F21
		memset(byAuthorizationCode, 0, 6);			// 89
		memset(byAuthorizationResponseCode, 0, 2);	// 8A
		memset(byIssuerAuthenticationData, 0, 16);	// 91
		
		byARCLen = 0;
		byIssuerAuthenticationDataLength = 0;				
		wIssuerScriptLength = 0;				// 
	}
	
	BYTE byAcquirerIdentifier[6];			// 9F01 Acquirer Identifier 
	BYTE byAmountAuthorized[6];				// 81   Amount, Authorised 
	BYTE byAuthorizationCode[6];			// 89   Authorisation Code 	
	BYTE byAuthorizationResponseCode[2];	// 8A	Authorisation Response Code
	BYTE byARCLen;
	BYTE byIssuerAuthenticationData[16];	// 91	Issuer 	Authentication Data
	BYTE byIssuerAuthenticationDataLength;		
	BYTE byIssuerScript[261];				// 86
	WORD wIssuerScriptLength;
	BYTE byTermIdentifier[8];				// 9F1C Terminal Identification 
	BYTE byTransDate[3];					// 9A	Transaction Date
	BYTE byTransTime[3];					// 9F21 Transaction Time 
}AUTHRESPONSE, *PAUTHRESPONSE;		// ARPC
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//CADIDATELIST
typedef struct tag_Candidate_List
{
	tag_Candidate_List()
	{
		byIndex = 0;
		byAppPI = 0;
		memset(szAid, 0, 33);
		memset(szAppLabel, 0, 17);
		memset(szAppPreName, 0, 17);
		memset(byLanguagePre, 0, 8);
		byIssuerCTI = 0;
	}
	
	BYTE byIndex;
	BYTE byAppPI;						// 87 ApplPriorityIndicator
	char szAid[33];
	char szAppLabel[17];				//Tag 50 Application Label
	char szAppPreName[17];				//9F12 ans 1~16 Application Preferred Name
	BYTE byIssuerCTI;					//9F11 IssuerCodeTableIndex
	BYTE byLanguagePre[8];				//5F2D Language preference;
}CANDIDATE_LIST, * PCANDIDATE_LIST;

//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// DATA TYPE
#define TYPE_ICS			0
//ICS TYPE
#define EMV_ICSVALUE_CVM		0x00000001

#define TYPE_TERMINAL		1
#define TYPE_APPL			2
#define TYPE_KEY			3

//Define Tag
#define TAG_ICS				0xDF01
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//ICS	CVM Capability
#define SUPPORT_SIGNATURE					0x20	//ok

typedef struct tag_ICS
{
	BYTE	byTerminalType;		// bit8: offline only
								// bit7: offline with online
								// bit6: online only
								// bit5: attend
								// bit4: unattend
								// bit3: finical
								// bit2: cardholder
								// bit1: merchant

	BYTE	byCardDataInput;	// bit8: Manual Key Entry
								// bit7: Magnetic Stripe
								// bit6: IC with Contacts (M)

	BYTE	byCVMCapability;	// bit8: offline palin pin
								// bit7: online encrypher pin
								// bit6: signature
								// bit5: offline encipher pin
								// bit4: no cvm required

	BYTE   bySecurityCapability;// bit8: SDA
								// bit7: DDA
								// bit6: Card capture
								// bit5: RFU
								// bit4: CDA

	BYTE	byTxnType[2];		// byte1 bit8: cash
								// byte1 bit7: goods
								// byte1 bit6: service
								// byte1 bit5: cashback
								// byte1 bit4: Inquiry
								// byte1 bit3: Transfer
								// byte1 bit2: Payment
								// byte1 bit1: Administrative
								// byte2 bit8: Cash Deposit
								// byte2 bit7~bit1: RFU

	BYTE	byTmDataInput;		//	bit8: Keypad
								//	bit7: Numeric Keys
								//	bit6: Alphabetic and Special Character Keys
								//	bit5: Command Keys
								//	bit4: Function Keys

	BYTE	byTmDataOutput[2];	//	byte1 bit8: Print Attendant
								//	byte1 bit7: Print Cardholder
								//	byte1 bit6: Display Attendant
								//	byte1 bit5: Display Cardholder
								//	byte1 bit4: RFU
								//	byte1 bit3: RFU
								//	byte1 bit2: Code Table 10
								//	byte1 bit1: Code Table 9

								//	byte2 bit8: Code Table 8
								//	byte2 bit7: Code Table 7
								//	byte2 bit6: Code Table 6
								//	byte2 bit5: Code Table 5
								//	byte2 bit4: Code Table 4
								//	byte2 bit3: Code Table 3
								//	byte2 bit2: Code Table 2
								//	byte2 bit1: Code Table 1

	BYTE	byApplicationSelect;// bit8: support PSE
								// bit7: support cardholder confirm
								// bit6: prefer order
								// bit5: partial aid select(M)
								// bit4: support multi language
								// bit3: common character set(M)
								// bit2: EMV Language Selection Method

	BYTE	byMaxCAPK;
	BYTE	byExponent;			//	bit8: 3
								//	bit7: 2^16+1
								//	bit6: 3 and 2^16+1	
	BYTE	byDataAuthen;		// bit8: support check revocation ca public key
								// bit7: contain default DDOL
								// bit6: operator ation required during loading ca public key fail
								// bit5: verify ca public key checksum 

	BYTE	byCVM;				// bit8: support bypass pin entry
								// bit7: support Sub bypass pin entry 
								// bit6: support get data for pin try counter
								// bit5: support fail cvm
								// bit4: amount knows before cvm

	BYTE	byTRM;				// bit8: floor limit checking
								// bit7: random selection
								// bit6: velocity checking
								// bit5: support transaction log
								// bit4: support exception file
								// bit3: performance of terminal risk management based on AIP setting

	BYTE	byActionAnalysis;	// bit8: support terminal action codes
								// bit7: default action codes prior to first generate ac
								// bit6: default action codes after first generate ac
								// bit5: TAC/IAC default skipped
								// bit4: TAC/IAC default normal
								// bit3: CDA failure before TAA
								// bit2: Can the values of TACs be changed
								// bit1: Can the TACs be deleted or disabled


	BYTE	byCDAMode;			// bit8: CDA always 1 GenAC
								// bit7: CDA never 1 GenAC
								// bit6: CDA always 2 GenAC
								// bit5: CDA never 2 GenAC

	BYTE	byCompletion;		// bit8: support force online
								// bit7: support force accept
								// bit6: support advice
								// bit5: support issuer initiated voice referral
								// bit4: support batch data capture
								// bit3: support online data capture
								// bit2: support default TDOL

	BYTE	byException;		// POS entry mode

	BYTE	byMISC;				// bit8: PINPAD
								// bit7: PIN and amount in same keypad
								// bit6: ICC/Magstripe Reader commbined
								// bit5: Magstripe Reader first
								// bit4: Account Type select
								// bit3: on fly script processing
								// bit2: Issuer Script device limt >128?
								// bit1: Internel date management

	BYTE	bySelKernelConfig;	// bit8: Support Selectable Kernel Config
								// bit7: Amount Select Range Criteria X
								// bit6: Amount Select Range Criteria X and Y
								// bit5: Amount Select Range Criteria X per AID
								// bit4: Amount Select Range Criteria X and Y per AID


}ICS_DATA;

//////////////////////////////////////////////////////////////////////////
//EMVGetTransactionResult fun result
//if result >=200 then need print sign

#define TRANSACTION_APPROVE     0
#define TRANSACTION_REJECT      1
#define TRANSACTION_NEEDPRINT   200

//////////////////////////////////////////////////////////////////////////
//For Pboc
typedef struct Tag_CardHolderCheck
{
	Tag_CardHolderCheck()
	{
		memset(byCardNumber, 0, 40);
		byNumLen = 0;
		byCardType = 0;
	}	

	BYTE byCardNumber[40];
	BYTE byNumLen;
	BYTE byCardType;

}CARDHOLDERCHECK, *PCARDHOLDERCHECK;
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//For Transaction log
typedef struct tag_TransRecord
{
	tag_TransRecord()
	{
		memset(by5A, 0, 20);
		byPanLen = 0;
		by5F34 = 0;
		memset(by9F02, 0, 6);
		memset(by9F03, 0, 6);
		memset(by9A, 0, 3);
		memset(by9F21, 0, 3);

		memset(byAmountCount, 0, 6);
	}
	BYTE by5A[20];		//Tag 5A PAN
	BYTE byPanLen;		
	BYTE by5F34;		//Sequence Number -1
	BYTE by9F02[6];		//Tag 9F02 Trans Amount - 6
	BYTE by9F03[6];		//Tag 9F03 Trans Cash Amount -6
	BYTE by9A[3];		//Tag 9A Trans Date YYMMDD --3
	BYTE by9F21[3];		//Tag 9F21 Trans Time HHMMSS -3

	BYTE byAmountCount[6];	//for amount sum
}TRANSRECORD, *PTRANSRECORD;


//////////////////////////////////////////////////////////////////////////
//
typedef struct tag_ExceptionFile
{
	tag_ExceptionFile()
	{
		memset(by5A, 0, 20);

		by5ALen = 0;
		by5F34 = 0;
	}
	BYTE by5A[20];			//Tag 5A Application Primary Account Number (PAN) 
	BYTE by5ALen;			//Pan Len
	BYTE by5F34;			//Application Primary Account Number (PAN) Sequence Number 

}EXCEPTIONFILE, *PEXCEPTIONFILE;
//////////////////////////////////////////////////////////////////////////


#define TRANSLOG_FILENAME	"SAIO\\System\\TransLog"
#define EXCEPTION_FILENAME	"SAIO\\System\\Exception"

#define WM_STRMSG				(WM_USER + 3)

#endif
