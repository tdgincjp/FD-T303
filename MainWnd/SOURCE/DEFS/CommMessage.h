
/* 
 * File:   CommMessage.h
 * Author: Jun
 *
 * Created on July 18, 2013, 12:00 PM
 */

#ifndef COMMMESSAGE_H
#define	COMMMESSAGE_H

#pragma warning(disable: 4996)

#ifdef	__cplusplus
extern "C" {
#endif
#define SOH       0x01        // Start of hearder
#define STX       0x02        // Start of text
#define ETX       0x03        // End of text
#define EOT       0x04        // End of transmission
#define ENQ       0x05        // Inquiry
#define ACK       0x06        // Acknowledge
#define NAK       0x15        // Negative acknowledge
#define ETB       0x17        // End of block
#define CAN       0X18        // Cancel
#define FS        0X1C        // File seperator
#define GS        0X1D        // Group seperator
#define RS        0X1E        // Record seperator
	
#define BUFFERSIZE  4096

//#define ECR_CONFIGURATION				"079"			Removed for PCI JC April 30/15	
#define ECR_GET_RECEIPT					"080"		
//#define ECR_GET_TRACK_DATA			"081"		 Removed for PCI JC April 8/15
//#define ECR_DISPLAY_WALLPAPER			"082"			
#define ECR_HOST_DOWNLOAD				"090"		
#define ECR_HANDSHAKE					"095"		
//#define ECR_MERCHANT_REVERSAL			"058"		
#define	ECR_UNKNOWN_TXN					"ZZZ"		// JC Dec 5/14 to process an unknown Txn Code
#define ECR_STATUS						"999"		// JC Dec 10/14 App Status command to retrieve status
#define ECR_PURCHASE					"000"	
#define ECR_TXN_CANCEL					"900"		//JC Oct 28/15


#define TAG_ECR_MESSAGE_TYPE			"MT"
#define TAG_ECR_TRAN_CODE				"TC"
#define TAG_ECR_ECHO_DATA				"EC"
#define TAG_ECR_CONDITION_CODE			"CC"
#define TAG_ECR_AMOUNT					"AM"
#define TAG_ECR_DL_REQUIRED				"DR"
#define TAG_ECR_IP_SETTINGS				"IP"
#define TAG_ECR_HOST_SELECTION			"HP"		//JC Dec 30/14 Primary Host tag
#define TAG_ECR_HOST_SELECTION2			"HS"		//JC Dec 30/14 Secondary Host tag
#define TAG_ECR_PARAM_HOST_SELECTION	"GA"		//JC Dec 30/14 Gateway
#define TAG_ECR_TERMINAL_ID				"TI"		//JC Dec 30/14 Terminal ID
#define TAG_ECR_FORMATTED_RCPT			"FR"
#define TAG_ECR_TRACK1					"T1"
#define TAG_ECR_TRACK2					"T2"
#define TAG_ECR_IMAGE_INDEX				"II"
#define TAG_ECR_EDIT_ERROR				"ET"		//JC Dec 8/14 Edit Error TAG (this has the TAG ID of where the edit error is)
#define TAG_ECR_STATUS					"ST"		// JC Dec 10/14 Application Status
#define TAG_ECR_CUSTOMER_TEXT			"CT"		//JC Mar 13/15 To add new TAG in response with Customer Text
#define TAG_ECR_HOST_RESPONSE_CODE		"HC"		//Jun May 4/15 Host Response Code TAG
#define	TAG_ECR_RECEIPT_TYPE			"RT"		//JC May 13/15 to allow selection of the type of receipt to print Cardholder or Merchant
#define TAG_ECR_APPLICATION_INFORMATION "AI"		//AM June 18/15 
#define TAG_ECR_PRINT_ON_PED			"PR"
#define TAG_ECR_INVOICE_NUMBER_CODE				"IN"		//Jun Oct 22/15 Txn result TAG
#define TAG_ECR_AUTHORIZATION_NUMBER_CODE		"AN"		//Jun Oct 22/15 Txn result TAG
#define TAG_ECR_CARD_TYPE_CODE					"CY"		//Jun Oct 22/15 Txn result TAG
#define TAG_ECR_CARD_PLAN_CODE					"CP"		//Jun Oct 22/15 Txn result TAG
#define TAG_ECR_CARDHOLDER_LANGUAGE_CODE		"CL"		//Jun Oct 22/15 Txn result TAG
#define TAG_ECR_CARD_NUMBER_CODE				"CN"		//Jun Oct 22/15 Txn result TAG

#define SZ_ECR_HEADER						5
#define SZ_ECR_MESSAGE_TYPE					3
#define SZ_ECR_TRAN_CODE					3
#define SZ_ECR_ECHO_DATA_MAX				50
#define SZ_ECR_CONDITION_CODE				4				//JC Dec 8/14 should be 4, not 3.....
#define SZ_ECR_AMOUNT_MAX					9
#define SZ_ECR_IP_SETTINGS					49
#define SZ_ECR_HOST_SELECTION				108				//JC Dec 30/14 increased size
#define SZ_ECR_TERMINAL_ID					8				//JC Dec 30/14 Add terminal ID
#define SZ_ECR_FORMATTED_RCPT				2500
#define SZ_ECR_RECEIPT_TYPE					1				//JC May 13/15
#define SZ_ECR_TRACK1						80
#define SZ_ECR_TRACK2						80
#define SZ_ECR_IMAGE_INDEX					2
#define SZ_ECR_PAYLOAD_MAX_LENGTH			2579
#define SZ_ECR_TAG							2				//JC Dec 8/14 Tag Size
#define SZ_ECR_STATUS						6				//JC Dec 10/14 Status Tag Size
#define SZ_CUSTOMER_TEXT					100				//JC April 4/15
#define SZ_RESP_CODE						10				//Jun May 4/15
#define SZ_ECR_INVOICE_NUMBER				8
#define SZ_ECR_PRINT_FLAG					1
#define SZ_ECR_PRINT_TYPE					1

#define ERROR_CONDITION_CODE_OK				"0000"	//JC Dec 8/14 Any Approval 
#define ERROR_CONDITION_CODE_DECLINE		"1000"	//JC Dec 8/14 Any Decline from Host
#define ERROR_PED_BUSY						"1001"	//JC Apr 10/15 A transaction is already in process

#define ERROR_CONDITION_CODE_NO_RECEIPT		"1050"	//JC April 23/15 No Receipt to Print
#define ERROR_CONDITION_CODE_TIMEOUT		"1051"	//JC Dec 8/14 Timeout waiting for Cardholder
#define ERROR_CONFIGURATION_NEEDED			"1052"	//JC April 23 COnfiguration Needed *******
#define ERROR_HOST_DOWNLOAD_REQUIRED		"1053"  //JC April 23/15 Host download required *******
#define ERROR_CONDITION_CODE_NETWORK		"1054"	//JC Dec 8/14 General Network Error
#define ERROR_CONDITION_CODE_CANCEL			"1059"	//JC Dec 8/14 Client Pressed Cancel
#define ERROR_CONDITION_CODE_INVALID_CARD_SWIPE           "1060"    //Jun May 5/15 Invalid Card Swipe
//#define ERROR_CONDITION_CODE_TXN_SUPPORT_CHECK            "1061"    //Jun May 5/15 TXN support check
#define ERROR_CONDITION_CODE_NOTHING_TO_CANCEL	"1065"	//JC Oct 28/15

#define ERROR_OPEN_DEVICE					"1070"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_ENTERPIN_FAIL				"1071"	//Jun Feb 5/15 EMV Error
#define ERROR_ENTER_ONLINE_PIN				"1072"	//Jun Feb 5/15 EMV Error
#define ERROR_DECLINED_BY_CARD				"1073"	//Jun Feb 5/15 EMV Error
#define ERROR_PLEASE_SEE_PHONE				"1074"	//Jun Feb 5/15 EMV Error
#define ERROR_MSR_OPEN_FAIL					"1075"	//Jun Feb 5/15 EMV Error
#define ERROR_AMOUNT_ABOVE_MAXIMUM			"1076"	//Jun Feb 5/15 EMV Error
#define ERROR_APPLICATION_NO_SUPPORTED		"1077"	//Jun Feb 5/15 EMV Error
#define ERROR_CARD_REMOVED					"1078"	//Jun Feb 5/15 EMV Error
#define ERROR_DEBIT_CARD_NO_SUPPORTED		"1079"	//Jun Feb 5/15 EMV Error
#define ERROR_CREDIT_CARD_NO_SUPPORTED		"1080"	//Jun Feb 5/15 EMV Error
#define ERROR_EMVERR_ICC_COMM				"1081"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_CARD_BLOCKED				"1082"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_FRAUD_CARD				"1083"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_APPLICATION_NOTSUPPORT	"1084"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_STOP_TRANSACTION			"1085"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_CHIP_ERROR				"1086"	//Jun Feb 5/15 EMV Error
#define ERROR_EMV_CARD_PROCESS				"1087"	//Jun Feb 5/15 EMV Error


#define ERROR_FORMAT_CC_GENERAL				"2000"		//**JC Dec 8/14 General format error
#define ERROR_FORMAT_CC_HEADER				"2001"		//*JC Dec 8/14 Header is bad or missing
#define ERROR_FORMAT_CC_LENGTH				"2002"		//**JC Dec 8/14 Message length is bad (way too long)
#define ERROR_FORMAT_CC_TRANCODE			"2003"		//*JC Dec 8/14 Trancode is not known
#define ERROR_FORMAT_CC_MTYPE				"2004"		//*JC Dec 8/14 Message type is bad not "REQ"
#define ERROR_FORMAT_CC_MISSING_TAG			"2005"		//*JC Dec 9/14 Manditory TAG is missing
#define ERROR_FORMAT_CC_TAG_LENGTH			"2006"		//*JC Dec 9/14 TAG LENGTH IS TOO LONG
#define ERROR_FORMAT_CC_TAG_DATA			"2007"		//JC Dec 9/14 TAG Data is malformed
#define ERROR_FORMAT_CC_MISSING_SYNC		"2008"		//*JC Dec 9/14 Missing Sync bytes in header
#define ERROR_FORMAT_CC_INVALID_AUDIT		"2009"		//*JC Dec 9/14 Invalid Audit Byte value

#define ERROR_FORMAT_CC_DATA_WIRE			"3000"		//*Jun Nov 9/15 Invalid Audit Byte value
	


typedef enum
	{
		MTAG_START = 0,								//JC Dec 5/14 MTAGs = Manditory, CTAGS = Conditional, OTags=Optional
		MTAG_ECR_MESSAGE_TYPE,
		MTAG_ECR_TRAN_CODE,
		MTAG_ECR_ECHO_DATA,
		MTAG_ECR_CONDITION_CODE,
		MTAG_ECR_AMOUNT,
		MTAG_ECR_INVOICE_NUMBER,
		MTAG_ECR_DL_REQUIRED,
		MTAG_ECR_IP_SETTINGS,
		MTAG_ECR_HOST_SELECTION,
		MTAG_ECR_FORMATTED_RCPT,
		MTAG_ECR_TRACK1,
		MTAG_ECR_TRACK2,
		MTAG_ECR_IMAGE_INDEX,
		MTAG_ECR_STATUS,								//JC Dec 10/14 Status Message
		MTAG_END = 99,

		OTAG_START = 100,
		OTAG_ECR_MESSAGE_TYPE,
		OTAG_ECR_TRAN_CODE,
		OTAG_ECR_ECHO_DATA,
		OTAG_ECR_CONDITION_CODE,
		OTAG_ECR_AMOUNT,
		OTAG_ECR_DL_REQUIRED,
		OTAG_ECR_IP_SETTINGS,
		OTAG_ECR_HOST_SELECTION,
		OTAG_ECR_HOST_SELECTION2,			//JC Dec 30/14 Secondary host address or name
		OTAG_ECR_PARAM_HOST_SELECTION,		//JC Dec 30/14
		OTAG_ECR_TERMINAL_ID,				//JC Dec 30/14
		OTAG_ECR_FORMATTED_RCPT,
		OTAG_ECR_TRACK1,
		OTAG_ECR_TRACK2,
		OTAG_ECR_IMAGE_INDEX,
		OTAG_ECR_RECEIPT_TYPE,				//JC May 13/15
		OTAG_ECR_PRINT_ON_PED,
		OTAG_ECR_PRINT_TYPE,
		OTAG_END = 199,

		CTAG_START = 200,
		CTAG_ECR_MESSAGE_TYPE,
		CTAG_ECR_TRAN_CODE,
		CTAG_ECR_ECHO_DATA,
		CTAG_ECR_CONDITION_CODE,
		CTAG_ECR_AMOUNT,
		CTAG_ECR_DL_REQUIRED,
		CTAG_ECR_IP_SETTINGS,
		CTAG_ECR_HOST_SELECTION,
		CTAG_ECR_FORMATTED_RCPT,
		CTAG_ECR_TRACK1,
		CTAG_ECR_TRACK2,
		CTAG_ECR_IMAGE_INDEX,
		CTAG_ECR_EDIT_ERROR,					//JC Dec 5/12 Conditional TAG if Edit Error
		CTAG_ECR_HOST_RESPONSE_CODE,			//Jun May 4/15 Host Response Code TAG
		CTAG_ECR_CUSTOMER_TEXT,					//JC Mar 13/15 Conditional TAG with Text 
		CTAG_ECR_APPLICATION_INFORMATION,
		CTAG_ECR_INVOICE_NUMBER_CODE,			//Jun Oct 22/15 Txn result TAG
		CTAG_ECR_AUTHORIZATION_NUMBER_CODE,		//Jun Oct 22/15 Txn result TAG
		CTAG_ECR_CARD_TYPE_CODE,				//Jun Oct 22/15 Txn result TAG
		CTAG_ECR_CARD_PLAN_CODE,				//Jun Oct 22/15 Txn result TAG
		CTAG_ECR_CARDHOLDER_LANGUAGE_CODE,		//Jun Oct 22/15 Txn result TAG
		CTAG_ECR_CARD_NUMBER_CODE,				//Jun Oct 22/15 Txn result TAG
		CTAG_END = 299
	} sEcrTags;

	typedef struct
	{
		char TranCode[SZ_ECR_TRAN_CODE+1];
		BYTE ReqTblID;
		BYTE RespTblID;
	} sEcrMsgProcess;
	
	enum
	{
		END_OF_TAGS,
		CONFIGURATION_TAGS,
		GET_RECEIPT_TAGS,
		GET_TRACK_DATA_TAGS,
		DISPLAY_WALLPAPER_TAGS,
		HOST_DOWNLOAD_TAGS,
		HANDSHAKE_TAGS,
		PURCHASE_TAGS,
		MERCHANT_REVERSAL_TAGS,
		STATUS_TAGS,					//JC Dec 10/14
		TXN_CANCEL_TAGS,				//JC Oct 28/15
		UNKNOWN_TXN_TAGS			//JC Dec 5/14
	};

	const unsigned short EcrReqTagTable[][18] =  //JC Jan 1/15 changed to 12 from 10 for config
	{
//		{ CONFIGURATION_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_IP_SETTINGS,
//													OTAG_ECR_HOST_SELECTION, OTAG_ECR_HOST_SELECTION2, OTAG_ECR_PARAM_HOST_SELECTION, OTAG_ECR_TERMINAL_ID, OTAG_ECR_ECHO_DATA, END_OF_TAGS }, //JC Dec 30/14 Secondary Host, gateway, terminal id 

		{ GET_RECEIPT_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_RECEIPT_TYPE, OTAG_ECR_ECHO_DATA, END_OF_TAGS },		//JC May 13/15
		
//		{ GET_TRACK_DATA_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS },
		
//		{ DISPLAY_WALLPAPER_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, 
//															MTAG_ECR_IMAGE_INDEX, END_OF_TAGS },
		
		{ HOST_DOWNLOAD_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS },
		
//		{ HANDSHAKE_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS },
		
		{ PURCHASE_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_AMOUNT, OTAG_ECR_ECHO_DATA, END_OF_TAGS },
		
//		{ MERCHANT_REVERSAL_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS },

		{ STATUS_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS }, //JC Dec 10/14 Application Status Request Tag List

		{ TXN_CANCEL_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS }, //JC Oct 28/15

		{ UNKNOWN_TXN_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE,  END_OF_TAGS }			//JC Dec 5/14

	};

	const unsigned short EcrRespTagTable[][18] =	//JC Jan 1/15 changed to 13 from 10 for config
	{
//		{ CONFIGURATION_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE, OTAG_ECR_IP_SETTINGS, OTAG_ECR_HOST_SELECTION, 
//				OTAG_ECR_HOST_SELECTION2, OTAG_ECR_PARAM_HOST_SELECTION, OTAG_ECR_TERMINAL_ID, CTAG_ECR_ECHO_DATA, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, END_OF_TAGS },		//JC Dec 10/14 Add Edit Error Tag DEC 30/14 add all config options on response 

		{ GET_RECEIPT_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
													CTAG_ECR_ECHO_DATA, CTAG_ECR_FORMATTED_RCPT, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, END_OF_TAGS },	//JC Dec 10/14 Add Edit Error Tag
		
//		{ GET_TRACK_DATA_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
//													CTAG_ECR_ECHO_DATA, CTAG_ECR_TRACK1, CTAG_ECR_TRACK2, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, END_OF_TAGS }, //JC Dec 10/14 Add Edit Error Tag
		
//		{ DISPLAY_WALLPAPER_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
//													CTAG_ECR_ECHO_DATA, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, END_OF_TAGS }, //JC Dec 10/14 Add Edit Error Tag
		
		{ HOST_DOWNLOAD_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
													CTAG_ECR_ECHO_DATA, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, CTAG_ECR_APPLICATION_INFORMATION, END_OF_TAGS }, //JC Dec 10/14 Add Edit Error Tag
		
//		{ HANDSHAKE_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
//													CTAG_ECR_ECHO_DATA, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, CTAG_ECR_APPLICATION_INFORMATION, END_OF_TAGS },		//JC Dec 10/14 Add Edit Error Tag
		
		{ PURCHASE_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
													CTAG_ECR_ECHO_DATA, CTAG_ECR_DL_REQUIRED, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, CTAG_ECR_CUSTOMER_TEXT,
													CTAG_ECR_INVOICE_NUMBER_CODE,			//Jun Oct 22/15 Txn result TAG
													CTAG_ECR_AUTHORIZATION_NUMBER_CODE,		//Jun Oct 22/15 Txn result TAG
													CTAG_ECR_CARD_TYPE_CODE,				//Jun Oct 22/15 Txn result TAG
													CTAG_ECR_CARD_PLAN_CODE,				//Jun Oct 22/15 Txn result TAG
													CTAG_ECR_CARDHOLDER_LANGUAGE_CODE,		//Jun Oct 22/15 Txn result TAG
													CTAG_ECR_CARD_NUMBER_CODE,				//Jun Oct 22/15 Txn result TAG
													END_OF_TAGS }, //JC Dec 10/14 Add Edit Error Tag, Mar 13/15 Added Customer Text
		
//		{ MERCHANT_REVERSAL_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE,
//													CTAG_ECR_ECHO_DATA, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, END_OF_TAGS }, //JC Dec 10/14 Add Edit Error Tag

		{ STATUS_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE, MTAG_ECR_STATUS, OTAG_ECR_ECHO_DATA, CTAG_ECR_APPLICATION_INFORMATION, END_OF_TAGS }, //JC Dec 10/14 Application Status Response Tag List

		{ TXN_CANCEL_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE, OTAG_ECR_ECHO_DATA, END_OF_TAGS }, //JC Oct 28/15

		{ UNKNOWN_TXN_TAGS, MTAG_ECR_MESSAGE_TYPE, MTAG_ECR_TRAN_CODE, MTAG_ECR_CONDITION_CODE, CTAG_ECR_EDIT_ERROR,CTAG_ECR_HOST_RESPONSE_CODE, END_OF_TAGS }			//JC Dec 5/14

	};

	const sEcrMsgProcess EcrMsgTagTable[] =
	{
//		{ ECR_CONFIGURATION,		CONFIGURATION_TAGS,			CONFIGURATION_TAGS},  Removed JC April 30/15
		{ ECR_GET_RECEIPT,			GET_RECEIPT_TAGS,			GET_RECEIPT_TAGS},
	//	{ ECR_GET_TRACK_DATA,		GET_TRACK_DATA_TAGS,		GET_TRACK_DATA_TAGS},  JC April 8/15
//		{ ECR_DISPLAY_WALLPAPER,	DISPLAY_WALLPAPER_TAGS,		DISPLAY_WALLPAPER_TAGS	},
		{ ECR_HOST_DOWNLOAD,		HOST_DOWNLOAD_TAGS,			HOST_DOWNLOAD_TAGS},
//		{ ECR_HANDSHAKE,			HANDSHAKE_TAGS,				HANDSHAKE_TAGS},
		{ ECR_PURCHASE,				PURCHASE_TAGS,				PURCHASE_TAGS},
//		{ ECR_MERCHANT_REVERSAL,	MERCHANT_REVERSAL_TAGS,		MERCHANT_REVERSAL_TAGS},
		{ ECR_STATUS,				STATUS_TAGS,				STATUS_TAGS},			//JC Dec 10/14 to return local status
		{ ECR_TXN_CANCEL,			TXN_CANCEL_TAGS,			TXN_CANCEL_TAGS},			//JC Oct 28/15
		{ ECR_UNKNOWN_TXN,			UNKNOWN_TXN_TAGS,			UNKNOWN_TXN_TAGS}		//JC DEC 5/14 to return TXN type Unknown
	};

struct TXN_Result  //Jun Oct 22/15 Txn result TAG
{
	char InvoiceNo[SZ_INVOICE_NO+1];
	char AuthCode[SZ_APPROVAL_CODE+1];
	char CardType[2];
	char ServType[SZ_SERVICE_TYPE+1];
	char CustLang[2];
	char Account[SZ_ACCOUNT+1];
	TXN_Result()
	{
		Init();
	}

	void Init()
	{
		memset((void*)&InvoiceNo, 0, sizeof(TXN_Result));
	}
};

struct ECR_TranDataRec
{
	char AuditByte;
	char MessageType[SZ_ECR_MESSAGE_TYPE+1];
	char TranCode[SZ_ECR_TRAN_CODE+1];
	BYTE EchoData[SZ_ECR_ECHO_DATA_MAX];
	int EchoDataLength;
	char ConditionCode[SZ_ECR_CONDITION_CODE+1];
	char Amount[SZ_ECR_AMOUNT_MAX+1];
	BOOL DL_Required;
	char IP_Settings[SZ_ECR_IP_SETTINGS+1];
	BOOL HostSelectionIsURL;							// JC Dec 31/14 - Host 1 to URL or IP address
	char HostSelection[SZ_ECR_HOST_SELECTION+1];		// JC - Primary Host name or address
	int  HostSelectionPort;								// JC Dec 31/14 - Primary Host port
	BOOL HostSelection2IsURL;							// JC Dec 31/14 - Host 2 to URL or IP address
	char HostSelection2[SZ_ECR_HOST_SELECTION+1];		// JC Dec 30/14 - add secondary host name or address
	int  HostSelection2Port;							// JC Dec 31/14 - Secondary Host port
	BOOL ParamHostSelectionIsURL;								// JC Dec 31/14 - Parameter Host set to URL or IP address
	char ParamHostSelection[SZ_ECR_HOST_SELECTION+1];			// JC Dec 30/14 - add Host address to POSX gateway
	int  ParamHostSelectionPort;								// JC Dec 31/14 - Parameter Host port
	char Terminal_ID[SZ_ECR_TERMINAL_ID+1];				// JC Dec 30/14 - add terminal ID
	BYTE FormattedReceipt[SZ_ECR_FORMATTED_RCPT];
	int ReceiptLength;
	char Track1[SZ_ECR_TRACK1+1];
	char Track2[SZ_ECR_TRACK2+1];
	char ImageIndex[SZ_ECR_IMAGE_INDEX+1];
	BOOL EditError;
	char EditErrorTag[SZ_ECR_TAG+1];		//JC Tag With Error
	char Customer_Text[SZ_CUSTOMER_TEXT+1];		//JC April 4/15 CUSTOMER Text for TAG CT
	char RespCode[SZ_RESP_CODE+1];		//Jun May 4/15 Host Response Code TAG
	BYTE ReceiptType[SZ_ECR_RECEIPT_TYPE+1];			//JC May 13/15
	char InvoiceNumber[SZ_ECR_INVOICE_NUMBER+1];
	char PrintFlag[SZ_ECR_PRINT_FLAG+1];
	char PrintType[SZ_ECR_PRINT_FLAG+1];

	TXN_Result strTxnResult; //Jun Oct 22/15 Txn result TAG

	ECR_TranDataRec()
	{
	}
	void Init()
	{
		memset((void*)&AuditByte, 0, sizeof(ECR_TranDataRec));
		AuditByte = '0';
		EchoDataLength = 0;
		ReceiptLength = 0;
		DL_Required = FALSE;
		strcpy(MessageType,"REQ");
		strcpy(TranCode,"000");
		strcpy(RespCode,"000");
		EditError = FALSE;						//JC Dec 8/14
		strcpy(EditErrorTag,"  ");				//JC Dec 8/14

		strTxnResult.Init();  //Jun Oct 22/15 Txn result TAG
	}
};

struct CommMessageHead
{
	BYTE SyncByte[2];
	BYTE AuditByte;
	DWORD PayloadLength;
	CommMessageHead()
	{
		SyncByte[0] = '!';
		SyncByte[1] = '!';
		AuditByte = '0';
		PayloadLength = 0;
	}
	
	char* Format(char* buf)
	{
		sprintf(buf,"%c%c%c%04d",SyncByte[0],SyncByte[1],AuditByte,PayloadLength);
		return buf;
	}
	void GetValue(BYTE* buf)
	{
		SyncByte[0] = buf[0];
		SyncByte[1] = buf[1];
		AuditByte = buf[2];
		PayloadLength = (buf[3]-'0')*1000+(buf[4]-'0')*100+(buf[5]-'0')*10+buf[6]-'0';
	}
	int GetLength()
	{
		return 7;
	}
};

struct EchoMsg
{
	char Type[3];
	WORD Len;
	char Value[51];
	EchoMsg()
	{
		strcpy(Type,"EC");
		Len = 0;
		memset(Value,0,sizeof(Value));
	}

	char* Format(char* buf)
	{
		if(Len != 0)
			sprintf(buf,"%s%02d%s",Type,Len,Value);
		return buf;
	}

	int GetLength()
	{
		if(Len != 0)
			return 4+Len;
		else
			return 0;
	}

};

struct AmountMsg
{
	char Type[3];
	WORD Len;
	char Value[10];
	AmountMsg()
	{
		strcpy(Type,"AM");
		Len = 0;
		memset(Value,0,sizeof(Value));
	}

	char* Format(char* buf)
	{
		if(Len != 0)
			sprintf(buf,"%s%02d%s",Type,Len,Value);
		return buf;
	}

	int GetLength()
	{
		if(Len != 0)
			return 4+Len;
		else
			return 0;
	}
	void GetValue(BYTE* buf)
	{
		memcpy((void*)Type,(void*)buf,2);
		Len = (buf[2]-'0')*10+buf[3]-'0';
		memcpy((void*)Value,(void*)(buf+4),Len);
	}

};


struct ConditionCode
{
	char Type[3];
	WORD Len;
	char Value[4];
	ConditionCode()
	{
		strcpy(Type,"CC");
		Len = 3;
		strcpy(Value,"000");
	}

	char* Format(char* buf)
	{
		sprintf(buf,"%s%02d%s",Type,Len,Value);
		return buf;
	}
	int GetLength()
	{
		return 7;
	}
};
struct MessageType
{
	char Type[3];
	WORD TypeLen;
	char TypeValue[4];

	char TransactionCode[3];
	WORD TransactionCodeLen;
	char TransactionCodeValue[4];

	MessageType()
	{
		strcpy(Type,"MT");
		strcpy(TypeValue,"REQ");
		TypeLen = 3;

		strcpy(TransactionCode,"TC");
		strcpy(TransactionCodeValue,"000");
		TransactionCodeLen = 3;
	}

	char* Format(char* buf)
	{
		sprintf(buf,"%s%02d%s%s%02d%s",Type,TypeLen,TypeValue,TransactionCode,TransactionCodeLen,TransactionCodeValue);
		return buf;
	}

	void GetValue(BYTE* buf)
	{
		memcpy((void*)Type,(void*)buf,2);
		TypeLen = (buf[2]-'0')*10+buf[3]-'0';
		memcpy((void*)TypeValue,(void*)(buf+4),3);

		memcpy((void*)TransactionCode,(void*)(buf+7),2);
		TransactionCodeLen = (buf[9]-'0')*10+buf[10]-'0';
		memcpy((void*)TransactionCodeValue,(void*)(buf+11),3);
	}

	int GetLength()
	{
		return 14;
	}

};

struct HandShake_ResponseMessage
{
	MessageType MsgType;
	ConditionCode Code;
	EchoMsg Echo;

	HandShake_ResponseMessage()
	{
		strcpy(MsgType.TypeValue,"RSP");
		strcpy(MsgType.TransactionCodeValue,"095");
	}
	void isEcho(BYTE* buf)
	{
		int len = MsgType.GetLength();
		if ( buf[len]=='E' && buf[len+1] == 'C')
		{
			Echo.Len = (buf[len+2]-'0')*10+buf[len+3]-'0';
			strncpy(Echo.Value,(char*)&buf[len+4],Echo.Len);
		}
	}
	char* Format(char* buf)
	{
		MsgType.Format(buf);
		Code.Format(buf + MsgType.GetLength());
		Echo.Format(buf + MsgType.GetLength() + Code.GetLength());
		return buf;
	}

	WORD GetLength()
	{
		return MsgType.GetLength() + Code.GetLength() + Echo.GetLength();
	}
};

#ifdef	__cplusplus
}
#endif

#endif	/* COMMMESSAGE_H */
