/*
 * DesECB.h
 */
#ifndef _DESECB_H_
#define _DESECB_H_

/* constants definition */
#ifndef DES_ENCRYPT
#define DES_ENCRYPT	1
#define DES_DECRYPT	0
#endif /* DES_ENCRYPT */

/* macros definition */
#define	Des2Ecb(pbKey1, pbKey2, pbData, dwBytes, Encrypt)\
		Des3Ecb(pbKey1, pbKey2, pbKey1, pbData, dwBytes, Encrypt)
#define	XAC_Des2Ecb(pdwKs1, pdwKs2, pbData, dwBytes, Encrypt)\
		XAC_Des3Ecb(pdwKs1, pdwKs2, pdwKs1, pbData, dwBytes, Encrypt)


/* Define _EXPORT */
/*
#if !defined( _WINDOWS )
	#undef _EXPORT
	#define _EXPORT
#elif !defined( _EXPORT)
	#define _EXPORT    __declspec(dllimport)
#endif*/	/* _EXPORT */

#ifndef _EXPORT
#define _EXPORT	__declspec(dllimport)
#endif	/* _EXPORT */


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* functions protocol */
void _EXPORT DesEcb(		/* DES ECB */
	BYTE	pbKey[8], 
	BYTE	*pbData, 
	DWORD	dwBytes, 
	BOOL	Encrypt
	);
void _EXPORT Des3Ecb(		/* Triple DES ECB */
	BYTE	pbKey1[8], 
	BYTE	pbKey2[8], 
	BYTE	pbKey3[8],
	BYTE	*pbData, 
	DWORD	dwBytes, 
	BOOL	Encrypt
	);
void XAC_DesEcb(		/* DES ECB */
	DWORD	pdwKs[32], 
	BYTE	*pbData,
	DWORD	dwBytes,
	BOOL	Encrypt
	);
void XAC_Des3Ecb(		/* Triple DES ECB */
	DWORD	pdwKs1[32], 
	DWORD	pdwKs2[32], 
	DWORD	pdwKs3[32], 
	BYTE	*pbData,
	DWORD	dwBytes,
	BOOL	Encrypt
	);

#ifdef __cplusplus
}
#endif

#endif /* _DESECB_H_ */