#include "stdafx.h"
#include <comutil.h>
#include "RapidConnect.h"
#include "..\\COMM\\SSLCon.h"
#include "..\\defs\\struct.h"
#include "..\\Utils\\string.h"
#import "MSXML3.dll" rename_namespace(_T("MSXML"))
#include "msxml2.h"
#include "..\\data\\DebugFile.h"				//JC Nov 11/15 to add debug logging 

CRapidConnect::CRapidConnect(HWND hwnd)
{
	////////////////////////////////////////////////////////////////////
	//Create DataWire object 
 	pDataWire = new CDataWire(hwnd);

	//Create FirstData Struct
	pFDTranRec = new FD_Rec();

	////////////////////////////Super necessary///////////////////////////////
//	CLSID clsid;
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); 
	hr = m_IXMLDOMDocument.CreateInstance(__uuidof(MSXML::DOMDocument));
	//hr = CLSIDFromProgID(L"MSXML.DOMDocument", &clsid);
	//hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL  , 
 //     IID_IXMLDOMDocument, (void**)&m_IXMLDOMDocument);

	if(FAILED(hr))
    {
		Stop();
	}

	m_hWnd = hwnd;
	m_iFBSeqNum = 1;

	error = new char[1024];

}

CRapidConnect::~CRapidConnect()
{
	delete pDataWire;
	delete pFDTranRec;
	delete error;
}
//-----------------------------------------------------------------------------
//!	\main entry point for RapidConnect
//! This will start a sale transaction or a download transaction 
//! based on the pTranReq provided
//-----------------------------------------------------------------------------
void CRapidConnect::Start(TRAN_TranDataRec *pTranReq, CONF_Data *pConfig, BOOL stopflag)
{
	pTRANDATA = pTranReq;
	m_pConfig = pConfig;
	int	Werr;

	delete pFDTranRec;
	pFDTranRec = new FD_Rec();
	int iTryCounter = 0; // only try 3 times
	int iRevCounter = 0;


	CDebugFile::DebugSizeCheck();							//JC Nov 23/15 Check file size, delete if too big
	if (pTranReq->HostTranCode == TRAN_HOST_INIT)
	{
		pDataWire->GenerateClientRef();
		pFDTranRec->FdStatus = FD_STATUS_DOWNLOAD;
		DoHostDownload(&Werr);							//JC Oct 28/15
	}
	else
	{
		try
		{
tran_start:
			iTryCounter++;
			pDataWire->GenerateClientRef();
			pFDTranRec->FdStatus = FD_STATUS_TRAN_START;
			///////////////////////////////////////////////////////////
			//SERIALIZE XML TRANSACTION
			if (GenerateTransactionPayload())
			{
				pFDTranRec->FdStatus = FD_STATUS_TRAN_PAYLOAD;
			}
			else
			{
				pTRANDATA->TranStatus = ST_DECLINED;
				pFDTranRec->FdStatus = FD_STATUS_PAYLOAD_ERROR;
				goto tran_end;
			}
			

			//////////////////////////////////////////////////////////
			//GENERATE DATAWIRE WRAPPER
			if(pDataWire->GenerateDwWrapper(pFDTranRec))
			{
				pFDTranRec->FdStatus = FD_STATUS_TRAN_READY;
			}
			else
			{
				pTRANDATA->TranStatus = ST_DECLINED;
				pFDTranRec->FdStatus = FD_STATUS_DATAWIRE_WRAP_ERROR;
				goto tran_end;
			}
			
			/////////////////////////////////////////////////////////
			//SEND TRANSACTION REQUEST
			if( SendHttpsRequest(pFDTranRec->DataWire_FullRequest, FALSE))
			{
				/////////////////////////////////////////////////////////
				//PARSE THE RESPONSE
				CString sResponse = CString(pFDTranRec->DataWire_FullResponse);
/*
			CString strTemp = CString(pFDTranRec->DataWire_FullRequest);

			if ( strTemp.GetLength()> 800)
			{
				TRACE(L"\n Start SendHttpsRequest( len = %d ):\n %s\n",strTemp.GetLength(),strTemp.Mid(0,800));
				TRACE(L"\n %s\n",strTemp.Mid(800));
			}
			else
				TRACE(L"\n Start SendHttpsRequest( len = %d ):\n %s\n",strTemp.GetLength(),CString(pFDTranRec->DataWire_FullRequest));



			if ( sResponse.GetLength()> 800)
			{
				TRACE(L"\n Start sResponse( len = %d ):\n %s\n",sResponse.GetLength(),sResponse.Mid(0,800));
				TRACE(L"\n %s\n",sResponse.Mid(800));
			}
			else
				TRACE(L"\n Start sResponse( len = %d ):\n %s\n",sResponse.GetLength(),sResponse);
*/

			CString strTemp = CString(pFDTranRec->DataWire_FullRequest);
			CString strTemp1;															//JC NOv 13/15
			strTemp1.Format(L"RSP>>(L%d):%s", strTemp.GetLength(), strTemp);
			char *wstr = alloc_tchar_to_char(strTemp1.GetBuffer(strTemp1.GetLength()));
			if ( pTRANDATA->bRequestedPIN )     // Jun Dec 11/15
			{
				char *p = strstr(wstr,pTRANDATA->PINBlock);
				if ( p != NULL)
				{
					int len = strlen(pTRANDATA->PINBlock);
					for (int i = 0;i< len;i++)
						*(p+i) = '*';
				}
			}
			CDebugFile::DebugLogHost(wstr);
			strTemp1.ReleaseBuffer();

				if (ParseXmlResponse(sResponse))
				{
					pFDTranRec->FdStatus = FD_STATUS_TRAN_FINISH;
					if(pFDTranRec->iRespCode == 0 && !pTRANDATA->bReversal)
					{
						pTRANDATA->TranStatus = ST_APPROVED;
					}
					else
					{
						pTRANDATA->TranStatus = ST_DECLINED;

						if(pFDTranRec->isRetry && iTryCounter <= 3)
							goto tran_start;

						else if(pTRANDATA->bReversal && iRevCounter <= 3)
						{
							iRevCounter++;
							goto tran_start;
						}

					}
				}
				else
				{
					if (pTRANDATA->bReversal)
					{
						if (iRevCounter <= 3) //1 original tran, + 3 reversals
						{
							iRevCounter++;
							goto tran_start; 
						}
					}
					else
					{						
						pTRANDATA->TranStatus = ST_DECLINED;
						pFDTranRec->FdStatus = FD_STATUS_PARSE_ERROR;
						strcpy(pTRANDATA->HostRespText, "PARSE ERROR");
					}
				}
			}
			else
			{
				pFDTranRec->FdStatus = FD_STATUS_COMM_ERROR;
				pTRANDATA->TranStatus = ST_DECLINED;
				strcpy(pTRANDATA->HostRespText, "COMMUNICATION ERROR");	
			}
		}
		catch (...)
		{
			pFDTranRec->FdStatus = FD_STATUS_COMM_ERROR;
			pTRANDATA->TranStatus = ST_DECLINED;
			strcpy(pTRANDATA->HostRespText, "EXCEPTION OCCURRED");
		}
	}
tran_end:
	Stop();
}



//-----------------------------------------------------------------------------
//!	\brief  entry point for host download transactions 
//-----------------------------------------------------------------------------
void CRapidConnect::DoHostDownload(int *Werr)
{
	int iDLReqNum = 0;
	Werr=0;
	m_iDownloadStage = DL_STAGE_START;
	try
	{
		while(m_iDownloadStage < DL_STAGE_FINISH)
		{
			///////////////////////////////////////////////////////////
			//SERIALIZE XML DOWNLOAD
			if (GenerateDownloadPayload(iDLReqNum))
			{
				pFDTranRec->FdStatus = FD_STATUS_DL_PAYLOAD;
			}
			else
			{
				pFDTranRec->FdStatus = FD_STATUS_PAYLOAD_ERROR;
				goto endtran;
			}
			

			//////////////////////////////////////////////////////////
			//GENERATE DATAWIRE WRAPPER
			if (pDataWire->GenerateDwWrapper(pFDTranRec))
			{
				pFDTranRec->FdStatus = FD_STATUS_DL_READY;

			}
			else
			{
				pFDTranRec->FdStatus = FD_STATUS_DATAWIRE_WRAP_ERROR;
				goto endtran;

			}
			
			/////////////////////////////////////////////////////////
			//SEND DOWNLOAD REQUEST
			if (SendHttpsRequest(pFDTranRec->DataWire_FullRequest, TRUE))
			{
				/////////////////////////////////////////////////////////
				//PARSE THE RESPONSE
				CString sResponse = CString(pFDTranRec->DataWire_FullResponse);
				if (ParseDownloadResponse(sResponse))
				{
					pFDTranRec->FdStatus = FD_STATUS_TRAN_SUCCESS;
					if (pFDTranRec->iRespCode == 0)
					{
						pFDTranRec->FdStatus = FD_STATUS_TRAN_FINISH;
						m_iDownloadStage = DL_STAGE_FINISH;
						SetInitStatus((int)STS_DLL_OK);
						pTRANDATA->TranStatus = ST_APPROVED;
					}
					else
					{
						pFDTranRec->FdStatus = FD_STATUS_TRAN_FINISH;
						m_iDownloadStage = DL_STAGE_ERROR;
					}
				}
				else
				{
					pFDTranRec->FdStatus = FD_STATUS_PARSE_ERROR;
					m_iDownloadStage = DL_STAGE_ERROR;
				}
			}
			else
			{
					pFDTranRec->FdStatus = FD_STATUS_COMM_ERROR;
					m_iDownloadStage = DL_STAGE_ERROR;
			}
		}
	}
	catch (...)
	{
		m_iDownloadStage = DL_STAGE_ERROR;
		pFDTranRec->FdStatus = FD_STATUS_DOWNLOAD_ERROR;
		strcpy(pTRANDATA->HostRespText, "EXCEPTION");
	}

endtran:
	if (m_iDownloadStage == DL_STAGE_FINISH)
		pTRANDATA->TranStatus = ST_APPROVED;
	else
		pTRANDATA->TranStatus = ST_DECLINED;

}


