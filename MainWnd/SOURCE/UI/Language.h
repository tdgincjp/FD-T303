

#pragma once

class CLanguage
{
private:
//	static int m_usLanguage;
// Class enumerations
public:
	// Communication event
	typedef enum
	{
		IDX_CORR,
		IDX_OK2,
		IDX_CANCEL,
		IDX_CHQ,
		IDX_SAV,
		IDX_CRD,

	  IDX_SELECTED,           
	  IDX_DO_NOT,
	  IDX_REMOVE_CARD,        
	  IDX_SELECT,             
	  IDX_YES,             
	  IDX_NO,              
		IDX_EMV_APPROVED,
		IDX_EMV_DECLINED,
		IDX_EMV_ENTER_PIN,
		IDX_EMV_PIN_OK,
		IDX_EMV_INCORRECT_PIN,
		IDX_EMV_CANCEL_OR_ENTER,
		IDX_EMV_CARD_ERROR,
		IDX_EMV_CHIP_ERROR,
		IDX_EMV_PLEASE_WAIT,
		IDX_EMV_PROCESSING,
		IDX_EMV_ERROR,
		IDX_EMV_REMOVE_CARD,
		IDX_EMV_USE_CHIP_READER,
		IDX_EMV_USE_MAGSTRIPE,
		IDX_EMV_TRY_AGAIN,
	  IDX_OK,                 
	  IDX_SELECT_ACCT,     
	  IDX_CARD_REMOVED,    

		IDX_ERROR,            

		IDX_COMMUNICATIONS,
		IDX_REVERSED,
		IDX_SOCKET_CONNECT,
		IDX_SOCKET_SENDING,
		IDX_SOCKET_RECEIVING,
		IDX_SOCKET_PROCESSING,
		IDX_SOCKET_NOTSETUP,
		IDX_SOCKET_SENDING_ERROR,
		IDX_HOST_INIT_DONE,
		IDX_SOCKET_RECV_TIMEOUT,
		IDX_KEY_EXCHANGE_ERROR,

	  IDX_PURCHASE,        
	  IDX_REFUND,          
	  IDX_PREAUTH,         
		IDX_VOID,
		IDX_VOIDED,
		IDX_COMPLETION,
		IDX_FORCEPOST,

		IDX_ADMIN,
		IDX_OEMID,
		IDX_PROCESSORS_NUMBER,
		IDX_ARCHITECTURER,
		IDX_PROCESSOR_MASK,
		IDX_OS_TEST_PERFORMED,
		IDX_SELF_TEST_PERFORMED,
		IDX_VERSION_NUMBER,
		IDX_INIT_EMV,

		IDX_ADMIN_MENU,
		IDX_CONFIG_MENU,
		IDX_REPORT_MENU,

		IDX_REPRINT_MENU,
		IDX_LANGUAGE_MENU,
		IDX_SOUND_MENU,
		IDX_TRAINING_MENU,
		IDX_COMMS_TEST_MENU,
		IDX_CLERK_PSWD_MENU,
		IDX_MNGR_PSWRDT_MENU,
		IDX_DOWNLOAD_MENU,
		IDX_KEY_EXCHANGE_MENU,
		IDX_POSX_DOWNLOAD_MENU,

		IDX_GENERAL_PARMS,
		IDX_TERM_CONFIG,
		IDX_DIAL_CONFIG,
		IDX_ETHERNET_CNFG,
		IDX_PRINTER_CNFG,

		IDX_TERM_ID,
		IDX_COMM_TYPE,
		IDX_CLERK_ID,
		IDX_FRAUD_CHECK,
		IDX_MANUAL_ENTRY,
		IDX_TIP_PROMPT,
		IDX_INVOICE_MENU,
		IDX_SURCHARGE_MENU,
		IDX_CASHBACK_MENU,
		IDX_PREAUTH_AGING_DAYS_MENU,
		IDX_DAYS,

		IDX_PHONE_MENU,
		IDX_PHONE2_MENU,
		IDX_DIAL_MODE_MENU,
		IDX_BAUD_RATE_MENU,

		IDX_HOST_ADDR,
		IDX_HOST_SETUP,
		IDX_IP_PORT,
		IDX_DIAL_BACKUP,
		IDX_STATIC_IP,
		IDX_HEADER_MSG,
		IDX_TRAILER_MSG,
		IDX_COPY_LOG,
		IDX_RESTART,
		IDX_SHOW_MORE,
		IDX_GO_BACK,
		IDX_DEBUG_API,									//JC Nov 11/15 ADD DEBUG API
		IDX_ENABLE,										//JC Nov 11/15 ADD DEBUG API
	    IDX_DISABLE,									//JC Nov 11/15 ADD DEBUG API

		IDX_BATCH_TOTALS,
		IDX_BATCH_CLOSE,
		IDX_INVOICE_DET,
		IDX_CARD_DETAIL,
		IDX_CARD_SUMMARY,
		IDX_DEBIT_SUMMARY,
		IDX_CLERK_DETAIL,
		IDX_TIP_DETAIL,
		IDX_PRE_AUTH_MENU,
		IDX_EMV_INFO,

		IDX_AID_PARAMS,
		IDX_CAPK_INFO_MENU,
		IDX_EMV_STATISTIC_MENU,
		IDX_OFFLINE_INFO,
	
		IDX_CHEQUE,
		IDX_SAVING,
		IDX_TIME_OUT,
		IDX_ENTER_PASSWORD,
		IDX_ENTER_AMOUNT,
		IDX_ENTER_CLERK,
		IDX_ID_NAME,
		IDX_ENTER_RECEIPT,
		IDX_NUMBER_NAME,
		IDX_INSERT_SWIPE_TAP,
		IDX_SWIPE_TAP,
		IDX_INSERT_SWIPE,
		IDX_SWIPE,
		IDX_KEY_CARD_NUMBER,
		IDX_KEY_CARD_NUMBER1,
		IDX_FAIL_OPEN_DEVICE,
		IDX_PASS_TO,
		IDX_CUSTOMER,
		IDX_CLERK,
		IDX_ENTER_CASHBACK,
		IDX_AMOUNT,
		IDX_PLEASE_WAIT,
		IDX_FEE,
		IDX_APPROVED,
		IDX_PARTIAL_APPROVED,
		IDX_AMOUNT_DUE,
		IDX_AVAIL_BAL,
		IDX_UNSUCCESSFUL,
		IDX_SELECT_ACCOUNT,
		IDX_RETRIEVE_CARD,
		IDX_PASS_TO_CLERK,
		IDX_AUTH_CODE,
		IDX_INVO_NUMBER,
		IDX_DECLINED,
		IDX_PRESS_OK_FOR,
		IDX_CUSTOMER_COPY,
		IDX_ENTER_EXPIRY,
		IDX_DATE,
		IDX_IMPRINT_CARD,
		IDX_PRESS_OK,
		IDX_DO_NOT_REMOVE,
		IDX_CARD,
		IDX_PLEASE,
		IDX_ENTER_LAST,
		IDX_DIGITS,
		IDX_TRANSACTION,
		IDX_CANCELLED,
		IDX_CANCELL,
		IDX_CARD_NOT_SUPPORT,

		IDX_DUPLICATE,
		IDX_RECEIPT_TERM,
		IDX_RECEIPT_MERCH,
		IDX_RECEIPT_INVOICE,
		IDX_RECEIPT_RECEIPTNUM,
		IDX_RECEIPT_CLERK,
		IDX_RECEIPT_TRANS,
		IDX_RECEIPT_AUTH,
		IDX_RECEIPT_BATCH_ONE,

		IDX_RECEIPT_CARD,
		IDX_RECEIPT_CARDTYPE,
		IDX_RECEIPT_CREDIT,
		IDX_RECEIPT_DEBIT,
		IDX_RECEIPT_DATE,
		IDX_RECEIPT_TIME,
		IDX_RECEIPT_RECEIPT,
		IDX_RECEIPT_REFERENCE,

		IDX_RECEIPT_SALE,
		IDX_RECEIPT_SALE_VOID,
		IDX_RECEIPT_REFUND,
		IDX_RECEIPT_REFUND_VOID,
		IDX_RECEIPT_PREAUTH,
		IDX_RECEIPT_COMPLETION,
		IDX_RECEIPT_FORCE_POST,

		IDX_RECEIPT_AMOUNT,
		IDX_RECEIPT_CASHBACK,
		IDX_RECEIPT_CASHBACK_FEE,
		IDX_RECEIPT_TIP,
		IDX_RECEIPT_SURCHANGE,
		IDX_RECEIPT_TOTAL,

		IDX_RECEIPT_CHIP_CARD_SWIPED,
		IDX_RECEIPT_CHIP_CARD_KEYED,
		IDX_RECEIPT_TRANSACTION,
		IDX_RECEIPT_APPROVED,
		IDX_RECEIPT_NOT_COMPLETED,
		IDX_RECEIPT_NOT_APPROVED,

		IDX_RECEIPT_VERIFIED_BY_PIN,
		IDX_SIGNATURE_NOT_REQUIRED,
		IDX_RECEIPT_MERCHANT_COPY,
		IDX_RECEIPT_CUSTOMER_COPY,
		IDX_RECEIPT_CANCELLED,

		IDX_RECEIPT_CHIP_CARD_MALFUNCTION,

		IDX_RECEIPT_THANK_YOU1,
		IDX_RECEIPT_THANK_YOU2,
		IDX_RECEIPT_THANK_YOU3,
		IDX_RECEIPT_THANK_YOU4,
		IDX_RECEIPT_THANK_YOU5,

		IDX_RECEIPT_THANK_YOU21,
		IDX_RECEIPT_THANK_YOU22,
		IDX_RECEIPT_THANK_YOU23,
		IDX_RECEIPT_THANK_YOU24,
		IDX_RECEIPT_THANK_YOU25,

		IDX_RECEIPT_CARDHOLDER_SIGNATURE,

		IDX_RECEIPT_THANK_YOU11,
		IDX_RECEIPT_THANK_YOU12,
		IDX_RECEIPT_THANK_YOU13,
		IDX_RECEIPT_THANK_YOU14,
		IDX_RECEIPT_THANK_YOU15,

		IDX_HOST_OPEN_BATCH_TOTALS,
		IDX_HOST_BATCH_TOTALS,
		IDX_STORE_BATCH_TOTALS,
		IDX_END_OF_REPORT,

		IDX_RECEIPT_BATCH,
		IDX_RECEIPT_OPEN,
		IDX_RECEIPT_CLOSE,
		IDX_BATCH_IN_BALANCE,
		IDX_BATCH_OUT_OF_BALANCE,

		IDX_RECEIPT_BATCH_CLOSED,
		IDX_RECEIPT_BATCH_NOT_CLOSED,
		IDX_RECEIPT_BATCH_CLOSE_TOTALS,

		IDX_RECEIPT_RETURN, 
		IDX_RECEIPT_VOID, 
		IDX_RECEIPT_NET_TOT,

		IDX_INVOICE_DETAIL_REPORT,

		IDX_RECEIPT_SURCHARGE,
		IDX_CARD_DETAIL_REPORT,

		IDX_RECEIPT_CARD_TOTALS, 
		IDX_RECEIPT_SUB, 
		IDX_RECEIPT_TXN, 
		IDX_RECEIPT_ALL_TRANSACTIONS, 
		IDX_RECEIPT_COMPL, 
		IDX_CARD_SUMMARY_REPORT, 

		IDX_RECEIPT_SURCH, 
		IDX_DEBIT_SUMMARY_REPORT, 

		IDX_RECEIPT_REPORT_CLERK, 
		IDX_RECEIPT_REPORT, 
		IDX_ALL_OPERATORS_REPORT, 
		IDX_RECEIPT_INV,
		IDX_RECEIPT_NONE,
		IDX_RECEIPT_TIPS,
		IDX_RECEIPT_CLERK_ID,
		IDX_TIP_REPORT,

		IDX_PREAUTH_REPORT,

		IDX_PRESS_OK_CONFIRM,
		IDX_PRESS_OK_TO,
		IDX_CLOSE_BATCH,
		IDX_PRINTING,
		IDX_CHANGE,
		IDX_CONFIRMED,
		IDX_CONFIRM,
		IDX_CONFIRM_NEW,
		IDX_PREV,
		IDX_NEXT,
		IDX_ENTER_INVOICE,
		IDX_VOID_INV,
		IDX_ENTER_FEE,
		IDX_ENTER_NEW,
		IDX_HOST_ADDRESS,
		IDX_PASSWORD,
		IDX_PHONE_NUMBER,
		IDX_NO_NETWORK,
		IDX_MISMATCHED,
		IDX_DUP_INVOICE,
		IDX_RE_ENTER,
		IDX_ENTER,

		IDX_INVOICE,
		IDX_MODE,
		IDX_ENTER_TIP,
		IDX_LOCKED,

		IDX_LOCKED_ENTERD,
		IDX_SUPER_PASSWORD,
		IDX_MANAGER_PASSWORD,
		IDX_CLERK_PW,
		IDX_MANAGER_PW,

		IDX_NON_INITIALIZED,
		IDX_HOST_DOWNLOAD,
		IDX_REQUIRED,
		IDX_EMV_AID,
		IDX_EMV_CAPK,
		IDX_TERMINAL_ID,
		IDX_OPTION,
		IDX_FIRST_TIP,
		IDX_SECOND_TIP,
		IDX_THIRD_TIP,
		IDX_PERCENT,

		IDX_EXITING,
		IDX_TRAINING_MODE,
		IDX_INITIALIZING,
		IDX_ENTERING,
		IDX_NO_RECORDING_LOG,

		IDX_CARD_NOT_MATCH,
		IDX_APPROVAL_CODE,
		IDX_DEBIT_NOT_SUPPORT,
		IDX_CAN_NOT_PRINT,
		IDX_CANNOT_SWIPE_CHIP,
		IDX_DECLINED_BY_CARD,

		IDX_CARD_BLOCKED,
		IDX_NO_SUPPORTED,
		IDX_APPLICATION,
		IDX_FRAUD_CARD,
		IDX_BLOCKED,
		IDX_NOT_ACCEPTED,
		IDX_CARD_PROCESS,
		IDX_FALLBACK_TO_MAGSTRIPE,
		IDX_PLEASE_REMOVE_CARD,
		IDX_REMOVE_CARD_QUICKLY,
		IDX_DEBIT_CARD,
		IDX_CREDIT_CARD,
		IDX_MSR_OPEN_FAIL,
		IDX_EEED_DEBIT_CARD,
		IDX_NO_PREAUTH_RECORDING,
		IDX_BT_YES,
		IDX_BT_NO,
		IDX_VISA,
		IDX_MASTERCARD,
		IDX_AMERICAN_EXPRESS,
		IDX_DISCOVER,
		IDX_JCB,
		IDX_DINERS_CLUB,
		IDX_PROPRIETARY_DEBIT,

		IDX_MANUAL,
		IDX_SWIPED,
		IDX_CHIP,
		IDX_CTLS,
		IDX_PORT,
		IDX_IP_PORT_NUMBER,
		IDX_UNKNOWN,
		IDX_BT_ADMIN,
		IDX_BT_FINAN,
		IDX_ENTER_ONLINE_PIN,

		IDX_EMV_KERNEL_ID,
		IDX_TERMINAL_TYPE,
		IDX_TERMINAL_CAPABILITIES,
		IDX_ADD_TERM_CAPABILITIES,
		IDX_TRANSACTION_CURRENCY_CODE,
		IDX_AID_NAME,
		IDX_AID,
		IDX_APP_VER_NO,
		IDX_FLOOR_LIMIT,
		IDX_THRESHOLD,
		IDX_TARG_PERCENT,
		IDX_MAX_TARG_PERCENT,
		IDX_DEFAULT_TDOL,
		IDX_DEFAULT_DDOL,
		IDX_TAC_DEFAULT,
		IDX_TAC_DENIAL,
		IDX_TAC_ONLINE,
		IDX_FALLBACK_ALLOWED,
		IDX_EMV_CAPK_REPORT,
		IDX_EMV_AID_REPORT,
		IDX_EMV_STATISTIC_REPORT,
		IDX_RID,
		IDX_INDEX,

		IDX_AMOUNT_ABOVE,
		IDX_MAXIMUM,

		IDX_ENTER_MAXIMUM,
		IDX_ENTER_LIMIT,

		IDX_RFID_ERROR,

		IDX_INVALID_EXPIRY_DATE,

		IDX_RECOMMENDED_APPNAME,
		IDX_RCONTACTLESS_PARAMETERS,

		IDX_EMV_APP_VERSION,
		IDX_MSD_APP_VERSION,
		IDX_CVM_LIMIT,
		IDX_TXN_LIMIT,
		IDX_TOS,
		IDX_RRL,
		IDX_TTI,
		IDX_TTQ,
		IDX_MTI,

		IDX_KEY_MODULUS,
		IDX_KEY_EXPONENT,
		IDX_BATCH_IS_EMPTY,
		IDX_DO_PRINT_RECEIPT,

		IDX_RECEIPT_TYPE,
		IDX_CUST,
		IDX_MERCH,

		IDX_SETTLE_TYPE_MENU,
		IDX_CLEAR_BATCH_MENU,
		IDX_WARNING_CLEAR,
		IDX_CURRENT_BATCH,
		IDX_INVOICE_NUMBER,
		IDX_IS_NOT_FOUND,

		IDX_ENTER_ORIGINAL,

		IDX_CHIP_CARD_READ_FAIL,
		IDX_PIN_ENTRY_ERROR,
		IDX_OFFLINE_FUNDS_DECLINE,
		IDX_TECHNICAL_FALLBACK,

		IDX_INVALID_CARD_NUMBER,

		IDX_OFFLINE_DECLINED_TRAN,

		IDX_PAN,
		IDX_PAN_SEQ,
		IDX_IAC_DEFAULT,
		IDX_IAC_DENIAL,
		IDX_IAC_ONLINE,
		IDX_DE55,

		IDX_CONTACTLESS_TRANS,
		IDX_LIMIT_EXCEEDED,
		IDX_INSERT_SWIPE_CARD,

		IDX_ENTRY_MODE,
		IDX_TAP_FAILED_PLEASE,
		IDX_SEE_PHONE,
		IDX_LAST_PIN_TRY,
		IDX_SELECT_LANGUAGE,

		IDX_IP_ADDR,
		IDX_IP_MASK,
		IDX_GATEWAY,
		IDX_DNS,

		IDX_SOFT_KEYBOARD,

		IDX_PLEASE_INSERT,
		IDX_CHIP_CARD,
		IDX_SURC_THRESHOLD,

		IDX_CHEQUING,
		IDX_SAVINGS,
		IDX_DEFAULT,
		IDX_ENTER_SETTLE,
		IDX_FLEX_TIME,
		IDX_LINE,
		IDX_HEADER_LINE,
		IDX_TRAILER_LINE,
		IDX_PARAM_DNLD_MENU,
		IDX_BACK,
		IDX_EDIT,
		IDX_COPY,
		IDX_SAVE,
		IDX_TEST,
		IDX_PRODUCT,
		IDX_FAILED_TO,
		IDX_READ_SWIPE,
		IDX_MERCHANT_ID,
		IDX_TIMEZONE_OFFSET,
		IDX_END
	} 
	TextIndex;

// Construction
public:
	CLanguage();
//	virtual ~CLanguage();
	static int m_usLanguage;
	static HWND  m_hWnd; 

	static BOOL  m_bPassToCustomer;
	static BOOL  m_bLanguageSelected;
	static int m_CustomerLanguage;

	static void SetLanguage(int usLanguage);
	static void SetLanguage();
	static int GetLanguage();
	static void GetTextFromIndex(int usIndex,char** pText);
	static CString GetText(int usIndex);
	static char* GetCharText(int usIndex);
	static void SetDisplayWnd(HWND hwnd);

	static void SetCustomerFlag(BOOL flag);

// Operations
public:
};

