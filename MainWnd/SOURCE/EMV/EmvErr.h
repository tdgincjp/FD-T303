#ifndef EMVERR_H
#define	EMVERR_H

#pragma warning(disable: 4996)

#ifdef	__cplusplus
extern "C" {
#endif


#define EMVERR_TRANSACTION_TYPE_NOT_SUPPORT		0xF101
#define EMVERR_INVALID_PARAMETER							0xF102
#define EMVERR_MAJOR_CONFIG_SETTING						0xF103
#define EMVERR_USER_CANCEL										0xF104
#define EMVERR_ICC_COMM_ERROR									0xF105
#define EMVERR_CB_NOTSET											0xF106
#define EMVERR_INVALID_RSPDATA								0xF500
#define EMVERR_UNSUFFICIENTDATA								0xF501
#define EMVERR_INVALID_TLV										0xF502
#define EMVERR_STOP_TRANSACTION								0xF503
#define EMVERR_DATA_MISSING										0xF504
#define EMVERR_CARD_BLOCK											0xF505
#define EMVERR_NOSUPPORT_CARDAID							0xF506
#define EMVERR_INVALIDPARAMETER								0xF507
#define EMVERR_GENERATEAC_WRONG								0xF508
#define EMVERR_FRAUDCARD_DATA									0xF509
#define EMVERR_MISS_ADFNAME										0xF50A
#define EMVERR_MISS_APLABEL										0xF50B
#define EMVERR_DATA_EXIST											0xF50C		
#define EMVERR_FILE_NOTFOUND									0xF50D
#define EMVERR_RECORD_NOTFOUND								0xF50E
#define EMVERR_DATAPARENT_NULL								0xF50F	
#define EMVERR_CMD_NOTALLOWED									0xF510
#define EMVERR_APPLICATION_BLOCKED						0xF511

#define EMVERR_NOT_ACCEPTED										0xF600

#ifdef	__cplusplus
}
#endif

#endif