//-----------------------------------------------------------------------------
//!	\Set init status
//!	\param	status - enum eDownloadStatus
//-----------------------------------------------------------------------------
void CRapidConnect::SetInitStatus(int status)
{
	CDataFile::Save(FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, sizeof(int));
}


//-----------------------------------------------------------------------------
//!	\brief  sets a trace message and nullifies the HWND object
//-----------------------------------------------------------------------------
void CRapidConnect::Stop()
{
	switch (pFDTranRec->FdStatus)
	{
	case FD_STATUS_TRAN_FINISH:
	case FD_STATUS_TRAN_SUCCESS:
		TRACE(L"FirstData Tran Finished\n");
		if (pTRANDATA->TranStatus == ST_APPROVED)
			TRACE(L"APPROVED\n");
		else
			TRACE(L"DECLINED\n");
		break;
	case FD_STATUS_TRAN_ERROR:
		TRACE(L"FirstData Tran Error\n");
		break;
	case FD_STATUS_COMM_ERROR:
		TRACE(L"FirstData Tran Comms Error\n");
		break;
	case FD_STATUS_PARSE_ERROR:
		TRACE(L"FirstData Tran XML Parsing Error\n");
		break;
	case FD_STATUS_PAYLOAD_ERROR:
		TRACE(L"FirstData Tran Payload Generation Error\n");
		break;
	case FD_STATUS_DATAWIRE_WRAP_ERROR:
		TRACE(L"FirstData Tran DataWire Wrapper Generation Error\n");
		break;
	case FD_STATUS_DOWNLOAD_ERROR:
		TRACE(L"FirstData Download Error\n");
		break;
	case FD_STATUS_RESP_LEN:
		TRACE(L"FirstData BUFFER SIZE TOO SMALL Error\n");
		break;
	default:
		TRACE(L"FirstData Tran exit premature %02d\n", pFDTranRec->FdStatus);
		break;
	}
	
	m_TranStatus = pTRANDATA->TranStatus;

	if(m_hWnd != NULL)
	{
		if(!CDataFile::m_bTraining)
			::PostMessage(m_hWnd, WM_SOCK_RECV, 2,0);
		m_hWnd = NULL;
	}

}





//-----------------------------------------------------------------------------
//!	\ Generates the <CreditRequest> or <DebitRequest> which is saved to 
//! pFDTranRec->RapidConnect_Payload using fields from pTRANDATA
//-----------------------------------------------------------------------------
BOOL CRapidConnect::GenerateTransactionPayload()
{
	MSXML::IXMLDOMDocument2Ptr pXMLDoc;    
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML::DOMDocument));
	if(FAILED(hr))
    {
		return FALSE;
    }

	int CardType  =0;
	if (pTRANDATA->ServType[0] == 'V')
		CardType = CardType_Visa;
	else if (pTRANDATA->ServType[0] == 'M')
		CardType = CardType_MasterCard;
	else if (strcmp(pTRANDATA->ServType, "AX" ) == 0 )
		CardType = CardType_Amex;
	else if (strcmp(pTRANDATA->ServType, "DS" ) == 0 ) 
		CardType = CardType_Discover;
	else if (strcmp(pTRANDATA->ServType, "DC" ) == 0 ) 
		CardType = CardType_Diners;
	else if (strcmp(pTRANDATA->ServType, "JC" ) == 0)
		CardType = CardType_JCB;
	else if (strcmp(pTRANDATA->ServType, "P0" ) == 0)
		CardType = CardType_PropietaryDebit;

	pFDTranRec->isCredit = true;
	if (pTRANDATA->CardType == CARD_DEBIT)
	{
		pFDTranRec->isCredit = false;
	}

	//for later :)
	bool isEMV = false;

