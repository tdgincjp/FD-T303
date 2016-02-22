#ifndef CONSTANT_H
#define	CONSTANT_H

#pragma warning(disable: 4996)

#ifdef	__cplusplus
extern "C" {
#endif


//#define unsigned char BYTE

#define APP_VERS "1.00"

#define INIT_DATA_DIR				"\\Saio\\user programs\\INIT\\"
#define EMV_KEY_DATA_DIR		"\\Saio\\user programs\\INIT\\EMVKEY\\"
#define SERVICE_DATA_FILE		"\\Saio\\user programs\\INIT\\SERVDATA"
#define EMV_DATA_FILE				"\\Saio\\user programs\\INIT\\EMVDATA"
#define CTLS_DATA_FILE			"\\Saio\\user programs\\INIT\\CTLSDATA"
#define TEMP_INIT_FILE			"\\Saio\\user programs\\INIT\\TEMPINIT"
#define TEMP_TOTALS_FILE		"\\Saio\\user programs\\INIT\\TEMPTOTS"
#define TOTALS_FILE					"\\Saio\\user programs\\INIT\\TOTALS"
#define TOTALS_FILE_TRAIN					"\\Saio\\user programs\\INIT\\TOTALSTRAIN"
#define HOST_TOTALS_FILE		"\\Saio\\user programs\\INIT\\HOSTTOTALS"

#define  TRANSACTION_LOG_FILE   "\\Saio\\user programs\\INIT\\TRANSACTIONLOG" 
#define  PREAUTH_LOG_FILE		"\\Saio\\user programs\\INIT\\PREAUTHLOG" 
#define  BATCH_LOG_FILE			"\\Saio\\user programs\\INIT\\BATCHLOG" 

#define EMV_KEY_FILE_PREFIX "EK"

#define HOST_RESP_LENGTH      1024
#define DID_k_LENGTH          39

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

// language
#define ENGLISH		0
#define FRENCH		1
  
// card type
#define NO_CARD     0
#define CARD_CREDIT 1
#define CARD_DEBIT  2
#define CARD_ADMIN  4

// account type
#define	ACCT_CHEQUE   '1'     // chequeing account
#define	ACCT_SAVING   '2'     // saving account
#define	ACCT_CREDIT   '4'     // default to Credit card
#define	ACCT_DEFAULT  '0'     // for Flash

// Size of transaction data fields
#define SZ_HEADER				48
#define SZ_TRAN_CODE            2
#define SZ_ACCOUNT              19
#define SZ_AMOUNT               7
#define SZ_TOTAL                9
#define SZ_EXPIRY_DATE          4
#define	SZ_SERVICE_CODE			2
#define	SZ_SERVICE_TYPE			2
#define SZ_TRACK2               40
#define SZ_TRACK1               80
#define SZ_APPROVAL_CODE        8
#define SZ_MAC					8
#define SZ_AUTOSET				6
#define SZ_TERMID               8
#define SZ_MERCHID				15
#define SZ_RESPONSE_TEXT		48
#define SZ_CLERKID_TEXT			6
#define SZ_RECEIPT_TEXT			12
#define SZ_TRANSMISSIONNO_TEXT	2
#define SZ_PASSWORD_TEXT        6
#define SZ_DEVICE_PASSWORD_TEXT 4
#define SZ_DATATIMEFORMAT       16

#define SZ_FULL_SEQUENCE_NO		10
#define SZ_BATCH_NO             3
#define SZ_SEQ_NO               3
#define SZ_SHIFT_NO             3
#define SZ_ISO_CODE             2
#define SZ_RESPONSE_CODE        3
#define SZ_CARD_DESCRIPTION     12
#define SZ_CUSTNAME             24
#define SZ_APPN_LABEL           16
#define SZ_APP_IDENTIFIER       32
#define SZ_TVR					10
#define SZ_AUTH_REQ_CRYPTO		16
#define SZ_FINAL_CRYPTO			16
#define SZ_TSI                  4
#define SZ_RID                  10
#define SZ_INVOICE_NO			7
#define SZ_CLERKID_NO			6
#define SZ_DEVICE_MODEL			10
#define SZ_APPROVALCODE_NO		8
#define SZ_FRAUD_NO				4


#define SZ_TRACK3               127
#define SZ_PIN_BLOCK			16
#define SZ_PIN_KSN				10
#define SZ_KEY					16
#define SZ_TRAN_DATE			6
#define SZ_TRAN_TIME			6
#define SZ_TRAN_DATE_TIME       12

#define SZ_TERM_CAPABILITY      3
#define SZ_ADD_TERM_CAPABILITY  5
#define SZ_TERM_COUNTRY_CODE	3

#define SZ_HOST_BUFF_MAX		1024

//SAIO INFO
#define SZ_INFO_FW_VERSION 4
#define SZ_INFO_HW_VERSION 4
#define SZ_INFO_OS_BUILD 4
#define SZ_INFO_SN_NUMBER 20
#define SZ_BYTE_SAIO_INFO 32

//PARAM DOWNLOAD
#define SZ_PARAM_TID 8
#define SZ_PARAM_SERIAL 20
#define SZ_PARAM_RECEIPT 40
#define SZ_PARAM_FOOTER 40
#define SZ_PARAM_CASHBACK_SURCHARGE 5
#define SZ_PARAM_MERCHANT_SURCHARGE 5
#define SZ_PARAM_LANGUAGE 10
#define SZ_PARAM_AUTOSETTLE_TIME 7
#define SZ_PARAM_TIP_GUIDE 2
#define SZ_PARAM_TIP_GUIDE 2
#define SZ_PARAM_TIP_GUIDE 2
#define SZ_PARAM_MERCHANT_NAME 20
#define SZ_PARAM_MERCHANT_PASSWORD 6
#define SZ_PARAM_CONTACTLESS_LIMIT 5
#define SZ_PARAM_BOOL 4

//DATAWIRE
#define SZ_DATAWIRE_TID 11
#define SZ_DATAWIRE_MID 16
#define SZ_DATAWIRE_GID 6
#define SZ_DATAWIRE_DID 25
#define SZ_DATAWIRE_CLIENT_REF 14
#define SZ_DATAWIRE_MAX_BUFFER 2047
#define SZ_RAPIDCONNECT_MAX_PAYLOAD 2047
#define SZ_DATETIME 16
#define SZ_STAN 10
#define SZ_REFERENCE 10
#define SZ_VERSION 10

//LOGFILE
#define SZ_SERIAL 10
#define SZ_CHAR 1
#define SZ_2CHAR 2
#define SZ_LOG_RECORD 51

// ECR request code definitions common
//#define ECR_PURCHASE            0
#define ECR_PRE_AUTH            1
#define ECR_ADVICE              2
#define ECR_FORCE_POST          3
#define ECR_REFUND              4

#define ECR_PURCHASE_CORR       11
#define ECR_REFUND_CORR         12
#define ECR_MC_EMVDATA          20
#define ECR_DISPLAY_MESSAGE     42
#define ECR_GET_PED_SERIAL      44
#define ECR_CLOSE_BATCH         60
#define ECR_RESET_TOTALS        66
#define ECR_DAY_TOTALS          67
#define ECR_HOST_INIT						90
#define ECR_EMV_INIT						91
#define ECR_KEY_EXCHANGE        96
#define ECR_CANCEL              97

#define KEY_ABORT       11
#define KEY_MSR         12
#define KEY_ICC         13
#define KEY_CTLS        14
#define KEY_CTLS_EMV    15
#define KEY_ERROR       16  // use UI_GetError() to get details
#define KEY_CTLS_ERROR  17
#define KEY_MANUAL      18
#define KEY_ICC_REMOVED 19
#define KEY_NO_CARD     20
//#define KEY_TIMEOUT     21
#define KEY_EVENT       22  
#define KEY_NOT_ALLOWED 23  
#define KEY_CHIP        24  


#define ERRORENTRY						300
#define ERRORENTRY1						301
#define ALERTENTRY						302


#define EMV_SELECT_LANGUAGE		500
#define EMV_CONFIRM_AMOUNT		501
#define EMV_SELECT_ACCOUNT		502
#define EMV_REMOVE_CARD 			503
#define DISPLAY_WINDOW 		    504
#define EMV_PASS_COUSTOMER	  505
#define EMV_START							506
#define EMV_WAIT							507
#define CANCELENTRY		        200

#define KEY_ENTER		1 
#define KEY_CANCEL	2 
#define KEY_CORR		3 
#define KEY_BUT1		4 
#define KEY_BUT2		5 
#define KEY_BUT3		6
#define KEY_TIMEOUT	7
#define KEY_CARD_REMOVED	8

#define EMV_NO_ERROR					0
#define EMV_ERROR_CANCEL			1
#define EMV_ERROR_TIMEOUT			2
#define EMV_ERROR_CARD_ABSENT	3
#define EMV_ERROR_DECLINED_BY_CARD	4
#define EMV_PARTIAL_TXN				5
#define EMV_CARD_NOT_MATCH				6
#define EMV_DEBIT_NOT_SUPPORT				7
#define EMV_CREDIT_NOT_SUPPORT				8

typedef enum
{
	EMV_STATE_START,
	EMV_STATE_APP_SELECTION,
	EMV_STATE_INIT_APP,
	EMV_STATE_READ_APP_DATA,
	EMV_STATE_OFFLINE_DATA_AUTH,
	EMV_STATE_PROCESS_RESTRICTION,
	EMV_STATE_CARD_HOLDER_VERIFICATION,
	EMV_STATE_TERM_RISK_MANAGEMENT,
	EMV_STATE_TERM_ACTION_ANALYSIS,
	EMV_STATE_ONLINE_PROCESSING,
	EMV_STATE_COMPLETION,	
	EMV_STATE_ERROR	
} eEmvState;

typedef enum
{
  ENTRY_MODE_SWIPED,
  ENTRY_MODE_MANUAL,
  ENTRY_MODE_CHIP,
  ENTRY_MODE_CTLS,
  ENTRY_MODE_CTLS_MSR
} eEntryMode;

// Transaction Status
/********************/
/* CONDITION CODES  */
/********************/
typedef enum
{
	ST_ECR_OK = 500,//1000,
	ST_OK,
	ST_CANCEL,                              // CANCEL entered during dialogue
	ST_TIMEOUT,                             // timer expired
	ST_ABORT,                               // ECR abort the transaction
	ST_NO_CARD,                             // timer expired while waiting for card entry
	ST_CARD_REMOVED,
	ST_CARD_DECLINE,
	ST_CHIP_ERROR,
	ST_CHIP_NOT_ALLOW,
	ST_CANNOT_SWIPE_CHIP,
	ST_CHIP_SWIPE_AT_ECR,
	ST_EMV_CARD_PROBLEM,
	ST_EMV_CARD_NOT_SUPPORT,
	ST_EMV_INVALID_CARD,
	ST_APPLICATION_BLOCKED,
	ST_CARD_BLOCKED,
	ST_CHIPCARD_ERROR,
	ST_NO_SUPPORTED_APP,
	ST_CHIP_TYPE_NOT_SUPPORT,
	ST_CARD_DECL_HOST_APPR,
	ST_CARD_DECL_HOST_DECL,
	ST_CARD_DECL_BAD_HOST,
	ST_CARD_DECL_NO_HOST,
	ST_CARD_DECL_NOT_CONNECTED,
	ST_CTLS_EMV_FAIL,
	ST_CARD_NOT_SUPPORT,
	ST_INIT_DONE,                           // initialization completed, send logon
	ST_BATCH1,                              // Close Batch part 1 done, fo part w
	ST_BATCH,                               // Batch closed, balanced, process
	ST_BATCH_NOBAL,                         // Batch closed, not balanced, process
	ST_AUTO_RETRY,                          // Auto retry requested
	ST_CAN_ACCEPT,                          // Cancellation req accepted; PP reset
	ST_BAD_TRNCD,                           // invalid transaction code
	ST_BAD_AMT,                             // invalid amount
	ST_NO_AMT,                              // no amount
	ST_BAD_TRK2,                            // invalid track2 data
	ST_NO_TRK2,                             // no track 2
	ST_BAD_APPR,                            // invalid approval code
	ST_NO_APPR,                             // no approval code
	ST_BAD_FS,                              // invalid or missing FS char
	ST_NO_EOT,                              // no EOT character
	ST_NO_FIELD,                            // unknown or illegal field
	ST_NOT_ALLOWED,                         // transaction not allowed
	ST_INVALID_CARD,                        // card not supported
	ST_REF_LIMIT,                           // over refund limit
	ST_INIT_REQ,                            // forced init requested
	ST_REQ_REJ,                             // req rejected, trn in progress
	ST_CANCEL_REJ,                          // Cancel req rejected, resp recvd
	ST_INIT_FAIL,                           // Initialization failure
	ST_MAC_ERR,                             // MAC error, forward reversal to host
	ST_NO_TRAN,                             // no transaction requested
	ST_BAD_DATA,                            // unrecoverable data error
	ST_UNDEFINE,                            // undefined error
	ST_HOST_TIMEOUT,                        // host response timeout
	ST_ERR_BASE24,                          // data error in Base24 response
	ST_ERR_BASE24_EMV,                      // data error in Base24 response
	ST_REVERSED,
	ST_TRAN_ERR_OP,
	ST_INVALID_CARD_SWIPED,
	ST_TENDER_MISMATCH,                     // Tender type mismatch on swipe
	ST_BAD_MSG,                             // bad message, non base24 from rb
	ST_REQ_ACCEPTED,
	ST_TRAN_COMPLETE,
	ST_RES_RECEIVED,
	ST_B24_OK,
	ST_B24_INVALID,
	ST_CONNECT_FAILED,
	ST_COM_ERROR,
	ST_ADMIN_COMM_FAILED,
	ST_INVALID_REF_NUM,                     // invalid Reference Number field on Air Miles
	ST_INVALID_FLAG,
	ST_KEYS_PROBLEM,
	ST_HOST_INIT_EMV_FAILURE,               // EMV DID or other EMV error
	ST_BAD_EMV_REQUEST_DATA,
	ST_BAD_EMV_ADD_REQUEST_DATA,
	ST_BAD_EMV_CHIP_CONDITION_CODE,
	ST_BAD_EMV_REASON_ONLINE_CODE,
	ST_NO_MC_EMV_DATA,
	ST_BAD_EMV_ISSUER_SCRIPT_RESULTS,
	ST_BAD_EMV_APPLICATION_LABEL,
	ST_BAD_EMV_APPL_PREFERRED_NAME,
	ST_BAD_EMV_CARD_NAME,
	ST_MISSING_EMV_CARD_NAME,
	ST_BAD_EMV_APPLICATION_ID,
	ST_BAD_EMV_ARQC,
	ST_BAD_EMV_TVR_ARQC,
	ST_BAD_EMV_TC_AAC,
	ST_BAD_EMV_TVR_TC_AAC,
	ST_BAD_EMV_CVM_INDICATOR,
	ST_BAD_TSI,
	ST_INVALID_ENCRYPTED_TRACK2,
	ST_INVALID_MSG_TXT,
	ST_INVALID_TRK_IND,
	ST_INVALID_MSG_TIMEOUT,
	ST_HOST_ERR,
	ST_HOST_INIT_REQUIRED,
	ST_EMV_NOT_INITIALIZED,
	ST_EMV_GET_CARD_DATA_FAILED,
	ST_EMV_HOST_ERR,
	ST_BAD_ORIG_AMT,
	ST_NO_ORIG_AMT,
	ST_SYTEM_ERROR,
	ST_REQUEST_ERROR,
	ST_BAD_RESPONSE,
	ST_NO_ECR_ID,
	ST_BAD_ECR_ID,
	ST_APPROVED,
	ST_DECLINED,
	ST_HOST_ERROR,
	ST_SEND_ERROR,
	ST_RESPONSE_ERROR,
	ST_ECR_INVALID_HEADER,
	ST_ECR_INVALID_MESSAGE_LENGTH,			//JC Dec 7/14
	ST_ECR_INVALID_MESSAGE_TYPE,
	ST_ECR_INVALID_TRAN_CODE,
	ST_ECR_MISSING_TAG,
	ST_ECR_INVALID_TAG_LENGTH,				//JC DEC 9/14
	ST_ECR_INVALID_TAG_FORMAT,				//JC DEc 9/14
	ST_ECR_INVALID_SYNC,					//JC Dec 10/14 Missing Sync bytes in header 
	ST_ECR_INVALID_AUDIT					//JC Dec 10/14 Invalid audit byte
} eCondCodeStatus;

typedef enum
{
	NO_REVERSAL,
	CONTROLLER_REVERSAL,
	MAC_REVERSAL,
	CHIP_REVERSAL,
	TIMEOUT_REVERSAL
} eReversalType;

typedef enum
{
  STS_DLL_OK,
  STS_DLL_VIRGINTERMINAL,
  STS_DLL_LASTDOWNLOADFAILED,
  STS_DLL_HOSTREQUESTEDDOWNLOAD,
  STS_DLL_FORCEDOWNLOAD,
  STS_DLL_DOWNLOAD_REQUIRED,
  STS_DLL_NO_KEYS
} eDownloadStatus;

//FirstData RapidConnect Download
typedef enum
{
	DL_STAGE_START = 2000,
	DL_STAGE_TABLE,
	DL_STAGE_BIN,
	DL_STAGE_BIN_MORE_PAGES,
	DL_STAGE_FINISH,
	DL_STAGE_ERROR = 2100
} eDownloadStage;

//FirstData Transaction Status
typedef enum
{
	FD_STATUS_INIT = 3000,
	FD_STATUS_DOWNLOAD,
	FD_STATUS_TRAN_START,
	FD_STATUS_TRAN_PAYLOAD,
	FD_STATUS_DL_PAYLOAD,
	FD_STATUS_TRAN_READY,
	FD_STATUS_DL_READY,
	FD_STATUS_TRAN_SUCCESS,
	FD_STATUS_TRAN_FINISH,
	FD_STATUS_TRAN_ERROR = 3100,
	FD_STATUS_COMM_ERROR,
	FD_STATUS_PARSE_ERROR,
	FD_STATUS_PAYLOAD_ERROR,
	FD_STATUS_DATAWIRE_WRAP_ERROR,
	FD_STATUS_DOWNLOAD_ERROR,
	FD_STATUS_RESP_LEN
} eFDStatus;

typedef enum
{
	DW_INVALID_SESSION = 6,
	DW_NETWORK_ERROR8 = 8,
	DW_HOST_BUSY = 200,
	DW_HOST_UNAVAILABLE, 
	DW_HOST_CONNECT_ERROR,
	DW_HOST_DROP, 
	DW_HOST_COMM_ERROR, 
	DW_NO_RESPONSE,
	DW_HOST_SEND_ERROR, 
	DW_TRANSPORT_TIMEOUT = 405,
	DW_NETWORK_ERROR = 505
} eDatawireStatusCodes;

//LogFile RecordTypeCode
typedef enum
{
	LOG_APPLICATION,
	LOG_DEBUG
}eRecordTypeCodes;

//LogFile EventType
typedef enum
{
	EVENT_CREATE,
	EVENT_START,
	EVENT_EXIT,
	EVENT_ERROR
}eEventTypes;

typedef	enum
{
	TRAN_UNKNOWN = 0,

	// Financial Transactions
	FUNCTION_FINAN_BEGIN = 20,
		TRAN_PURCHASE,
		TRAN_PREAUTH,
		TRAN_ADVICE,
		TRAN_FORCE_POST,
		TRAN_REFUND,
		TRAN_PURCHASE_VOID,
		TRAN_REFUND_VOID,
	FUNCTION_FINAN_END,

	FUNCTION_GENERAL_BEGIN,
		TRAN_CANCEL,
	FUNCTION_GENERAL_END,

	// Administrative Transactions
	FUNCTION_ADMIN_BEGIN = 101,
		TRAN_HOST_INIT,
		TRAN_KEY_EXCHANGE,
		TRAN_CLOSE_BATCH,
		TRAN_HANDSHAKRE,
		TRAN_BATCH_TOTAL,
	FUNCTION_ADMIN_END,

	FUNCTION_LOCAL_BEGIN = 200,
		TRAN_EXTENDED_CFG,
		TRAN_GET_TRACK_DATA,
		TRAN_GET_PED_SERIAL,
	FUNCTION_LOCAL_END,
	
	FUNCTION_EXIT_APP_BEGIN = 300,
		FUNCTION_EXIT_APP,
	FUNCTION_EXIT_APP_END,

	// keep this last
	END_OF_TRAN_ID
} eEcrTxnType;


/***************************
 * File Data ID Definitions
 ***************************/
typedef enum
{
  FID_CFG_BEGIN = 0,
  FID_CFG_LANGUAGE,
  FID_CFG_PINENTRY_TMO,
  FID_CFG_BALDISP_TMO,
  FID_CFG_CUSTENTRY_TMO,
  FID_CFG_HOST_TMO,
  FID_CFG_ECR_TMO,
  FID_CFG_IP_SETUP,
  FID_CFG_CTLS_READER_ENABLE,
  FID_CFG_CHIP_PREAUTH_ENABLED,
  FID_CFG_CASHBACK_FLG,
  FID_CFG_CASHBACK_LIMIT,
  FID_CFG_DEVICE_MODEL,
  FID_CFG_INVOICE,
  FID_CFG_EMVCARD_CHECK,
  FID_CFG_SURCHARGE_LIMIT,
  FID_CFG_END
} eFieldIdConfig;
  
typedef enum
{
  FID_MERCH_CFG_BEGIN = 100,
  FID_MERCH_CFG_TERM_ID,
  FID_MERCH_CFG_NAME,
  FID_MERCH_CFG_ADDRESS,
  FID_MERCH_CFG_CITY_PROV,
  FID_MERCH_CFG_PHONE,
  FID_DEBIT_ONLY_TERMINAL_FLAG,
  FID_MERCH_CFG_PHONE2,
  FID_MERCH_CFG_END,
} eFieldIdMerchantConfigData;
  
typedef enum
{
  FID_TERM_SAVED_BEGIN = 200,
  FID_TERM_PIN_KEY,
  FID_TERM_MAC_KEY,
  FID_TERM_DECRY_KEY,
  FID_TERM_OLD_PIN_KEY,
  FID_TERM_OLD_MAC_KEY,
  FID_TERM_OLD_DECRY_KEY,
  FID_TERM_MAC_REQ_FIDS,
  FID_TERM_MAC_RSP_FIDS,
  FID_TERM_HOST_INIT_STATUS,
  FID_TERM_EMV_ENABLE_FLG,
  FID_TERM_CTLS_ENABLE_FLG,
  FID_TERM_INV_NUMBER,
  FID_TERM_SEQ_NUMBER,
  FID_TERM_TRANSMISSION_NUMBER,
  FID_TERM_EMV_FALLBACK,
	FID_TERM_ALLOWED_TRANS,
	FID_TERM_REVERSAL_RECORD,
  FID_TERM_SAVED_END,
  FID_TERM_SHIFT_NUMBER,
  FID_TERM_BATCH_NUMBER,
	FID_TERM_SEQ_FLAG,
  FID_CLOSE_BATCH_FLAG,
  FID_TERM_LANGUAGE_FLAG,
  FID_TERM_AUTOSET_TIME,
  FID_TERM_AUTOSET_FLAG,
  FID_EMV_VERSION,
} eFieldIdTerminalData;

typedef enum
{
  FID_EMV_SAVED_BEGIN = 300,
  FID_EMV_TRANSACTION_SN,                     
  FID_EMV_TERMINAL_CAPABILITIES,                     
  FID_EMV_ADDITIONAL_TERMINAL_CAPABILITIES,           
  FID_EMV_TERMINAL_COUNTRY_CODE_NUMERIC,              
  FID_EMV_TERMINAL_COUNTRY_CODE_ALPHA,                
  FID_EMV_TERMINAL_TYPE,                              
  FID_EMV_TRAN_CURRENCY_CODE,                  
  FID_EMV_TRAN_CURRENCY_EXPONENT,              
  FID_EMV_TRAN_REFERENCE_CURRENCY_CODE,        
  FID_EMV_TRAN_REFERENCE_CURRENCY_EXPONENT,    
  FID_EMV_TRAN_REFERENCE_CURRENCY_CONVERSION,
  FID_EMV_AUTHORIZATION_RESPONSE_CODE_LIST,
  FID_EMV_CTLS_RECEIPT_REQUIRED_LIMIT,
  FID_EMV_MERCHANT_TYPE_INDICATOR,
  FID_EMV_TERMINAL_OPTION_STATUS,
  FID_EMV_TERMINAL_TRAN_INFO,
  FID_EMV_FLASH_TERMINAL_CAPABILITIES,                     
  FID_EMV_SAVED_END  
} eFieldIdEmvData;

typedef enum
{
  FID_EMV_TAG_BEGIN = 380,
  FID_EMV_TAG_VISA_CTLS_TXN_LIMIT,                     
  FID_EMV_TAG_VISA_CTLS_CVM_LIMIT,                     
  FID_EMV_TAG_MAST_CTLS_TXN_LIMIT,                     
  FID_EMV_TAG_MAST_CTLS_CVM_LIMIT,                     
  FID_EMV_TAG_END  
} eFieldId;

typedef enum
{
  FID_PASSWORD_BEGIN = 400,
  FID_PASSWORD_CLERK,                     
  FID_PASSWORD_MANAGER,                     
  FID_PASSWORD_END  
} eFieldIdPassword;

typedef enum
{
  FID_STATICS_BEGIN = 410,
  FID_STATICS_CHIP_CARD_FAIL,                     
  FID_STATICS_PIN_ENTRY_ERROR,                     
  FID_STATICS_OFFLINE_FUNDS_DECLINE,                     
  FID_STATICS_TECHNICAL_FALLBACK,                     
  FID_STATICS_END  
} eFieldIdStaticsReport;

typedef enum
{
  FID_OFFLINE_BEGIN = 420,
  FID_OFFLINE_PAN,                     
  FID_OFFLINE_PAN_SEQ,                     
  FID_OFFLINE_IAC_DEFAULT,                     
  FID_OFFLINE_IAC_DENIAL,                     
  FID_OFFLINE_IAC_ONLINE,                     
  FID_OFFLINE_TAC_DEFAULT,                     
  FID_OFFLINE_TAC_DENIAL,                     
  FID_OFFLINE_TAC_ONLINE,
  FID_OFFLINE_DE55,
  FID_OFFLINE_9F26,
  FID_OFFLINE_9F27,
  FID_OFFLINE_9F10,
  FID_OFFLINE_9F37,
  FID_OFFLINE_9F36,
  FID_OFFLINE_95,
  FID_OFFLINE_9A,
  FID_OFFLINE_9C,
  FID_OFFLINE_9F02,
  FID_OFFLINE_5F2A,
  FID_OFFLINE_82,
  FID_OFFLINE_5A,
  FID_OFFLINE_9F1A,
  FID_OFFLINE_9F34,
  FID_OFFLINE_9F03,
  FID_OFFLINE_5F34,
  FID_OFFLINE_8A,
  FID_OFFLINE_DEBIT,

  FID_OFFLINE_END  
} eFieldIdOfflineReport;



typedef enum
{
	FID_PARAM_TIP_TYPE =700,
	FID_PARAM_AUTOSETTLE_TIME,
	FID_PARAM_DEBIT_REFUND_ENABLED,
	FID_PARAM_CREDIT_REFUND_ENABLED,
	FID_PARAM_VOID_ENABLED,
	FID_PARAM_PAY_BY_CUSTOMER,
	FID_PARAM_PREDIAL_ENABLED,
	FID_PARAM_MERCHANT_NAME,
	FID_PARAM_MERCHANT_PASSWORD,
	FID_PARAM_VOID_PASS_TYPE,
	FID_PARAM_RETURN_PASS_TYPE,
	FID_PARAM_SETTLE_PASS_TYPE,
	FID_PARAM_FORCE_PASS_TYPE,
	FID_PARAM_CONTACTLESS_LIMIT

	//we now save all these to actual variables
	//FID_PARAM_RECEIPT_LINE1 = 700,
	//FID_PARAM_RECEIPT_LINE2,
	//FID_PARAM_RECEIPT_LINE3,
	//FID_PARAM_RECEIPT_LINE4,
	//FID_PARAM_FOOTER_LINE1,
	//FID_PARAM_FOOTER_LINE2,
	//FID_PARAM_FOOTER_LINE3,
	//FID_PARAM_FOOTER_LINE4,
	//FID_PARAM_CASHBACK_SURCHARGE,
	//FID_PARAM_MERCHANT_SURCHARGE,
	//FID_PARAM_ID_LANGUAGE,
	//FID_PARAM_MANUAL_CARD_ENTRY,
	//FID_PARAM_TIP_GUIDE1,
	//FID_PARAM_TIP_GUIDE2,
	//FID_PARAM_TIP_GUIDE3,
	//FID_PARAM_TIP_GUIDE_ENABLED,
	//FID_PARAM_CASHBACK_ENABLED,
} eParamDownload;


typedef enum
{
	FID_INFO_FW_VERSION = 750, 
	FID_INFO_HW_VERSION,
	FID_INFO_OS_BUILD,
	FID_INFO_SN_NUMBER,
}eSaioGetInfo;


#define FID_UPDATE_ALL     500

/*--- card error */
#define CARD_NO_ERROR                0
#define CARD_ERR_CARD_PROBLEM        1
#define CARD_ERR_INV_CARD_TYPE       2
#define CARD_ERR_CARD_NOT_SUPPORT    3
#define CARD_ERR_AMOUNT_EXCEEDED     4
#define CARD_ERR_INVALID_TENDER      5
#define CARD_ERR_INVALID_MODE        6
#define CARD_ERR_CREDIT_NO_APPR      7
#define CARD_ERR_CHIP_SWIPE_AT_ECR   8
#define CARD_ERR_CANNOT_SWIPE_CHIP   9

#define KEY_WRONG_TENDER      		80
#define KEY_WRONG_TYPE        		81
#define KEY_AMOUNT_EXCEED     		82
#define KEY_CANCEL_BL         		83
#define KEY_NO_APPROVAL       		84
#define KEY_EMV_TRAN          		85
#define KEY_PIN_CNTR_EXHAUST  		86
#define KEY_CARD_DECLINE      		87
#define KEY_CHIP_ERROR        		88
#define KEY_EMV_COMM_ERROR    		89
#define KEY_CHIP_NOT_ALLOW    		90
#define KEY_CANNOT_SWIPE_CHIP 		91
#define KEY_EMV_CARD_ERROR    		92
#define KEY_CTLS_EMV_TRAN     		93
#define KEY_CTLS_EMV_CARD_ERROR		94
#define KEY_CANNOT_KEY_CHIP 			95
#define KEY_CARD_DECLINE_HOST_APPROVED	96
#define KEY_CARD_DECLINE_HOST_DECLINED	97
#define KEY_CARD_DECLINE_BAD_HOST 			98
#define KEY_CARD_DECLINE_NO_HOST 				99
#define KEY_NOT_SUPPORTED        				100

/*
#define ECR_PURCHASE            0
#define ECR_PRE_AUTH            1
#define ECR_ADVICE              2
#define ECR_FORCE_POST          3
#define ECR_REFUND              4
#define ECR_BALANCE_INQ         7
#define ECR_PURCHASE_CORR       11
#define ECR_REFUND_CORR         12
#define ECR_VOID_LAST           13
#define ECR_GET_PED_SERIAL      44
#define ECR_CLOSE_BATCH         60
#define ECR_CLERK_TOTALS        64
#define ECR_MERCH_SUBTOTALS     65
#define ECR_RESET_TOTALS        66
#define ECR_EXTENDED_CFG        87
#define ECR_INITIALIZATION      90
#define ECR_EMV_INITIALIZATION  91
#define ECR_HANDSHAKE           95
#define ECR_KEY_EXCHANGE        96
#define ECR_CANCEL              97
*/
#define RID_INTERAC     "A000000277"
#define RID_MASTER_CARD "A000000004"
#define RID_VISA_CARD   "A000000003"
#define RID_AMEX_CARD   "A000000025"
#define RID_JCB_CARD    "A000000065"

#define AID_MAESTRO     "A0000000043060"
#define AID_VISA_DEBIT  "A0000000030201"
#define AID_MASTERCARD  "A0000000041010"
#define AID_VISA        "A0000000031010"
#define AID_INTERAC     "A0000002771010"


#ifdef	__cplusplus
}
#endif

#endif