////////////////////////////////////////////////////////////// GMF CONTAINER
/// <GMF xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="com/firstdata/Merchant/gmfV3.10">
//////////////////////////////////////////////////////////////
	//MSXML::IXMLDOMElementPtr pXmlGMFContainer = pXMLDoc->createNode(NODE_ELEMENT, _T("GMF"), _T("com/firstdata/Merchant/gmfV3.10"));
	
	if(pXMLDoc->loadXML(_T("<GMF></GMF>")) == VARIANT_FALSE)// xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"com/firstdata/Merchant/gmfV3.10\"
    {
		return FALSE;
    }
	MSXML::IXMLDOMElementPtr pXmlGMFContainer = pXMLDoc->GetdocumentElement();
	pXmlGMFContainer->setAttribute(_T("xmlns:xsi"), _T("http://www.w3.org/2001/XMLSchema-instance")); 
	pXmlGMFContainer->setAttribute(_T("xmlns:xsd"), _T("http://www.w3.org/2001/XMLSchema")); 
	pXmlGMFContainer->setAttribute(_T("xmlns"), _T("com/firstdata/Merchant/gmfV3.10")); 

	/////////////////////////////////////////////////////////////////////////
	/// <CreditRequest> CREDIT REQUEST OR DEBIT REQUEST <DebitRequest> ///////
	/////////////////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlRequestType;
	if (pTRANDATA->bReversal)
		pXmlRequestType = pXMLDoc->createElement(_T("ReversalRequest")); 
	else
	{
		if (pFDTranRec->isCredit)
			pXmlRequestType = pXMLDoc->createElement(_T("CreditRequest")); 
		else
			pXmlRequestType = pXMLDoc->createElement(_T("DebitRequest")); 
	}
	pXmlGMFContainer->appendChild(pXmlRequestType);

	////////////////////////////////////////////////
	/// <CommonGrp> COMMON GROUP
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlCommonGrp = pXMLDoc->createElement(_T("CommonGrp")); 
	pXmlRequestType->appendChild(pXmlCommonGrp);
	
		/// <PymtType>
	MSXML::IXMLDOMElementPtr pXmlPymtType = pXMLDoc->createElement(_T("PymtType"));
	if (pFDTranRec->isCredit)
		pXmlPymtType->Puttext("Credit");
	else
		pXmlPymtType->Puttext("Debit");
	pXmlCommonGrp->appendChild(pXmlPymtType);


	if (pTRANDATA->bReversal)
	{
		if(pTRANDATA->ReversalType == TIMEOUT_REVERSAL)
		{
			MSXML::IXMLDOMElementPtr pXmlReversalInd = pXMLDoc->createElement(_T("ReversalInd"));
			pXmlReversalInd->Puttext("Timeout");
			pXmlCommonGrp->appendChild(pXmlReversalInd);
		}
	}
	/////////////////////////////////////////////////
	// ADD GENERIC COMMON GROUP ELEMENTS (shared with download)
	// TxnType
	// Local Date time
	// Trans Date Time
	// STAN
	// RefNum
	// TPPID
	// Terminal ID
	// Merchant ID
	// POSEntryMode
	// POSCondCode
	// TermCatCode
	// TermEntryCapablt
	// TxnAmt
	// TxnCrncy
	// TermLocInd
	// CardCaptCap
	// GID
	/////////////////////////////////////////////////
	AddSharedCommonGroupElements(pXMLDoc, pXmlCommonGrp);

 ///     </CommonGrp>



	//////////////////////////////////////////////////////////////////////
	//CARD GROUP
    /// <CardGrp>
	//////////////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlCardGrp = pXMLDoc->createElement(_T("CardGrp"));
	pXmlRequestType->appendChild(pXmlCardGrp);
    
	if (!pTRANDATA->bReversal)
	{
		///   <Track2Data>..37</Track2Data>
		//This field contains the unaltered track 2 data from the magnetic stripe of the card
		MSXML::IXMLDOMElementPtr pXmlTrack2Data = pXMLDoc->createElement(_T("Track2Data"));
		pXmlTrack2Data->Puttext(pTRANDATA->Track2);
		pXmlCardGrp->appendChild(pXmlTrack2Data);
	}
	else
	{
		//For reversal provide the Account number and Expiry but do not include the Track2 data
		MSXML::IXMLDOMElementPtr pXmlAcctNum = pXMLDoc->createElement(_T("AcctNum"));
		pXmlAcctNum->Puttext(pTRANDATA->Account);
		pXmlCardGrp->appendChild(pXmlAcctNum);

		MSXML::IXMLDOMElementPtr pXmlCardExpiryDate = pXMLDoc->createElement(_T("CardExpiryDate"));
		CHAR ExpDate[7]; 
		memset(ExpDate, 0, 7);
		strcpy(ExpDate, "20");
		memcpy(&ExpDate[2], pTRANDATA->ExpDate, 4);
		pXmlCardExpiryDate->Puttext(ExpDate);
		pXmlCardGrp->appendChild(pXmlCardExpiryDate);
	}
	
	if (pFDTranRec->isCredit) ///Debit transactions do not contain a <CardType>
	{
		///    <CardType>Visa</CardType>
		// Card type indicates which brand of payment
		MSXML::IXMLDOMElementPtr pXmlCardType = pXMLDoc->createElement(_T("CardType"));
		if (CardType == CardType_Visa)
			pXmlCardType->Puttext("Visa");
		else if (CardType == CardType_MasterCard)
			pXmlCardType->Puttext("MasterCard");
		else if (CardType == CardType_Amex)
			pXmlCardType->Puttext("Amex");
		else if (CardType == CardType_Discover)
			pXmlCardType->Puttext("Discover");
		else if (CardType == CardType_JCB)
			pXmlCardType->Puttext("JCB");
		pXmlCardGrp->appendChild(pXmlCardType);
	}
    /// </CardGrp>



	if ((!pFDTranRec->isCredit || isEMV ) && !pTRANDATA->bReversal)
	{
		//////////////////////////////////////////////////////////////////////
		//PIN GROUP
		/// <PINGrp>
		//////////////////////////////////////////////////////////////////////
		MSXML::IXMLDOMElementPtr pXmlPINGrp = pXMLDoc->createElement(_T("PINGrp"));
		pXmlRequestType->appendChild(pXmlPINGrp);

		
		///   <PINData>PINBLOCKHEX</PINData>
		// 
		MSXML::IXMLDOMElementPtr pXmlPINData = pXMLDoc->createElement(_T("PINData"));
		pXmlPINData->Puttext(pTRANDATA->PINBlock);
		pXmlPINGrp->appendChild(pXmlPINData);

		
		///   <KeySerialNumData>PINKEYHEX</KeySerialNumData>
		// 
		MSXML::IXMLDOMElementPtr pXmlKeySerialNumData = pXMLDoc->createElement(_T("KeySerialNumData"));
		pXmlKeySerialNumData->Puttext(pTRANDATA->PINKSN);
		pXmlPINGrp->appendChild(pXmlKeySerialNumData);

	}


	//////////////////////////////////////////////////////////////////////
	//ADDITIONAL AMOUNTS GROUP	 
    ///<AddtlAmtGrp>
	//////////////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlAddtlAmtGrp = pXMLDoc->createElement(_T("AddtlAmtGrp"));
	pXmlRequestType->appendChild(pXmlAddtlAmtGrp);


	///   <PartAuthrztnApprvlCapablt>1</PartAuthrztnApprvlCapablt>
	//For Credit Authorization and Sale transactions the value must be 1.
	// but we dont support this so sending 0
	MSXML::IXMLDOMElementPtr pXmlPartAuthrztnApprvlCapablt = pXMLDoc->createElement(_T("PartAuthrztnApprvlCapablt"));
	pXmlPartAuthrztnApprvlCapablt->Puttext("0");
	pXmlAddtlAmtGrp->appendChild(pXmlPartAuthrztnApprvlCapablt);


	///</AddtlAmtGrp>

	if (CardType == CardType_Visa)
	{
		////////////////////////////////////////////////
		/// <VisaGrp> VISA GROUP
		////////////////////////////////////////////////
		MSXML::IXMLDOMElementPtr pXmlVisaGrp = pXMLDoc->createElement(_T("VisaGrp")); 
		pXmlRequestType->appendChild(pXmlVisaGrp);

		///      <ACI>Y</ACI>
		//Mandatory for Visa Credit Authorization and Sale transactions.
		MSXML::IXMLDOMElementPtr pXmlACI = pXMLDoc->createElement(_T("ACI"));
		pXmlACI->Puttext("Y");
		pXmlVisaGrp->appendChild(pXmlACI);


		///      <VisaBID>56412</VisaBID>
		//This field contains the Business Identifier (BID) provided by Visa to Third Party Servicers (TPS).
		MSXML::IXMLDOMElementPtr pXmlVisaBID = pXMLDoc->createElement(_T("VisaBID"));
		pXmlVisaBID->Puttext(" ");
		pXmlVisaGrp->appendChild(pXmlVisaBID);

		///      <VisaAUAR>000000000000</VisaAUAR>
		//This field contains Agent Unique Account Result (AUAR) provided by Visa to Third Party Servicers (TPS).
		MSXML::IXMLDOMElementPtr pXmlVisaAUAR = pXMLDoc->createElement(_T("VisaAUAR"));
		pXmlVisaAUAR->Puttext("000000000000");
		pXmlVisaGrp->appendChild(pXmlVisaAUAR);

		///      <TaxAmtCapablt>0</TaxAmtCapablt>
		//0 ?Terminal is Not Tax Prompt Capable
		MSXML::IXMLDOMElementPtr pXmlTaxAmtCapablt = pXMLDoc->createElement(_T("TaxAmtCapablt"));
		pXmlTaxAmtCapablt->Puttext("0");
		pXmlVisaGrp->appendChild(pXmlTaxAmtCapablt);

	   ///   </VisaGrp>
	}

	if (CardType == CardType_MasterCard)
	{
		////////////////////////////////////////////////
		/// <MCGrp> MASTERCARD GROUP
		////////////////////////////////////////////////
		if (pTRANDATA->EntryMode == ENTRY_MODE_CTLS_MSR)
		{
			// We need the Mastercard Group on a Request only when we need to add DevTypeInd on a contactless transaction :)
			MSXML::IXMLDOMElementPtr pXmlMCGrp = pXMLDoc->createElement(_T("MCGrp")); 
			pXmlRequestType->appendChild(pXmlMCGrp);

			///      <DevTypeInd>0</DevTypeInd>
			//0 ?Card
			MSXML::IXMLDOMElementPtr pXmlDevTypeInd = pXMLDoc->createElement(_T("DevTypeInd"));
			pXmlDevTypeInd->Puttext("0");
			pXmlMCGrp->appendChild(pXmlDevTypeInd);
		}
	}


	if (pTRANDATA->bReversal)
	{
		// We need the OrigAuthGroup on any secondary transaction (Reversals, voids, completions)
		MSXML::IXMLDOMElementPtr pXmlOrigAuthGrp = pXMLDoc->createElement(_T("OrigAuthGrp")); 
		pXmlRequestType->appendChild(pXmlOrigAuthGrp);

		///      <OrigLocalDateTime>0</OrigLocalDateTime>
		MSXML::IXMLDOMElementPtr pXmlOrigLocalDateTime = pXMLDoc->createElement(_T("OrigLocalDateTime"));
		pXmlOrigLocalDateTime->Puttext(pFDTranRec->OrigLocalDateTime);
		pXmlOrigAuthGrp->appendChild(pXmlOrigLocalDateTime);
		
		
		///      <OrigTranDateTime>0</OrigTranDateTime>
		MSXML::IXMLDOMElementPtr pXmlOrigTranDateTime = pXMLDoc->createElement(_T("OrigTranDateTime"));
		pXmlOrigTranDateTime->Puttext(pFDTranRec->OrigAuthDateTime);
		pXmlOrigAuthGrp->appendChild(pXmlOrigTranDateTime);

		
		///      <OrigSTAN>0</OrigSTAN>
		MSXML::IXMLDOMElementPtr pXmlOrigSTAN = pXMLDoc->createElement(_T("OrigSTAN"));
		pXmlOrigSTAN->Puttext(pFDTranRec->OrigAuthSTAN);
		pXmlOrigAuthGrp->appendChild(pXmlOrigSTAN);


	}



	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	//! SAVE XML, READ XML
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	CString sLocation = _T("\\SAIO\\User Programs\\INIT\\CreditRequest.xml");
	CStringA fileloc(sLocation);
	DeleteFile(sLocation);
	try
	{
		hr = pXMLDoc->save(sLocation.AllocSysString());
	}
	catch(_com_error errorObject)
    {
        printf("Exception thrown, HRESULT: 0x%08x", errorObject.Error());
		return FALSE;
    }
    catch(...)
    {
        printf("Unknown error");
		return FALSE;
    } 

	FILE* f = fopen((const char *)fileloc, "r");

	// Determine file size
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);

	char tmpBuf[SZ_RAPIDCONNECT_MAX_PAYLOAD+1];
	memset(tmpBuf, '\x0', SZ_RAPIDCONNECT_MAX_PAYLOAD+1);
	if (fsize < SZ_RAPIDCONNECT_MAX_PAYLOAD)
	{
		fseek(f, 0, SEEK_SET);
		fread(tmpBuf, sizeof(char), fsize, f);
	}
	else
	{
		pFDTranRec->FdStatus = FD_STATUS_RESP_LEN;
		return FALSE;
	}
	fclose(f);
	DeleteFile(sLocation);

	strcpy(pFDTranRec->RapidConnect_Payload, tmpBuf);

	return TRUE;
}



//-----------------------------------------------------------------------------
//!	\ Generates an XML char * which is saved to pFDTranRec->RapidConnect_Payload
//! using fields from pTRANDATA
//-----------------------------------------------------------------------------
BOOL CRapidConnect::GenerateDownloadPayload(int iDLReqNum)
{
	MSXML::IXMLDOMDocument2Ptr pXMLDoc;    
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML::DOMDocument));
	if(FAILED(hr))
    {
        error = "Failed to create the XML DOM instance";
    }

	if (m_iDownloadStage == DL_STAGE_START)
		m_iDownloadStage = DL_STAGE_TABLE;

////////////////////////////////////////////////////////////// GMF CONTAINER
/// <GMF xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="com/firstdata/Merchant/gmfV3.10">
//////////////////////////////////////////////////////////////
	//MSXML::IXMLDOMElementPtr pXmlGMFContainer = pXMLDoc->createNode(NODE_ELEMENT, _T("GMF"), _T("com/firstdata/Merchant/gmfV3.10"));
	
	if(pXMLDoc->loadXML(_T("<GMF></GMF>")) == VARIANT_FALSE)// xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"com/firstdata/Merchant/gmfV3.10\"
    {
		error = "Failed to Load the XML";
    }
	MSXML::IXMLDOMElementPtr pXmlGMFContainer = pXMLDoc->GetdocumentElement();
	//pXmlGMFContainer->setAttribute(_T("xmlns:xsi"), _T("http://www.w3.org/2001/XMLSchema-instance")); 
	//pXmlGMFContainer->setAttribute(_T("xmlns:xsd"), _T("http://www.w3.org/2001/XMLSchema")); 
	pXmlGMFContainer->setAttribute(_T("xmlns"), _T("com/firstdata/Merchant/gmfV3.10")); 

	/////////////////////////////////////////////////////////////////////////
	/// <AdminRequest> 
	/////////////////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlRequestType;
	pXmlRequestType = pXMLDoc->createElement(_T("AdminRequest")); 
	pXmlGMFContainer->appendChild(pXmlRequestType);

	////////////////////////////////////////////////
	/// <CommonGrp> COMMON GROUP
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlCommonGrp = pXMLDoc->createElement(_T("CommonGrp")); 
	pXmlRequestType->appendChild(pXmlCommonGrp);
	
	/////////////////////////////////////////////////
	// ADD GENERIC COMMON GROUP ELEMENTS (shared with credit/debit requests)
	// TxnType
	// Local Date time
	// Trans Date Time
	// STAN
	// TPPID
	// Terminal ID
	// Merchan ID
	// GID
	/////////////////////////////////////////////////
	AddSharedCommonGroupElements(pXMLDoc, pXmlCommonGrp);

 ///     </CommonGrp>



	//////////////////////////////////////////////////////////////////////
	//FILE DOWNLOAD GROUP
    /// <FileDLGrp>
	//////////////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlFileDLGrp = pXMLDoc->createElement(_T("FileDLGrp"));
	pXmlRequestType->appendChild(pXmlFileDLGrp);
    

	///   <FileType>TABLE</FileType>
	//This field indicates the type of the file to be downloaded.
	//TABLE ?Merchant/Terminal Parameter Table
	//DYNCRDTBL ?Dynamic Card Table Data
	MSXML::IXMLDOMElementPtr pXmlFileType = pXMLDoc->createElement(_T("FileType"));
	if (m_iDownloadStage == DL_STAGE_TABLE)
		pXmlFileType->Puttext("TABLE");
	else if (m_iDownloadStage == DL_STAGE_BIN)
		pXmlFileType->Puttext("DYNCRDTBL");		
	else
		m_iDownloadStage = DL_STAGE_ERROR;
	pXmlFileDLGrp->appendChild(pXmlFileType);

	if (m_iDownloadStage == DL_STAGE_BIN)
	{
		///   <SubFileType>TABLE</SubFileType>
		//This field indicates the specific type of file to be downloaded when the File Type is DYNCRDTBL
		MSXML::IXMLDOMElementPtr pXmlSubFileType = pXMLDoc->createElement(_T("SubFileType"));
		pXmlSubFileType->Puttext("BIN");
		pXmlFileDLGrp->appendChild(pXmlSubFileType);

		///   <LdSeqNum>1234</LdSeqNum>
		//This field contains the merchant generated unique sequence number assigned to the download.
		MSXML::IXMLDOMElementPtr pXmlLdSeqNum = pXMLDoc->createElement(_T("LdSeqNum"));
		char LdSeqNum [5];
		sprintf((char *)LdSeqNum, "%04d", iDLReqNum);
		pXmlLdSeqNum->Puttext(LdSeqNum);
		pXmlFileDLGrp->appendChild(pXmlLdSeqNum);
		
		///   <DMTblVer>00000</DMTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDMTblVer = pXMLDoc->createElement(_T("DMTblVer"));
		pXmlDMTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDMTblVer);
		
		///   <DBINTblVer>00000</DBINTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDBINTblVer = pXMLDoc->createElement(_T("DBINTblVer"));
		pXmlDBINTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDBINTblVer);

		///   <DRuTblVer>00000</DRuTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDRuTblVer = pXMLDoc->createElement(_T("DRuTblVer"));
		pXmlDRuTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDRuTblVer);
		
		///   <DReTblVer>00000</DReTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDReTblVer = pXMLDoc->createElement(_T("DReTblVer"));
		pXmlDReTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDReTblVer);

		///   <DSAFTblVer>00000</DSAFTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDSAFTblVer = pXMLDoc->createElement(_T("DSAFTblVer"));
		pXmlDSAFTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDSAFTblVer);
		
		///   <DPrTblVer>00000</DPrTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDPrTblVer = pXMLDoc->createElement(_T("DPrTblVer"));
		pXmlDPrTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDPrTblVer);
		
		///   <DPdTblVer>00000</DPdTblVer>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlDPdTblVer = pXMLDoc->createElement(_T("DPdTblVer"));
		pXmlDPdTblVer->Puttext("00001");
		pXmlFileDLGrp->appendChild(pXmlDPdTblVer);
	

		if (m_iFBSeqNum != 0)
		{
			//This field must only be submitted in a FileDownload request when the File Type is DYNCRDTBL
			//and File Block SeqNum is not 0000
			//This field indicates the current version of the BIN table to be downloaded. 

			///   <HMTblVer>????</HMTblVer>
			//This field indicates the current version of the BIN table to be downloaded.
			MSXML::IXMLDOMElementPtr pXmlHMTblVer = pXMLDoc->createElement(_T("HMTblVer"));
			pXmlHMTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHMTblVer);
			
			///   <HBINTblVer>00000</HBINTblVer>
			//This field indicates the current version of the BIN table to be downloaded. 
			MSXML::IXMLDOMElementPtr pXmlHBINTblVer = pXMLDoc->createElement(_T("HBINTblVer"));
			pXmlHBINTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHBINTblVer);

			///   <HRuTblVer>00000</HRuTblVer>
			//This field indicates the current version of the BIN table to be downloaded. 
			MSXML::IXMLDOMElementPtr pXmlHRuTblVer = pXMLDoc->createElement(_T("HRuTblVer"));
			pXmlHRuTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHRuTblVer);
			
			///   <HReTblVer>00000</HReTblVer>
			//This field indicates the current version of the BIN table to be downloaded. 
			MSXML::IXMLDOMElementPtr pXmlHReTblVer = pXMLDoc->createElement(_T("HReTblVer"));
			pXmlHReTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHReTblVer);

			///   <HSAFTblVer>00000</HSAFTblVer>
			//This field indicates the current version of the BIN table to be downloaded. 
			MSXML::IXMLDOMElementPtr pXmlHSAFTblVer = pXMLDoc->createElement(_T("HSAFTblVer"));
			pXmlHSAFTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHSAFTblVer);
			
			///   <HPrTblVer>00000</HPrTblVer>
			//This field indicates the current version of the BIN table to be downloaded. 
			MSXML::IXMLDOMElementPtr pXmlHPrTblVer = pXMLDoc->createElement(_T("HPrTblVer"));
			pXmlHPrTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHPrTblVer);
			
			///   <HPdTblVer>00000</HPdTblVer>
			//This field indicates the current version of the BIN table to be downloaded. 
			MSXML::IXMLDOMElementPtr pXmlHPdTblVer = pXMLDoc->createElement(_T("HPdTblVer"));
			pXmlHPdTblVer->Puttext("00001");
			pXmlFileDLGrp->appendChild(pXmlHPdTblVer);
		}
	}
	
	///   <FunCode>D</FunCode>
	//In a FileDownload request it indicates the requested action.
	//R ?Request Status
	//D ?Download
	//F ?Final Block in a SITECFG Load
	MSXML::IXMLDOMElementPtr pXmlFunCode = pXMLDoc->createElement(_T("FunCode"));
	pXmlFunCode->Puttext("D");
	pXmlFileDLGrp->appendChild(pXmlFunCode);

	if (m_iDownloadStage == DL_STAGE_BIN)
	{
		///   <CurrFileCreationDt>MMDDYYYYhhmmss</CurrFileCreationDt>
		//This field indicates the current version of the BIN table to be downloaded. 
		MSXML::IXMLDOMElementPtr pXmlCurrFileCreationDt = pXMLDoc->createElement(_T("CurrFileCreationDt"));
		char dtTemp [15];
		strcpy (dtTemp, "20");
		strcat (dtTemp, pTRANDATA->TranDate);
		strcat (dtTemp, pTRANDATA->TranTime);
		pXmlCurrFileCreationDt->Puttext("04012015101010");
		pXmlFileDLGrp->appendChild(pXmlCurrFileCreationDt);
		

		///   <FBSeq>0001</FBSeq>
		//This field is used by the merchant to keep track of the current downloading block. In a request message it
		//is used to indicate the desired block. In a response message it indicates the block that is being returned
		//In the first of the series of FileDownload request messages with Function Code D and File Type =
		//HOSTDISC, RECTXT or DYNCRDTBL, the value should be 0000.
		MSXML::IXMLDOMElementPtr pXmlFBSeq = pXMLDoc->createElement(_T("FBSeq"));
		char FbSeqNum [5];
		sprintf((char *)FbSeqNum, "%04d", m_iFBSeqNum);
		pXmlFBSeq->Puttext(FbSeqNum);
		pXmlFileDLGrp->appendChild(pXmlFBSeq); 
	}

	

    /// </FileDLGrp>


	//////////////////////////////////////////////////////////////////////
	//! SAVE XML, READ XML
	//////////////////////////////////////////////////////////////////////
	CDebugFile::DebugWrite("Writing FileDownLoad.xml");							//JC Nov 12/15
	CString sLocation = L"\\SAIO\\User Programs\\INIT\\FileDownloadReq.xml";		//JC Nov 24/15 Strange declaration
	CStringA fileloc(sLocation);
	DeleteFile(sLocation);
	try
	{
		hr = pXMLDoc->save(sLocation.AllocSysString());
	}
	catch(_com_error errorObject)
    {
        printf("Exception thrown, HRESULT: 0x%08x", errorObject.Error());
		char wstr[50];
		sprintf(wstr,"Exception thrown, HRESULT: 0x%08x", errorObject.Error());
		CDebugFile::DebugWrite(wstr);							//JC Nov 12/15
    }
    catch(...)
    {
        printf("Unknown error");
		CDebugFile::DebugWrite("Unknown Error");
    } 

	FILE* f = fopen((const char *)fileloc, "r");

	// Determine file size
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);

	char tmpBuf[SZ_RAPIDCONNECT_MAX_PAYLOAD+1];
	memset(tmpBuf, '\x0', SZ_RAPIDCONNECT_MAX_PAYLOAD+1);
	if (fsize < SZ_RAPIDCONNECT_MAX_PAYLOAD)
	{
		fseek(f, 0, SEEK_SET);
		fread(tmpBuf, sizeof(char), fsize, f);
	}
	else
	{
		pFDTranRec->FdStatus = FD_STATUS_RESP_LEN;
		return FALSE;
	}
	fclose(f);
	// JC Test Nov 8/15 DeleteFile(sLocation);

	strcpy(pFDTranRec->RapidConnect_Payload, tmpBuf);

	return TRUE;
}




//-----------------------------------------------------------------------------
//!	\ Encapsulates the SSLCon libraries, sends, recives and saves response to 
//!	 pFDTranRec->DataWire_FullResponse
//-----------------------------------------------------------------------------
BOOL CRapidConnect::SendHttpsRequest(const char * request, BOOL DebugLog)			//JC Nov 9/15	
{
	TCHAR buf[100];
	CSslConnection txnHttpsConnection = CSslConnection(m_hWnd);
	string strVerb = "POST";
	CString strRapidConnectAddressKey = L"DataWire Txn URL";
	char host[100] = {"prod.dw.us.fdcnet.biz"};//https://stg.dw.us.fdcnet.biz/rc  //JC Oct 28/15 "stg.dw.us.fdcnet.biz"
	if(CDataFile::Read(strRapidConnectAddressKey,buf))
		strcpy(host,alloc_tchar_to_char(buf));
	char version[SZ_VERSION +1] = "test";
	if(CDataFile::Read(L"VERSION",buf))
	{
		char *p = alloc_tchar_to_char(buf);
		strcpy(version,p);
		free(p);
		version[SZ_VERSION] = 0;
	}
	string sAgentName("LocoMobi303 v");
	sAgentName += version;
	string sServerName = host;
	string sUserName("");//if required
	string sPass(""); //if required
	string sObjectName = "/rc/" ;
	string sOrganizationUnitName("First Data Corporation");
	string strHeaders = "Host: prod.dw.us.fdcnet.biz\r\nContent-Type: text/xml\r\n";   //JC Oct 28/15 "stg.dw.us.fdcnet.biz"

	txnHttpsConnection.SetAgentName(sAgentName);
	txnHttpsConnection.SetCertStoreType(certStoreCA);
	txnHttpsConnection.SetObjectName(sObjectName);	
	txnHttpsConnection.SetOrganizationName(sOrganizationUnitName);
	txnHttpsConnection.SetPort(INTERNET_DEFAULT_HTTPS_PORT);
	txnHttpsConnection.SetServerName(sServerName); 
	txnHttpsConnection.SetHttpHeaders(strHeaders);
	txnHttpsConnection.SetRequestID(0);

		
	int len = strlen(request);
	char* req = new char [len];
	strcpy(req, request);
	if (DebugLog)
	{
		CDebugFile::DebugWrite("CRapidConnect::SendHttpsRequest Request----------->");	//JC Nov 9/15 Test
		CDebugFile::DebugWrite(req);													//JC Nov 9/15 Test
	}

	if (txnHttpsConnection.ConnectToHttpsServer(strVerb)) 
	{
		pTRANDATA->ComStatus = ST_OK;
		if(txnHttpsConnection.SendHttpsRequest(req, len))
		{
/*			CString strTemp = CString(request);

			if ( strTemp.GetLength() > 800)
			{
				TRACE(L"\nCRapidConnect SendHttpsRequest(len = %d ):\n %s\n",len,strTemp.Mid(0,800));
				TRACE(L"%s\n",strTemp.Mid(800));
			}
			else
				TRACE(L"\nCRapidConnect SendHttpsRequest(len = %d ):\n %s\n",len,strTemp);
*/
			const char * response = txnHttpsConnection.GetRequestResult().c_str();
			int len = strlen(response);

/*			strTemp = CString(response);
			if ( strTemp.GetLength() > 800)
			{
				TRACE(L"\nCRapidConnect response(len = %d ):\n %s\n",len,strTemp.Mid(0,800));
				TRACE(L"%s\n",strTemp.Mid(800));
			}
			else
				TRACE(L"\nCRapidConnect response(len = %d ):\n %s\n",len,strTemp);
*/
			if (len < SZ_DATAWIRE_MAX_BUFFER)
			{
				strcpy(pFDTranRec->DataWire_FullResponse, response);
				if (DebugLog)
				{
					CDebugFile::DebugWrite("CRapidConnect::SendHttpsRequest Response----------->");	//JC Nov 9/15 Test
					CDebugFile::DebugWrite(pFDTranRec->DataWire_FullResponse);						//JC Nov 9/15 Test
				}
				delete req;
				return TRUE;
			}
			else
			{
				pFDTranRec->FdStatus = FD_STATUS_RESP_LEN;
				delete req;
				return FALSE;
			}
		}
	}

	delete req;
	return FALSE;

}

//-----------------------------------------------------------------------------
//!	\ Uses MSXML libraries to parse the Download Response
//! \ Saves pertinent data to the pTRANDATA and to the registry
//-----------------------------------------------------------------------------
BOOL CRapidConnect::ParseDownloadResponse(CString response)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED); 

	CT2CA pszConvertedString(response);
	bool approval = false;
	bool reject = false;
	string temp(pszConvertedString);
	string sloctext = "<AdminResponse>";
	size_t sloc = temp.find(sloctext);
	if (sloc > temp.length())
	{
		reject = true;
		pTRANDATA->TranStatus = ST_DECLINED;
			strcpy(pTRANDATA->HostRespText, "DOWNLOAD FAILED");
		sloctext = "<RejectResponse>";
		sloc = temp.find(sloctext);
		if (sloc > temp.length())
		{
			return ParseDatawireResponse(response);
		}
	}
	string trimmed = temp.substr(sloc);
	string eloctext = "</GMF>";
	size_t eloc = trimmed.find(eloctext);
	if (eloc >= trimmed.length())
	{
		int i=0;
	}
	string trimmed2 = trimmed.substr(0, eloc);
	CString sTrimmedResp(trimmed2.c_str());
	_bstr_t bstrResponse(sTrimmedResp);


	IXMLDOMNodeListPtr pIDOMNodeList;
	
	try
	{
		HRESULT hr = m_IXMLDOMDocument->loadXML(bstrResponse);
		if (FAILED(hr))
		{

		}

		if (reject) 
		{
			const char * Reason = GetXmlValue(pIDOMNodeList , "AddtlRespData");
			if (Reason[0] == 0)
			{
				Reason = GetXmlValue(pIDOMNodeList , "ErrorData");
			}
			strcpy(pTRANDATA->HostRespText,Reason);
			return false;
		}

		
		//////////RESPONSE CODE
		/// <RespCode>
		const char * RespCode = GetXmlValue(pIDOMNodeList , "RespCode");
		pFDTranRec->iRespCode = atoi(RespCode);
		if (pFDTranRec->iRespCode == 0)
		{
			approval = true;

			if(m_iDownloadStage == DL_STAGE_TABLE)
			{
				//////////Table Load Merchant Name
				/// <TLMerchName>
				const char * TLMerchName = GetXmlValue(pIDOMNodeList, "TLMerchName");
				char * NAME = new char[strlen(TLMerchName)];
				strcpy(NAME, TLMerchName);
				strcat(NAME, " ");

				//////////Table Load Store Number
				/// <TLStNum>
				const char * TLStNum = GetXmlValue(pIDOMNodeList, "ReTLStNumfNum");
				strcat(NAME, TLStNum);

				//FID_MERCH_CFG_NAME will be NAME + STORE NUM
				CDataFile::Save(FID_MERCH_CFG_NAME, (BYTE *)NAME, strlen(NAME)+1);
				delete NAME;

				//////////Table Load Merchant Address
				/// <TLMerchAddr>
				const char * TLMerchAddr = GetXmlValue(pIDOMNodeList, "TLMerchAddr");
				char * ADDR = new char[strlen(TLMerchAddr)];
				strcpy(ADDR, TLMerchAddr);
				//FID_MERCH_CFG_ADDRESS will be ADDRESS +  CITY
				CDataFile::Save(FID_MERCH_CFG_ADDRESS, (BYTE *)ADDR, strlen(ADDR)+1);
				delete ADDR;

				
				//////////Table Load Merchant City
				/// <TLMerchCity>
				const char * TLMerchCity = GetXmlValue(pIDOMNodeList, "TLMerchCity");
				char * STATE = new char[strlen(TLMerchCity)];
				strcpy(STATE, TLMerchCity);
				strcat(STATE, ", ");

				//////////Table Load Merchant State
				/// <TLMerchState>
				const char * TLMerchState = GetXmlValue(pIDOMNodeList, "TLMerchState");
				strcat(STATE, TLMerchState);


				////////////Table Load Merchant Postal Code
				///// <TLMerchPostalCode>
				//const char * TLMerchPostalCode = GetXmlValue(pIDOMNodeList, "TLMerchPostalCode");
				//strcat(STATE, TLMerchPostalCode);

				//FID_MERCH_CFG_CITY_PROV will be STATE +  POSTAL
				CDataFile::Save(FID_MERCH_CFG_CITY_PROV, (BYTE *)STATE, strlen(STATE)+1);

				delete STATE;
				
				//////////Time Table Current Date and Time (Local)
				/// <TTDateTime> MMDDYYhhmm </TTDateTime>
				const char * TTDateTime = GetXmlValue(pIDOMNodeList, "TTDateTime");
				CTimeUtil time;
				char tempTime[13];
				memset(tempTime, 0, 13);
				memcpy(tempTime, &TTDateTime[4], 2);
				memcpy(&tempTime[2], &TTDateTime[0], 2);
				memcpy(&tempTime[4], &TTDateTime[2], 2);
				memcpy(&tempTime[6], &TTDateTime[6], 2);
				memcpy(&tempTime[8], &TTDateTime[8], 2);
				strcat(tempTime, "00");
				
				//YYMMDDhhmmss
				time.SetDateAndTime(tempTime);
				
			}
			else if (m_iDownloadStage == DL_STAGE_BIN)
			{
				//m_iFBSeqNum

				//m_sHBINTblVer
			}
			else
				m_iDownloadStage = DL_STAGE_ERROR;

		}
	    if(pIDOMNodeList)
			pIDOMNodeList->Release();
	}

	catch(...)
	{
		IXMLDOMParseErrorPtr pe = m_IXMLDOMDocument->parseError;
	    if(pIDOMNodeList)
		   pIDOMNodeList->Release();
		pIDOMNodeList = NULL;
	}

	return approval;
}

//-----------------------------------------------------------------------------
//!	\ Uses MSXML libraries to parse the Transaction XML Response
//! \ Saves pertinent data to the pTRANDATA object to be used by caller
//-----------------------------------------------------------------------------
BOOL CRapidConnect::ParseXmlResponse(CString response)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED); 

	CT2CA pszConvertedString(response);
	bool approval = false;
	bool reject = false;
	string sloctext;
	string temp(pszConvertedString);
	if (pTRANDATA->bReversal)
		sloctext = "<ReversalResponse>";
	else if (pFDTranRec->isCredit)
		sloctext = "<CreditResponse>";
	else
		sloctext = "<DebitResponse>";

	size_t sloc = temp.find(sloctext);
	if (sloc > temp.length())
	{
		reject = true;
		sloctext = "<RejectResponse>";
		sloc = temp.find(sloctext);
		if (sloc > temp.length())
		{
			//Need to figure out 400 or 500
			sloctext = "<h1>Bad Request</h1>"; // Bad request is a 400 error
			sloc = temp.find(sloctext);
			if (sloc > temp.length())
			{
				return ParseDatawireResponse(response);
			}
			else 
			{
				strcpy(pTRANDATA->HostRespText,"Bad Request");
				return false;
			}
		}
	}
	string trimmed = temp.substr(sloc);
	string eloctext = "</GMF>";
	size_t eloc = trimmed.find(eloctext);
	if (eloc >= trimmed.length())
	{
		int i=0;
	}
	string trimmed2 = trimmed.substr(0, eloc);
	CString sTrimmedResp(trimmed2.c_str());
	_bstr_t bstrResponse(sTrimmedResp);


	IXMLDOMNodeListPtr pIDOMNodeList;
	
	try
	{
		HRESULT hr = m_IXMLDOMDocument->loadXML(bstrResponse);
		if (FAILED(hr))
		{

		}

		if (reject) 
		{
			const char * Reason = GetXmlValue(pIDOMNodeList , "AddtlRespData");
			if (Reason[0] == 0)
			{
				Reason = GetXmlValue(pIDOMNodeList , "ErrorData");
			}
			strcpy(pTRANDATA->HostRespText,Reason);
			
			
			const char * RespCode = GetXmlValue(pIDOMNodeList , "RespCode");
			pFDTranRec->iRespCode = atoi(RespCode);
			strcpy(pTRANDATA->RespCode, RespCode);
			return TRUE;
		}

		
		//////////RESPONSE CODE
		/// <RespCode>
		const char *AuthID, *STAN, *RefNum;
		const char *RespCode = GetXmlValue(pIDOMNodeList , "RespCode");
		pFDTranRec->iRespCode = atoi(RespCode);
		strcpy(pTRANDATA->RespCode, RespCode);
		switch (pFDTranRec->iRespCode)
		{
			case 0:
				approval = true;

				//////////AUTH CODE
				/// <AuthID>
				AuthID = GetXmlValue(pIDOMNodeList, "AuthID");
				strcpy(pTRANDATA->AuthCode, AuthID);
				

				//////////REFERENCE NUMBER
				/// <RefNum>
				RefNum = GetXmlValue(pIDOMNodeList, "RefNum");
				sprintf(pTRANDATA->SequenceNum, "%010d", atoi(RefNum));


				//////////STAN
				/// <STAN>
				STAN = GetXmlValue(pIDOMNodeList, "STAN");
				sprintf(pTRANDATA->InvoiceNo, "%07d", atoi(STAN));
				break;
			case 505: //PLEASE RETRY
				pFDTranRec->isRetry = true;
				break;
			case 516: /*Please retry ?Reasons for this	error are one of the following:
						Format Error, Unable to route transaction, Switch or issuer
						unavailable, System Busy, Timeout*/	
				pFDTranRec->isRetry = true;
				break;
			default:
				approval = false;
				break;

		}

	    if(pIDOMNodeList)
			pIDOMNodeList->Release();
		
		return TRUE;
	}

	catch(...)
	{
		IXMLDOMParseErrorPtr pe = m_IXMLDOMDocument->parseError;
	    if(pIDOMNodeList)
		   pIDOMNodeList->Release();
		pIDOMNodeList = NULL;
		return FALSE;
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
//!	\ If there is an error or timeout scenario from Datawire where there is no
//	response in the rapid connect payload, the error must be retrieved from 
//  the Datawire wrapper. This function parses that scenario
//-----------------------------------------------------------------------------
BOOL CRapidConnect::ParseDatawireResponse(CString response)
{
	CDebugFile::DebugWrite("ParseDatawireResponse");		//JC Nov 9/15
	CT2CA pszConvertedString(response);
	string test(pszConvertedString);
	string sloctext = "<Response ";
	size_t sloc = test.find(sloctext);
	string trimmed = test.substr(sloc);
	CString sTrimmedResp(trimmed.c_str());
	_bstr_t bstrResponse(sTrimmedResp);

	wstring strFindText (_T("ReturnCode"));
	IXMLDOMNodeListPtr pIDOMNodeList;
	IXMLDOMNode *pIDOMNode = NULL;
	long value;
	BSTR bstrItemText;

	try
	{
		if(m_IXMLDOMDocument->loadXML(bstrResponse) == VARIANT_FALSE)
		{
			printf("Failed to Load the XML");
			return FALSE;
		}
		pIDOMNodeList = m_IXMLDOMDocument->getElementsByTagName((TCHAR*)strFindText.data());
		HRESULT hr = pIDOMNodeList->get_length(&value);
		if(SUCCEEDED(hr))
		{
			if(value > 0)
			{
			//RETURN CODE PROCESSING FROM DATAWIRE
			pIDOMNodeList->reset();
			for(int ii = 0; ii < value; ii++)
			{
				pIDOMNodeList->get_item(ii, &pIDOMNode);
				if(pIDOMNode)
				{
					pIDOMNode->get_text(&bstrItemText);
					pIDOMNode->Release();
					pIDOMNode = NULL;
					char * cRetCode = _com_util::ConvertBSTRToString(bstrItemText);
					pFDTranRec->iRespCode = atoi(const_cast<char*>(cRetCode));
					pTRANDATA->TranCode = pFDTranRec->iRespCode;
					strcpy(pTRANDATA->RespCode, cRetCode);
					switch(pFDTranRec->iRespCode)
					{
						case DW_INVALID_SESSION:		//006
							pFDTranRec->isRetry = false;
							pTRANDATA->TranStatus = ST_HOST_ERROR;
							break;
						case DW_NETWORK_ERROR8:			//008
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						case DW_HOST_BUSY:				//200
							pFDTranRec->isRetry = true;
							break;
						case DW_HOST_UNAVAILABLE:		//201
							pFDTranRec->isRetry = true;
							break;
						case DW_HOST_CONNECT_ERROR:		//202
							pFDTranRec->isRetry = true;
							break;
						case DW_HOST_DROP:				//203		
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						case DW_HOST_COMM_ERROR:		//204
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						case DW_NO_RESPONSE:			//205
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						case DW_HOST_SEND_ERROR:		//206
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						case DW_TRANSPORT_TIMEOUT:		//405
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						case DW_NETWORK_ERROR:			//505
							pTRANDATA->TranStatus = ST_TIMEOUT;		
							pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
							pTRANDATA->bReversal = TRUE;
							strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							break;
						default:					//ANYTHING ELSE
							pTRANDATA->TranStatus = ST_HOST_ERROR;
							break;
					}

					return TRUE;
				}
			}
		}
		else 
		{
			//STATUS PROCESSING FROM DATAWIRE
			strFindText = _T("Status");
			pIDOMNodeList = m_IXMLDOMDocument->getElementsByTagName((TCHAR*)strFindText.data());
			HRESULT hr = pIDOMNodeList->get_length(&value);
			CDebugFile::DebugWrite("In Status Processing");							//JC Nov 9/15
			if(value > 0)
			{
				if (SUCCEEDED(hr))
				{
					//RETURN CODE PROCESSING FROM DATAWIRE
					pIDOMNodeList->reset();
					for(int ii = 0; ii < value; ii++)
					{
						pIDOMNodeList->get_item(ii, &pIDOMNode);
						if(pIDOMNode)
						{
							wstring strFindAttr (_T("StatusCode"));
							IXMLDOMNamedNodeMap *iAttributes = NULL;
							pIDOMNode->get_attributes(&iAttributes);
							iAttributes->getNamedItem((TCHAR*)strFindAttr.data(), &pIDOMNode);
							pIDOMNode->get_text(&bstrItemText);
							pIDOMNode->Release();
							pIDOMNode = NULL;
							const char * cStatusCode = _com_util::ConvertBSTRToString(bstrItemText);
							pTRANDATA->TranCode = 999;
							pFDTranRec->iRespCode = 999;
							strcpy(pTRANDATA->RespCode, "999");

							if (str_compare(cStatusCode, "OK"))
							{
								//We should be in the section above getting the ReturnCode and evaluating that switch.
							}
							else if (str_compare(cStatusCode, "AuthenticationError"))
							{
								pTRANDATA->TranStatus = ST_HOST_ERROR;	
								strcpy(pTRANDATA->HostRespText, "Authentication Error");
							}
							else if (str_compare(cStatusCode, "UnknownServiceID"))
							{
								pTRANDATA->TranStatus = ST_HOST_ERROR;	
								strcpy(pTRANDATA->HostRespText, "UnknownServiceID");
							}
							else if (str_compare(cStatusCode, "XMLError"))
							{
								pTRANDATA->TranStatus = ST_HOST_ERROR;	
								strcpy(pTRANDATA->HostRespText, "XMLError");
							}
							else if (str_compare(cStatusCode, "OtherError"))
							{
								pTRANDATA->TranStatus = ST_HOST_ERROR;	
								strcpy(pTRANDATA->HostRespText, "OtherError");
								pFDTranRec->isRetry = true;
							}
							else if (str_compare(cStatusCode, "InvalidSessionContext"))
							{
								pTRANDATA->TranStatus = ST_HOST_ERROR;	
								strcpy(pTRANDATA->HostRespText, "InvalidSessionContext");
								// WE DO NOT USE SESSION
							}
							else if (str_compare(cStatusCode, "Timeout"))
							{
								pTRANDATA->TranStatus = ST_TIMEOUT;		
								pTRANDATA->ReversalType = TIMEOUT_REVERSAL;
								pTRANDATA->bReversal = TRUE;
								strcpy(pTRANDATA->HostRespText, "HOST TIMEOUT");
							}
							else
							{
								pTRANDATA->TranStatus = ST_HOST_ERROR;	
								strcpy(pTRANDATA->HostRespText, "Unknown Error");
							}
							CDebugFile::DebugWrite(pTRANDATA->HostRespText);					//JC Nov 9/15

						}
					}
				}
			}
		}
		
			
		}
		else
		{
			printf("Failed to Read the XML");
			return FALSE;
		}
		
		pIDOMNodeList->Release();
		pIDOMNodeList = NULL;
	}

	catch(...)
	{
		if(pIDOMNodeList)
			pIDOMNodeList->Release();
		if(pIDOMNode)
			pIDOMNode->Release();
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
//!	\ where 'value' represents the string key value of an element within pIDOMNodeList
//		ie.	<test1>value1</test1><test2>return me</test2> 
//		where value = "test2", returns "return me"
//		else return "" (empty string)
//-----------------------------------------------------------------------------
const char * CRapidConnect::GetXmlValue(IXMLDOMNodeListPtr pIDOMNodeList, string value)
{
	IXMLDOMNode *pIDOMNode = NULL;
	wstring strFindText(value.begin(), value.end());
	BSTR bstrItemText;
	long length;
	try
	{
		pIDOMNodeList = m_IXMLDOMDocument->getElementsByTagName((TCHAR*)strFindText.data());
		HRESULT hr = pIDOMNodeList->get_length(&length);
		if(SUCCEEDED(hr))
		{
		  pIDOMNodeList->reset();
		  for(int ii = 0; ii < length; ii++)
		  {
			 pIDOMNodeList->get_item(ii, &pIDOMNode);
			 if(pIDOMNode )
			 {
				pIDOMNode->get_text(&bstrItemText);
				
				_bstr_t interim(bstrItemText, false);   
				const char* strValue((const char*) interim);
				pIDOMNode->Release();
				return strValue;

			}
		  }
		}
	}
	catch (...)
	{

	}
	if (pIDOMNode)
		pIDOMNode->Release();
	return "";
}


//-----------------------------------------------------------------------------
//!	\ This function is called as part of GenerateTransactionPayload and 
//		Generate Download Payload, it is used to group common elements.
//-----------------------------------------------------------------------------
void CRapidConnect::AddSharedCommonGroupElements(MSXML::IXMLDOMDocument2Ptr pXMLDoc, MSXML::IXMLDOMElementPtr pXmlCommonGrp)
{
	BOOL bIsDownload = false;
	/// <TxnType>
	MSXML::IXMLDOMElementPtr pXmlTxnType = pXMLDoc->createElement(_T("TxnType"));
	if (pTRANDATA->TranCode == TRAN_PURCHASE)
	{
		pXmlTxnType->Puttext("Sale");
	}
	else if (pTRANDATA->TranCode == TRAN_HOST_INIT)
	{
		pXmlTxnType->Puttext("FileDownload");
		bIsDownload = true;
	}
	else 
	{
		pXmlTxnType->Puttext("?????ERROR");
	}
	pXmlCommonGrp->appendChild(pXmlTxnType);

		/// <LocalDateTime>
	MSXML::IXMLDOMElementPtr pXmlLocalDateTime = pXMLDoc->createElement(_T("LocalDateTime"));
		/// <TrnmsnDateTime>
	MSXML::IXMLDOMElementPtr pXmlTrnmsnDateTime = pXMLDoc->createElement(_T("TrnmsnDateTime"));
	char dtLocal [15];
	char dtGMT [15];
	CTimeUtil time;
	time.GetSystemDateTimeLong(dtLocal);
	pXmlLocalDateTime->Puttext(dtLocal);

	if(ApplyGMTDateOffset(dtLocal, dtGMT))
		pXmlTrnmsnDateTime->Puttext(dtGMT);
	else
		pXmlTrnmsnDateTime->Puttext(dtLocal);

	if (!pTRANDATA->bReversal)
	{
		strcpy(pFDTranRec->OrigAuthDateTime, dtGMT);
		strcpy(pFDTranRec->OrigLocalDateTime, dtLocal);
	}
	pXmlCommonGrp->appendChild(pXmlLocalDateTime);
	pXmlCommonGrp->appendChild(pXmlTrnmsnDateTime);


	/// <STAN> Use Datawire.ClientRef (pad to 6 digits)
	MSXML::IXMLDOMElementPtr pXmlSTAN = pXMLDoc->createElement(_T("STAN"));
	char * STAN = new char[SZ_DATAWIRE_CLIENT_REF];
	strcpy(STAN, &pDataWire->sRefNum[1]); 
	pXmlSTAN->Puttext(STAN);
	pXmlCommonGrp->appendChild(pXmlSTAN);

	//Save the STAN, we will need it for a reversal, do not overwrite if it is a reversal.
	if (!pTRANDATA->bReversal)
		strcpy(pFDTranRec->OrigAuthSTAN, STAN);

	delete STAN;

	if (!bIsDownload)
	{
		/// <RefNum> 
		MSXML::IXMLDOMElementPtr pXmlRefNum = pXMLDoc->createElement(_T("RefNum"));
		char * REFNUM;
		if (pTRANDATA->bReversal)
			REFNUM = pFDTranRec->ReferenceNum; 
		else
		{
			REFNUM = pDataWire->PadLeft(pDataWire->sRefNum, 10, '0'); 
			strcpy(pFDTranRec->ReferenceNum, REFNUM);
		}
		pXmlRefNum->Puttext(REFNUM);
		pXmlCommonGrp->appendChild(pXmlRefNum);
	}

	/// <TPPID>
	//Software version specific, will be assigned
	MSXML::IXMLDOMElementPtr pXmlTPPID = pXMLDoc->createElement(_T("TPPID"));
	TCHAR buf[10] = _T("");
	if(CDataFile::Read(L"Datawire Project ID", buf))
	pXmlTPPID->Puttext(buf); 
	pXmlCommonGrp->appendChild(pXmlTPPID);

	/// <TermID>
	// Terminal ID saved in Datawire TID
	MSXML::IXMLDOMElementPtr pXmlTermID = pXMLDoc->createElement(_T("TermID"));
	CDataFile::Read(L"Datawire TID", m_pConfig->TermID);
	pXmlTermID->Puttext(m_pConfig->TermID);
	pXmlCommonGrp->appendChild(pXmlTermID);

	/// <MerchID>
	//	Merchant ID, saved in DataWire MID
	MSXML::IXMLDOMElementPtr pXmlMerchID = pXMLDoc->createElement(_T("MerchID"));
	pXmlMerchID->Puttext(m_pConfig->MerchID);
	pXmlCommonGrp->appendChild(pXmlMerchID);


	if(!bIsDownload)
	{
		
		///        <MerchCatCode>5399</MerchCatCode>
		//The merchant category code (MCC) is an industry standard four-digit number used to
		//classify a business by the type of goods or services it provides.
		MSXML::IXMLDOMElementPtr pXmlMerchCatCode = pXMLDoc->createElement(_T("MerchCatCode"));
		pXmlMerchCatCode->Puttext("5399");
		pXmlCommonGrp->appendChild(pXmlMerchCatCode);


		/// <POSEntryMode>
		//900 no pin //901 pin //910 contactless mag //051 ICC + PIN
		MSXML::IXMLDOMElementPtr pXmlPOSEntryMode = pXMLDoc->createElement(_T("POSEntryMode"));
		if (pTRANDATA->EntryMode == ENTRY_MODE_SWIPED)
			pXmlPOSEntryMode->Puttext("901");			
		else if(pTRANDATA->EntryMode == ENTRY_MODE_CTLS_MSR)
			pXmlPOSEntryMode->Puttext("911");
		pXmlCommonGrp->appendChild(pXmlPOSEntryMode);


		///        <POSCondCode>02</POSCondCode>
		//00 ?Cardholder Present, Card Present
		//02 ?Cardholder Present, Unattended Device
		MSXML::IXMLDOMElementPtr pXmlPOSCondCode = pXMLDoc->createElement(_T("POSCondCode"));
		pXmlPOSCondCode->Puttext("00");
		pXmlCommonGrp->appendChild(pXmlPOSCondCode);



		///        <TermCatCode>01</TermCatCode>
		//06 ?Unattended Customer Terminal
		MSXML::IXMLDOMElementPtr pXmlTermCatCode = pXMLDoc->createElement(_T("TermCatCode"));
		//pXmlTermCatCode->Puttext("01");
		pXmlTermCatCode->Puttext("06");
		pXmlCommonGrp->appendChild(pXmlTermCatCode);


		///        <TermEntryCapablt>01</TermEntryCapablt>
		//00 ?Unspecified
		//09 ?Chip and magnetic stripe
		/*	We  may need to use unspecificed. There is no value available (yet)
			for Mag, Chip, and Contactless	*/
		MSXML::IXMLDOMElementPtr pXmlTermEntryCapablt = pXMLDoc->createElement(_T("TermEntryCapablt"));
		if(pTRANDATA->EntryMode == ENTRY_MODE_CTLS_MSR)
			pXmlTermEntryCapablt->Puttext("11");
		else
			pXmlTermEntryCapablt->Puttext("03");
		pXmlCommonGrp->appendChild(pXmlTermEntryCapablt);
		


		///        <TxnAmt>100</TxnAmt>
		//Length 12 Numerig
		MSXML::IXMLDOMElementPtr pXmlTxnAmt = pXMLDoc->createElement(_T("TxnAmt"));
		pXmlTxnAmt->Puttext(pTRANDATA->Amount);
		pXmlCommonGrp->appendChild(pXmlTxnAmt);
		

		///        <TxnCrncy>840</TxnCrncy>
		//Currency Codes 840 USD, 124 CANADA
		MSXML::IXMLDOMElementPtr pXmlTxnCrncy = pXMLDoc->createElement(_T("TxnCrncy"));
		pXmlTxnCrncy->Puttext("840");
		pXmlCommonGrp->appendChild(pXmlTxnCrncy);


		///        <TermLocInd>0</TermLocInd>
		// Terminal Location Indicator = 0 - On Premises, 1 - Off Premises
		MSXML::IXMLDOMElementPtr pXmlTermLocInd = pXMLDoc->createElement(_T("TermLocInd"));
		pXmlTermLocInd->Puttext("0");
		pXmlCommonGrp->appendChild(pXmlTermLocInd);




		///        <CardCaptCap>1</CardCaptCap>
		// Card Capture Capability = 0 No card capt. cap, 1 card capture capability
		// not the ability to capture the card, but to read track data...
		MSXML::IXMLDOMElementPtr pXmlCardCaptCap = pXMLDoc->createElement(_T("CardCaptCap"));
		pXmlCardCaptCap->Puttext("1");
		pXmlCommonGrp->appendChild(pXmlCardCaptCap);
	}
	
	///        <GroupID>20001</GroupID>
	// The assigned group ID by first data. GID in datawire.
	MSXML::IXMLDOMElementPtr pXmlGroupID = pXMLDoc->createElement(_T("GroupID"));
	pXmlGroupID->Puttext(pDataWire->GenerateGID());
	pXmlCommonGrp->appendChild(pXmlGroupID);
}


//-----------------------------------------------------------------------------
//!	\ IN dtLocal
//! \ OUT dtGmt
//! format: TimeUtil->GetSystemDateTimeLong(dtLocal);
//! using registry param "TZ Offset"
//-----------------------------------------------------------------------------
BOOL CRapidConnect::ApplyGMTDateOffset(char * dtLocal, char *dtGmt)
{
	TCHAR buf[3] = _T("");
	if(!CDataFile::Read(L"TZ Offset", buf))
		return FALSE;
	wstring tmp = buf;
	int size_needed = WideCharToMultiByte(CP_ACP, 0, tmp.c_str(), int(tmp.length() + 1), 0, 0, 0, 0); 
    string sOffset(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, tmp.c_str(), int(tmp.length() + 1), &sOffset[0], size_needed, 0, 0); 
	
	char hrs[3];
	memset(hrs, 0, 3);
	memcpy(hrs, &dtLocal[8], 2);
	int hours = atoi(hrs);
	int offset = atoi(sOffset.c_str());

	if (hours >= offset)
		hours -= offset;
	else
		hours = hours + 24 - offset;

	char gmthrs[2];
	sprintf(gmthrs, "%02d", hours);
	strcpy(dtGmt, dtLocal); 
	memcpy(&dtGmt[8], gmthrs, 2);

	return TRUE;
}


char* CRapidConnect::GetErrorText()
{
	return NULL;

}





