#include "stdafx.h"
#include "DataWire.h"
//#include "..\\COMM\\SSLCon.h"
#include "..\\defs\\struct.h"
#include "..\\Utils\\string.h"
#import "MSXML6.dll" rename_namespace(_T("MSXML"))
#include "msxml2.h"
#include "atlstr.h"
#include "..\\data\\DebugFile.h"				//JC Nov 11/15 to add debug logging 



//-----------------------------------------------------------------------------
//!	\brief  CONSTRUCT
//	Initialize RapidConnect specific varaibles and registry entries
//-----------------------------------------------------------------------------
CDataWire::CDataWire(HWND hwnd)
{
	TCHAR buf[100] = _T("");
	DataWireError=0;					//JC Oct 28/15

	StatusCode = L"";

	
	//Dealt with in RapidConnect
	//CoInitializeEx(NULL, COINIT_MULTITHREADED); 

	///////////// INITIALIZATION OF DEFAULTS /////////////////////
	if(!CDataFile::Read(L"Datawire App Key", buf))
		CDataFile::Save(L"Datawire App Key", L"RAPIDCONNECTSRS");

	if(!CDataFile::Read(L"Datawire GID", buf))
		CDataFile::Save(L"Datawire GID", L"20001");

	if(!CDataFile::Read(L"Datawire TID", buf))
		CDataFile::Save(L"Datawire TID", L"00000001");

	if(!CDataFile::Read(L"Datawire MID", buf))
		CDataFile::Save(L"Datawire MID", L"RCTST0000000514");

	if(!CDataFile::Read(L"Datawire ClientRef", buf))
		CDataFile::Save(L"Datawire ClientRef", L"001101");

	if(!CDataFile::Read(L"Datawire Project ID", buf))
		CDataFile::Save(L"Datawire Project ID", L"RTD002");

	if(!CDataFile::Read(L"DataWire SD URL", buf))
		CDataFile::Save(L"DataWire SD URL", L"prod.dw.us.fdcnet.biz"); //JC Oct 28/15 "stg.dw.us.fdcnet.biz"

	if(!CDataFile::Read(L"Datawire SRS Object", buf))
		CDataFile::Save(L"Datawire SRS Object", L"/sd/srsxml.rc");

//	sClientRef = new char[15];
//	sRefNum = new char [8];
	memset(sClientRef, 0 , 15);
	memset(sRefNum, 0 , 8);

	m_hWnd = hwnd;

	error = new char[1024];
	memset(error,0,1024);
}


//-----------------------------------------------------------------------------
//!	\brief DESTRUCT
//-----------------------------------------------------------------------------
CDataWire::~CDataWire()
{
	delete error;
	//Dealt with in RapidConnect 
	//CoUninitialize();
}




//-----------------------------------------------------------------------------
//!	\Generate the DataWire Wrapper around the RapidConnect_Payload. 
//	This is the public entry point to Datawire
//	This will kick off the SRS Process if "Datawire DID" is blank
//-----------------------------------------------------------------------------
BOOL CDataWire::GenerateDwWrapper(FD_Rec *pFDRec)
{
	CString error = CString(L"Error"); 
	pFDTranRec = pFDRec;

	//Serialize a string containing the XML to post
	MSXML::IXMLDOMDocument2Ptr pXMLDoc;    
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML::DOMDocument));		//JC Nov 14/15 Win10 DOMDocument to DOMDocument60
	if(FAILED(hr))
    {
		CDebugFile::DebugWrite("GenWrapper: Failed to create the XML class instance");							//JC Nov 12/15
        return FALSE;
    }

	
	
//////////////////////////////////////////////////////////////
/// <Request>
//////////////////////////////////////////////////////////////
	if(pXMLDoc->loadXML(_T(
		"<Request Version=\"3\" ClientTimeout=\"35\" xmlns=\"http://securetransport.dw/rcservice/xml\"></Request>"
		)) == VARIANT_FALSE)
    {
		CDebugFile::DebugWrite("GenWrapper: Failed to load the XML");											//JC Nov 24/15
        return FALSE;
    }
	MSXML::IXMLDOMElementPtr pXmlRequestElem = pXMLDoc->GetdocumentElement();

		
//////////////////////////////////////////////////////////////
/// <Processing node>
//////////////////////////////////////////////////////////////
    MSXML::IXMLDOMProcessingInstructionPtr pXMLProcessingNode =    
      pXMLDoc->createProcessingInstruction("xml", " version='1.0' encoding='UTF-8'");

    _variant_t vtObject;
    vtObject.vt = VT_DISPATCH;
    vtObject.pdispVal = pXmlRequestElem;
    vtObject.pdispVal->AddRef();

    pXMLDoc->insertBefore(pXMLProcessingNode,vtObject);

//////////////////////////////////////////////////////////////
/// <ReqClientId>
//////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlReqClientIdElem = pXMLDoc->createElement(_T("ReqClientId"));
    pXmlRequestElem->appendChild(pXmlReqClientIdElem);


	////////////////////////////////////////////////
	/// <DID> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlDIDElem = pXMLDoc->createElement(_T("DID"));
	pXmlDIDElem->Puttext(GenerateDID());
	pXmlReqClientIdElem->appendChild(pXmlDIDElem);


	
	////////////////////////////////////////////////
	/// <App> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlAppElem = pXMLDoc->createElement(_T("App"));
	pXmlAppElem->Puttext(GenerateAppKey());
	pXmlReqClientIdElem->appendChild(pXmlAppElem);


	
	////////////////////////////////////////////////
	/// <Auth> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlAuthElem = pXMLDoc->createElement(_T("Auth"));
	pXmlAuthElem->Puttext(GenerateAuth());
	pXmlReqClientIdElem->appendChild(pXmlAuthElem);

	
	////////////////////////////////////////////////
	/// <ClientRef> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlClientRefElem = pXMLDoc->createElement(_T("ClientRef"));
	pXmlClientRefElem->Puttext(sClientRef);
	pXmlReqClientIdElem->appendChild(pXmlClientRefElem);


//////////////////////////////////////////////////////////////
/// <Transaction>
//////////////////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlTransactionElem = pXMLDoc->createElement(_T("Transaction"));
	pXmlRequestElem->appendChild(pXmlTransactionElem);


	////////////////////////////////////////////////
	/// <ServiceID> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlServiceID = pXMLDoc->createElement(_T("ServiceID"));
	pXmlServiceID->Puttext(GenerateServiceID());
	pXmlTransactionElem->appendChild(pXmlServiceID);

	////////////////////////////////////////////////
	/// <Payload>
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlPayloadElem = pXMLDoc->createElement(_T("Payload"));
	MSXML::IXMLDOMAttributePtr pXmlEncodingAttr = pXMLDoc->createAttribute(_T("Encoding"));
	pXmlEncodingAttr->text = "cdata";
	pXmlPayloadElem->setAttributeNode(pXmlEncodingAttr);

	MSXML::IXMLDOMCDATASectionPtr pXmlCDATA = pXMLDoc->createCDATASection(pFDTranRec->RapidConnect_Payload);
	pXmlPayloadElem->appendChild(pXmlCDATA);
	pXmlTransactionElem->appendChild(pXmlPayloadElem);

	CString sLocation = _T("\\SAIO\\User Programs\\INIT\\DataWire.xml");
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

	char tmpBuf[SZ_DATAWIRE_MAX_BUFFER+1];
	memset(tmpBuf, '\x0', SZ_DATAWIRE_MAX_BUFFER+1);
	if (fsize < SZ_DATAWIRE_MAX_BUFFER)
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
	DeleteFile(sLocation);									//JC Nov 23/15 Delete Datawire.xml. Has PAN PCI needs to go.

	strcpy(pFDTranRec->DataWire_FullRequest, tmpBuf);

	return TRUE;
}




//-----------------------------------------------------------------------------
//!	\brief  organises service discovery, activation, and registration
//-----------------------------------------------------------------------------
bool   CDataWire::PerformSrsProcess()
{
	////////////////////////////////////////////////////
	//SRS Service Discovery
	if (!PerformServiceDiscovery())
		return false;

	///////////////////////////////////////////////////
	//Registration  
	if(!HttpsSendSrsRequest(true)) //yes isRegistration
		return false;
	
	//This must increment
	GenerateClientRef();

	////////////////////////////////////////////////////
	//Activation  
	if(!HttpsSendSrsRequest(false)) //not isRegistration
		return false;
	
	//This must increment before going back to the transaction that kicked it off!
	GenerateClientRef();

	return true;
}


//-----------------------------------------------------------------------------
//!	\brief  creates an HTTPS GET call and recieves a URL
//-----------------------------------------------------------------------------
bool   CDataWire::PerformServiceDiscovery()
{
	TCHAR buf[100];
	int len=0;
	wchar_t *mywstring;
	int sdTries= 0;

	////////////////////////////////////////////////////////////////////
	//Create SSL Connection to be used locally only
	string strVerb = "GET";
	string sAgentName("TDG");
	//Read from database the service discovery URL
	CString strSDAddressKey = L"DataWire SD URL";
	char host[100] = {"prod.dw.us.fdcnet.biz"};  //JC Oct 28/15 "stg.dw.us.fdcnet.biz"
	if(CDataFile::Read(strSDAddressKey,buf))
		strcpy(host,alloc_tchar_to_char(buf));
	char version[10] = "test";
	if(CDataFile::Read(L"VERSION",buf))
		strcpy(version,alloc_tchar_to_char(buf));
	string sServerName = host;
	char object[100] = {"/sd/srsxml.rc"};
	if(!CDataFile::Read(L"Datawire SRS Object", buf))
		strcpy(object, alloc_tchar_to_char(buf));
	string sObjectName = object;
	string sUserName("");//if required
	string sPass(""); //if required
	string sOrganizationUnitName("First Data Corporation");
	string strHeaders = "User-Agent: LocoMobi303 v";
		strHeaders += version;
		strHeaders += "\r\nHost: ";
		strHeaders += host;
		strHeaders += "\r\nCacheControl: no-cache\r\n";
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//Create SSL Connection to be used locally only
	CSslConnection discoveryHttpsConnection = CSslConnection(m_hWnd);
	//Populate connection
	discoveryHttpsConnection.SetAgentName(sAgentName);
	discoveryHttpsConnection.SetCertStoreType(certStoreCA);
	discoveryHttpsConnection.SetObjectName(sObjectName);	
	discoveryHttpsConnection.SetOrganizationName(sOrganizationUnitName);
	discoveryHttpsConnection.SetPort(INTERNET_DEFAULT_HTTPS_PORT);
	discoveryHttpsConnection.SetServerName(sServerName); 
	discoveryHttpsConnection.SetHttpHeaders(strHeaders);
	discoveryHttpsConnection.SetRequestID(0);
	///////////////////////////////////////////////////////////////////

sd_again:
	sdTries++;
	////////////////////////////////////////////////////////////////////
	//Open the SSL Connection
	if (!discoveryHttpsConnection.ConnectToHttpsServer(strVerb)) 
	{
		DataWireError = 1;						//JC Oct 28/15
		CDebugFile::DebugWrite("DataWire Error 1");			//JC Nove 9/15
		return false;
	}
	///////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//Open the SSL Connection
	if(!discoveryHttpsConnection.SendHttpsRequest())
	{
		DataWireError = 2;						//JC Oct 28/15
		CDebugFile::DebugWrite("DataWire Error 2");			//JC Nove 9/15
		return false; // we should always recieve a response
	}
	string response = discoveryHttpsConnection.GetRequestResult();
	len = response.length()+1;
	mywstring = new wchar_t[len];
	MultiByteToWideChar(CP_ACP,0,response.c_str(),-1,mywstring,len);
	CString	sResponse = mywstring;
	CDebugFile::DebugWrite("CDataWire::PerformServiceDiscovery Response----------->");	//JC Nov 9/15 test
	CDebugFile::DebugWrite((char *)response.c_str());				//JC Nov 9/15 test
	delete[] mywstring;


	if(!ParseServiceDiscoveryResp(sResponse))
	{
		if (sdTries <= 3)
			goto sd_again;
		DataWireError = 3;						//JC Oct 28/15
		CDebugFile::DebugWrite("DataWire Error 3");			//JC Nove 9/15
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
//!	\brief  retrieves the URL from the SD response
//-----------------------------------------------------------------------------
BOOL   CDataWire::ParseServiceDiscoveryResp(CString sResponse)
{
/*	int index = sResponse.Find(L"<URL>");
	if (index <= 0)
		return FALSE;

	int index1 = sResponse.Find(L"</URL>");
	if (index1 <= 0)
		return FALSE;

	CString temp = sResponse.Mid(index+5,index1-index-5);
	CDataFile::Save(L"DataWire SRS URL", temp.GetBuffer());
	temp.ReleaseBuffer();
	return TRUE;
*/

	MSXML::IXMLDOMDocument2Ptr pIXMLDOMDocument;   
	_bstr_t bstrResponse(sResponse);
	wstring strFindText (_T("URL"));
	IXMLDOMNodeListPtr pIDOMNodeList;
	IXMLDOMNode *pIDOMNode = NULL;
	long value;
	BSTR bstrItemText;
//	char error[1024];
	BOOL result = false;
	HRESULT hr = pIXMLDOMDocument.CreateInstance(__uuidof(MSXML::DOMDocument));		//JC Nov 14/15 Win10 DOMDocument to DOMDocument60
	if(FAILED(hr))
    {
			DataWireError = 4;						//JC Oct 28/15
			CDebugFile::DebugWrite("DataWire Error 4");			//JC Nove 9/15
 //       strcpy(error, "Failed to create the XML DOM instance");
    }

	try
	{
		if(pIXMLDOMDocument->loadXML(bstrResponse) == VARIANT_FALSE)
		{
			DataWireError = 5;						//JC Oct 28/15
			CDebugFile::DebugWrite("DataWire Error 5");			//JC Nove 9/15
//			strcpy(error, "Failed to Load the XML");
		}
		pIDOMNodeList = pIXMLDOMDocument->getElementsByTagName((TCHAR*)strFindText.data());

	   hr = pIDOMNodeList->get_length(&value);
	   if(SUCCEEDED(hr))
	   {
		  pIDOMNodeList->reset();
		  for(int ii = 0; ii < value; ii++)
		  {
			 pIDOMNodeList->get_item(ii, &pIDOMNode);
			 if(pIDOMNode )
			 {
				pIDOMNode->get_text(&bstrItemText);
				TCHAR tcURL[64];
				memset(tcURL, 0, 64*2);
				wcscpy(tcURL, &bstrItemText[8]);
				wstring test(&tcURL[0]);
				int loc = test.find(L"/");
				wstring wsURL = test.substr(0, loc);
				TCHAR URL[32];
				memset(URL, 0, 32*2);
				wcscpy(URL, wsURL.c_str());
				CDataFile::Save(L"DataWire SRS URL", (TCHAR *) &URL);
				//CDebugFile::DebugWrite("DataWire SRS URL");			//JC Nov 9/15
				//size_t size = wcstombs(NULL, URL, 0);				//JC Nov 9/15
				//c/har *charURL = new char[size + 1];					//JC Nov 9/15
				//wcstombs( charURL, URL, size + 1 );					//JC Nov 9/15
				//CDebugFile::DebugWrite(charURL);					//JC Nov 9/15
				pIDOMNode->Release();
				pIDOMNode = NULL;
				result = true;
			 }
		  }
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
	   DataWireError = 6;						//JC Oct 28/15
	   CDebugFile::DebugWrite("DataWire Error 6");			//JC Nove 9/15
//	   DisplayErrorToUser();
	}

	return result;

}






//-----------------------------------------------------------------------------
//!	\ isRegistration (true - registration, false - activation)
//! \ this function loops to retry activation until successful.
//-----------------------------------------------------------------------------
bool   CDataWire::HttpsSendSrsRequest(bool isRegistration)
{
	bool successfulResponse = false;
	CString sResponse;
	int  loopCounter = 0;
	int len=0;
	TCHAR buf[100];
	wchar_t *mywstring;

	CSslConnection srsHttpsConnection = CSslConnection(m_hWnd);

	////////////////////////////////////////////////////////////////////
	//Create SSL Connection to be used by SRS Service
	string strVerb = "POST";
	string sAgentName("TDG");
	CString strSRSAddressKey = L"DataWire SRS URL";
	char host[50] = {"stagingsupport.datawire.net"};//https://stagingsupport.datawire.net/nocportal/SRS.do
	
	if(CDataFile::Read(strSRSAddressKey,buf))
		strcpy(host,alloc_tchar_to_char(buf));
	else
	{
		DataWireError = 7;						//JC Oct 28/15
		CDebugFile::DebugWrite("DataWire Error 7");			//JC Nove 9/15
		return false; // We cannot continue if the SRS URL is blank, we cannot default the SRS URL either.
	}
	char version[10] = "test";
	if(CDataFile::Read(L"VERSION",buf))
		strcpy(version,alloc_tchar_to_char(buf));
	string sServerName = host;
	string sUserName("");//if required
	string sPass(""); //if required
	string sObjectName = "/nocportal/SRS.do" ;
	string sOrganizationUnitName("First Data Corporation");
	string strHeaders = "Content-Type: text/xml\r\n";
		strHeaders += "User-Agent: LocoMobi303 v";
		strHeaders += version;
		strHeaders += "\r\nHost: ";
		strHeaders += host;
		strHeaders += "\r\nCacheControl: no-cache\r\n";

		
	////////////////////////////////////////////////////////////////////
	//Populate the SSL Connection
	srsHttpsConnection.SetAgentName(sAgentName);
	srsHttpsConnection.SetCertStoreType(certStoreCA);
	srsHttpsConnection.SetObjectName(sObjectName);	
	srsHttpsConnection.SetOrganizationName(sOrganizationUnitName);
	srsHttpsConnection.SetPort(INTERNET_DEFAULT_HTTPS_PORT);
	srsHttpsConnection.SetServerName(sServerName); 
	srsHttpsConnection.SetHttpHeaders(strHeaders);
	srsHttpsConnection.SetRequestID(0);


	////////////////////////////////////////////////////////////////////
	//Open the SSL Connection
	if (!srsHttpsConnection.ConnectToHttpsServer(strVerb)) 
	{
		DataWireError = 9;						//JC Oct 28/15
		CDebugFile::DebugWrite("DataWire Error 9");			//JC Nove 9/15
		return false;
	}

	////////////////////////////////////////////////////////////////////
	// Create request, Send HttpsRequest, Loop, Wait 3 sec, send again 
	//until 100 tries or successful response.
	while(!successfulResponse && loopCounter < 5)
	{
		len = 0;
		char * request;
		if (isRegistration)
		{
			////////////////////////////////////////////////////////////////////
			//Registration Request
			request = SerializeRegistrationRequest();
		}
		else
		{
			////////////////////////////////////////////////////////////////////
			//Activation Request
			request = SerializeActivationRequest();
		}
		int len = strlen(request);

		
		////////////////////////////////////////////////////////////////////
		//Send the request
		if(!srsHttpsConnection.SendHttpsRequest(request, len))
		{
			DataWireError = 8;						//JC Oct 28/15
			CDebugFile::DebugWrite("DataWire Error 8");			//JC Nove 9/15
			return false; // we should always recieve a response
		}
			
		////////////////////////////////////////////////////////////////////
		//Read the response
		len = 0;
		string response = srsHttpsConnection.GetRequestResult();
		len = response.length()+1;
		mywstring = new wchar_t[len];
		MultiByteToWideChar(CP_ACP,0,response.c_str(),-1,mywstring,len);
		sResponse = CString(mywstring);
		CDebugFile::DebugWrite("CDataWire::HttpsSendSrsRequest Response----------->");
		CDebugFile::DebugWrite((char *)response.c_str());				//JC Nov 9/15 test
		delete[] mywstring;

		////////////////////////////////////////////////////////////////////
		//Parse Response
		if (isRegistration)
		{
			if (ParseRegistration(sResponse))
				return true;
			else
			{
				DataWireError = 10;						//JC Oct 28/15
				CDebugFile::DebugWrite("DataWire Error 10");			//JC Nove 9/15
				return false;
			}
		}
		else
		{
			if (ParseActivation(sResponse))
				return true;
		}

		
		
		////////////////////////////////////////////////////////////////////
		//Sleep and increment
		::Sleep(3000);
		loopCounter ++;
	}
	DataWireError = 11;						//JC Oct 28/15
	CDebugFile::DebugWrite("DataWire Error 11");			//JC Nove 9/15
	return false;
}


//-----------------------------------------------------------------------------
//!	\brief  serializes the XML for the registration request
//-----------------------------------------------------------------------------
char * CDataWire::SerializeRegistrationRequest()
{
//	char * error = new char[1024];
	MSXML::IXMLDOMDocument2Ptr pXMLDoc;    
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML::DOMDocument));		//JC Nov 14/15 Win10 DOMDocument to DOMDocument60
	if(FAILED(hr))
    {
        error = "Failed to create the XML DOM instance";
		CDebugFile::DebugWrite(error);			//JC Nove 9/15
    }

//////////////////////////////////////////////////////////////
/// <Request>
//////////////////////////////////////////////////////////////
	if(pXMLDoc->loadXML(_T("<Request></Request>")) == VARIANT_FALSE)
    {
		error = "Failed to Load the XML";
		CDebugFile::DebugWrite(error);			//JC Nove 9/15
    }
	MSXML::IXMLDOMElementPtr pXmlRequestElem = pXMLDoc->GetdocumentElement();


    //Add an attribute
    pXmlRequestElem->setAttribute(_T("Version"),_variant_t(_T("3")));

	
//////////////////////////////////////////////////////////////
/// <Processing node>
//////////////////////////////////////////////////////////////
    MSXML::IXMLDOMProcessingInstructionPtr pXMLProcessingNode =    
      pXMLDoc->createProcessingInstruction("xml", " version='1.0' encoding='UTF-8'");

    _variant_t vtObject;
    vtObject.vt = VT_DISPATCH;
    vtObject.pdispVal = pXmlRequestElem;
    vtObject.pdispVal->AddRef();

    pXMLDoc->insertBefore(pXMLProcessingNode,vtObject);





	////////////////////////////////////////////////
	/// <ReqClientID>> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlReqClientID = pXMLDoc->createElement(_T("ReqClientID")); 
	pXmlRequestElem->appendChild(pXmlReqClientID);


		/// <DID>
	MSXML::IXMLDOMElementPtr pXmlDID = pXMLDoc->createElement(_T("DID"));
	pXmlReqClientID->appendChild(pXmlDID);
	

		/// <App>
	MSXML::IXMLDOMElementPtr pXmlApp = pXMLDoc->createElement(_T("App"));
	pXmlApp->Puttext(GenerateAppKey());
	pXmlReqClientID->appendChild(pXmlApp);


		/// <Auth>
	MSXML::IXMLDOMElementPtr pXmlAuth = pXMLDoc->createElement(_T("Auth"));
	pXmlAuth->Puttext(GenerateAuth());
	pXmlReqClientID->appendChild(pXmlAuth);


		/// <ClientRef>
	MSXML::IXMLDOMElementPtr pXmlClientRef = pXMLDoc->createElement(_T("ClientRef"));
	pXmlClientRef->Puttext(sClientRef);
	pXmlReqClientID->appendChild(pXmlClientRef);


	
	////////////////////////////////////////////////
	/// <Registration>
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlActivation = pXMLDoc->createElement(_T("Registration")); 
	pXmlRequestElem->appendChild(pXmlActivation);
	
	/// <ServiceID>
	MSXML::IXMLDOMElementPtr pXmlServiceID = pXMLDoc->createElement(_T("ServiceID"));
	pXmlServiceID->Puttext(GenerateServiceID());
	pXmlActivation->appendChild(pXmlServiceID);



	//////////////////////////////////////////////////////////////////
	///////// SAVE THE FILE
	/////////////////////////////////////////////////////////////////
	CDebugFile::DebugWrite("Writing Registration.xml");							//JC Nov 12/15
	CString sLocation = _T("\\SAIO\\User Programs\\INIT\\Registration.xml");
	CStringA fileloc(sLocation);
	DeleteFile(sLocation);
	try
	{
		hr = pXMLDoc->save(sLocation.AllocSysString());
	}
	catch(_com_error errorObject)
    {
        printf("Exception thrown, HRESULT: 0x%08x", errorObject.Error());
		return error;
    }
    catch(...)
    {
        sprintf(error, "Unknown error");
		return error;
    } 
	//////////////////////////////////////////////////////////////
	//////////////////////READ AND RETURN/////////////////////////
	return ReadFileReturnContents(sLocation);
}

//-----------------------------------------------------------------------------
//!	\brief  serializes the XML for the activation request
//-----------------------------------------------------------------------------
char * CDataWire::SerializeActivationRequest()
{
//	char * error = new char[1024];
	MSXML::IXMLDOMDocument2Ptr pXMLDoc;    
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML::DOMDocument));		//JC Nov 14/15 Win10 DOMDocument to DOMDocument60
	if(FAILED(hr))
    {
        error = "Failed to create the XML DOM instance";
		CDebugFile::DebugWrite("Activation");			//JC Nove 9/15
		CDebugFile::DebugWrite(error);			//JC Nove 9/15
    }

//////////////////////////////////////////////////////////////
/// <Request>>
//////////////////////////////////////////////////////////////
	if(pXMLDoc->loadXML(_T("<Request></Request>")) == VARIANT_FALSE)
    {
		error = "Failed to Load the XML";
		CDebugFile::DebugWrite("Activation");			//JC Nove 9/15
		CDebugFile::DebugWrite(error);			//JC Nove 9/15
    }
	MSXML::IXMLDOMElementPtr pXmlRequestElem = pXMLDoc->GetdocumentElement();

	
    //Add an attribute
    pXmlRequestElem->setAttribute(_T("Version"),_variant_t(_T("3")));

	
//////////////////////////////////////////////////////////////
/// <Processing node>
//////////////////////////////////////////////////////////////
    MSXML::IXMLDOMProcessingInstructionPtr pXMLProcessingNode =    
      pXMLDoc->createProcessingInstruction("xml", " version='1.0' encoding='UTF-8'");

    _variant_t vtObject;
    vtObject.vt = VT_DISPATCH;
    vtObject.pdispVal = pXmlRequestElem;
    vtObject.pdispVal->AddRef();

    pXMLDoc->insertBefore(pXMLProcessingNode,vtObject);


	////////////////////////////////////////////////
	/// <ReqClientID>> 
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlReqClientID = pXMLDoc->createElement(_T("ReqClientID")); 
	pXmlRequestElem->appendChild(pXmlReqClientID);


		/// <DID>
	MSXML::IXMLDOMElementPtr pXmlDID = pXMLDoc->createElement(_T("DID"));
	pXmlDID->Puttext(GenerateDID());
	pXmlReqClientID->appendChild(pXmlDID);
	

		/// <App>
	MSXML::IXMLDOMElementPtr pXmlApp = pXMLDoc->createElement(_T("App"));
	pXmlApp->Puttext(GenerateAppKey());
	pXmlReqClientID->appendChild(pXmlApp);


		/// <Auth>
	MSXML::IXMLDOMElementPtr pXmlAuth = pXMLDoc->createElement(_T("Auth"));
	pXmlAuth->Puttext(GenerateAuth());
	pXmlReqClientID->appendChild(pXmlAuth);


		/// <ClientRef>
	MSXML::IXMLDOMElementPtr pXmlClientRef = pXMLDoc->createElement(_T("ClientRef"));
	pXmlClientRef->Puttext(sClientRef);
	pXmlReqClientID->appendChild(pXmlClientRef);


	
	////////////////////////////////////////////////
	/// <Activation>
	////////////////////////////////////////////////
	MSXML::IXMLDOMElementPtr pXmlActivation = pXMLDoc->createElement(_T("Activation")); 
	pXmlRequestElem->appendChild(pXmlActivation);
	
	/// <ServiceID>
	MSXML::IXMLDOMElementPtr pXmlServiceID = pXMLDoc->createElement(_T("ServiceID"));
	pXmlServiceID->Puttext(GenerateServiceID());
	pXmlActivation->appendChild(pXmlServiceID);



	//////////////////////////////////////////////////////////////////
	///////// SAVE THE FILE
	/////////////////////////////////////////////////////////////////
CDebugFile::DebugWrite("Writing Activation.xml");							//JC Nov 12/15
	CString sLocation = _T("\\SAIO\\User Programs\\INIT\\Activation.xml");
	CStringA fileloc(sLocation);
	DeleteFile(sLocation);
	try
	{
		hr = pXMLDoc->save(sLocation.AllocSysString());
	}
	catch(_com_error errorObject)
    {
        printf("Exception thrown, HRESULT: 0x%08x", errorObject.Error());
		return error;
    }
    catch(...)
    {
        sprintf(error, "Unknown error");
		return error;
    } 
	//////////////////////////////////////////////////////////////
	//////////////////////READ AND RETURN/////////////////////////
	return ReadFileReturnContents(sLocation);
}

//-----------------------------------------------------------------------------
//!	\brief  parses the XML from the registration response
//-----------------------------------------------------------------------------
BOOL   CDataWire::ParseRegistration(CString sResponse)
{
	CDebugFile::DebugWrite("In Parse Registration");			//JC Nove 9/15
	CT2CA pszConvertedString(sResponse);
	string test(pszConvertedString);
	size_t loc = test.find("<Response ");
	string trimmed = test.substr(loc);
	CString sTrimmedResp(trimmed.c_str());
	_bstr_t bstrResponse(sTrimmedResp);
	MSXML::IXMLDOMDocument2Ptr pIXMLDOMDocument;   
	wstring strFindDID (_T("DID"));
	wstring strFindURL (_T("URL"));
	IXMLDOMNodeListPtr pIDOMNodeList;
	IXMLDOMNode *pIDOMNode = NULL;
	long value;
	BSTR bstrItemText;
//	char * error = new char[1024];
	BOOL result = false;

	HRESULT hr = pIXMLDOMDocument.CreateInstance(__uuidof(MSXML::DOMDocument));		//JC Nov 14/15 Win10 DOMDocument to DOMDocument60
	if(FAILED(hr))
    {
        error = "Failed to create the XML DOM instance";
		CDebugFile::DebugWrite("ParseRegistration");			//JC Nove 9/15
		CDebugFile::DebugWrite(error);			//JC Nove 9/15
		return false;
    }

	try
	{
		if(pIXMLDOMDocument->loadXML(bstrResponse) == VARIANT_FALSE)
		{
			error = "Failed to Load the XML";
			CDebugFile::DebugWrite("ParseRegistration");			//JC Nove 9/15
			CDebugFile::DebugWrite(error);			//JC Nove 9/15
		}
		pIDOMNodeList = pIXMLDOMDocument->getElementsByTagName((TCHAR*)strFindDID.data());

	   hr = pIDOMNodeList->get_length(&value);
	   if(SUCCEEDED(hr))
	   {
		  pIDOMNodeList->reset();
		  for(int ii = 0; ii < value; ii++)
		  {
			 pIDOMNodeList->get_item(ii, &pIDOMNode);
			 if(pIDOMNode )
			 {
				pIDOMNode->get_text(&bstrItemText);
				//CDebugFile::DebugWrite((char *) bstrItemText);			//JC Nove 9/15
				if (bstrItemText[0] != '\x0')
				{
					CDataFile::Save(L"DataWire DID", (TCHAR *) bstrItemText);
					pIDOMNode->Release();
					pIDOMNode = NULL;
					result = true;
				}
				else
				{
					pIDOMNodeList->Release();
					pIDOMNodeList = NULL;
					return false;
				}
			 }
		  }
	   }
	   pIDOMNodeList->Release();
	   pIDOMNodeList = NULL;

	   pIDOMNodeList = pIXMLDOMDocument->getElementsByTagName((TCHAR*)strFindURL.data());

	   hr = pIDOMNodeList->get_length(&value);
	   if(SUCCEEDED(hr))
	   {
	     CDebugFile::DebugWrite("Succeded(HR)");			//JC Nove 9/15
		  pIDOMNodeList->reset();
		  for(int ii = 0; ii < value; ii++)
		  {
			 pIDOMNodeList->get_item(ii, &pIDOMNode);
			 if(pIDOMNode )
			 {
				pIDOMNode->get_text(&bstrItemText);
				TCHAR tcURL[64];
				memset(tcURL, 0, 64*2);
				wcscpy(tcURL, &bstrItemText[8]);
				wstring test(&tcURL[0]);
				int loc = test.find(L"/");
				wstring wsURL = test.substr(0, loc);
				TCHAR URL[32];
				memset(URL, 0, 32*2);
				wcscpy(URL, wsURL.c_str());
				CDataFile::Save(L"DataWire Txn URL", (TCHAR *) &URL);
				CDebugFile::DebugWrite("Datawire Txn URL written to registry");			//JC Nove 9/15
				pIDOMNode->Release();
				pIDOMNode = NULL;
				result = true;
			 }
		  }
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

	return result;
}


//-----------------------------------------------------------------------------
//!	\brief  parses the XML from the activation response
//-----------------------------------------------------------------------------
BOOL   CDataWire::ParseActivation(CString sResponse)
{
	CT2CA pszConvertedString(sResponse);
	string test(pszConvertedString);
	size_t loc = test.find("<Response ");
	string trimmed = test.substr(loc);
	CString sTrimmedResp(trimmed.c_str());
	_bstr_t bstrResponse(sTrimmedResp);
	MSXML::IXMLDOMDocument2Ptr pIXMLDOMDocument;
	wstring strFindStatus (_T("Status"));
	wstring strFindStatusCode (_T("StatusCode"));
	IXMLDOMNodeListPtr pIDOMNodeList;
	IXMLDOMNode *pIDOMNode = NULL;
	long value;
	BSTR bstrItemText;
//	char * error = new char[1024];
	BOOL result = false;
	HRESULT hr = pIXMLDOMDocument.CreateInstance(__uuidof(MSXML::DOMDocument));		//JC Nov 14/15 Win10 DOMDocument to DOMDocument60
	if(FAILED(hr))
    {
        error = "Failed to create the XML DOM instance";
		CDebugFile::DebugWrite("ParseActivation");			//JC Nove 9/15
		CDebugFile::DebugWrite(error);			//JC Nove 9/15
    }

	try
	{
		if(pIXMLDOMDocument->loadXML(bstrResponse) == VARIANT_FALSE)
		{
			error = "Failed to Load the XML";
			CDebugFile::DebugWrite("ParseActivation");			//JC Nove 9/15
			CDebugFile::DebugWrite(error);			//JC Nove 9/15
		}
		pIDOMNodeList = pIXMLDOMDocument->getElementsByTagName((TCHAR*)strFindStatus.data());

	   hr = pIDOMNodeList->get_length(&value);
	   if(SUCCEEDED(hr))
	   {
		  pIDOMNodeList->reset();
		  for(int ii = 0; ii < value; ii++)
		  {
			 pIDOMNodeList->get_item(ii, &pIDOMNode);
			 if(pIDOMNode )
			 {
				 IXMLDOMNamedNodeMap *atrStatusCode;
				 pIDOMNode->get_attributes(&atrStatusCode);
				 if(atrStatusCode)
				 {
					atrStatusCode->getNamedItem((TCHAR *)strFindStatusCode.data(), &pIDOMNode);
					if(pIDOMNode )
					{
						pIDOMNode->get_text(&bstrItemText);
						if (wcscmp(bstrItemText, (const wchar_t *)"Success"))
						{
							result = true;
						}
						StatusCode = CString(bstrItemText);
					}
				 }
				pIDOMNode->Release();
				pIDOMNode = NULL;
			 }
		  }
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

	return result;

}



//-----------------------------------------------------------------------------
//!	\brief  retrieves the DID from key "Datawire DID", if blank starts SRS Process 
//-----------------------------------------------------------------------------
char * CDataWire::GenerateDID()			//JC Nov 23/15, Name should be ValidateDID, we only Generate if there is no DID
{
	TCHAR DID[SZ_DATAWIRE_DID] = _T("");
	if (!CDataFile::Read(L"Datawire DID", DID)) // No DID so.....
	{
		//////WE NEED TO PERFORM THE SRS PROCESS
		if (PerformSrsProcess())
		{
			CDataFile::Read(L"Datawire DID", DID);
		}
	}
	size_t size = wcstombs(NULL, DID, 0);
	char* charDID = new char[size + 1];
	wcstombs( charDID, DID, size + 1 );
	if (charDID =="")
	{
		if(PerformSrsProcess())
			CDataFile::Read(L"Datawire DID", DID);
		size = wcstombs(NULL, DID, 0);
		charDID = new char[size + 1];
		wcstombs( charDID, DID, size + 1 );
	}
	
	return charDID;
}




//-----------------------------------------------------------------------------
//!	\brief  retrieves the "Datawire App Key" 
//-----------------------------------------------------------------------------
char * CDataWire::GenerateAppKey()
{
	TCHAR AppKey[100] = _T("");
	char* charAppKey = NULL;
	if(CDataFile::Read(L"Datawire App Key", AppKey))
	{
		size_t size = wcstombs(NULL, AppKey, 0);
		charAppKey = new char[size + 1];
		wcstombs( charAppKey, AppKey, size + 1 );
	}

	return charAppKey;
}



//-----------------------------------------------------------------------------
//!	\brief  Generates the AuthKey 1 and AuthKey 2 parameters from GID, MID, and TID
//  and combines them to create Auth as "GIDMID|TID"
//-----------------------------------------------------------------------------
char * CDataWire::GenerateAuth()
{	
	//GID+MID|TID
	TCHAR BAR[2] = _T("|");
	char* charGID = GenerateGID();
	char* charMID = GenerateMID();
	char* charTID = GenerateTID();
	char * charBar = new char[2];
	wcstombs( charBar, BAR, 2);
	char * charAuth = new char [SZ_DATAWIRE_GID+SZ_DATAWIRE_GID+SZ_DATAWIRE_TID+2];
	strcpy(charAuth, charGID);
	strcat(charAuth, charMID);
	strcat(charAuth, charBar);
	strcat(charAuth, charTID);
	return charAuth;
}

//-----------------------------------------------------------------------------
//!	\brief  retrieves the "Datawire TID" 
//-----------------------------------------------------------------------------
char * CDataWire::GenerateTID()
{
	TCHAR TID[SZ_DATAWIRE_TID] = _T("");
	CDataFile::Read(L"Datawire TID", TID);
	size_t sizeT = wcstombs(NULL, TID, 0);
	char* charTID = new char[sizeT + 1];
	wcstombs( charTID, TID, sizeT + 1 );
	return charTID;
}

//-----------------------------------------------------------------------------
//!	\brief  retrieves the "Datawire MID" 
//-----------------------------------------------------------------------------
char * CDataWire::GenerateMID()
{
	TCHAR MID[SZ_DATAWIRE_MID] = _T("");
	CDataFile::Read(L"Datawire MID", MID);
	size_t sizeM = wcstombs(NULL, MID, 0);
	char* charMID = new char[sizeM + 1];
	wcstombs( charMID, MID, sizeM + 1 );
	return charMID;
}

//-----------------------------------------------------------------------------
//!	\brief  retrieves the "Datawire GID" 
//-----------------------------------------------------------------------------
char * CDataWire::GenerateGID()
{
	TCHAR GID[SZ_DATAWIRE_GID] = _T("");
	CDataFile::Read(L"Datawire GID", GID);
	size_t sizeG = wcstombs(NULL, GID, 0);
	char* charGID = new char[sizeG + 1];
	wcstombs( charGID, GID, sizeG + 1 );
	return charGID;
}



//-----------------------------------------------------------------------------
//!	\brief  retrieves the "Datawire ClientRef", parses to an int, increments 
// and saves the client ref number. sets the sClientRef gloabal to the client ref
// number plus the "Datawire Project ID"
//!/ CCCCCCCVPPPPPP		(C- client ref, V- the letter "V", P- project ID "RTD002"
//-----------------------------------------------------------------------------
char * CDataWire::GenerateClientRef()
{
	TCHAR buf[10] = _T("");
	TCHAR ClientRef[SZ_DATAWIRE_CLIENT_REF] = _T("");
	CDataFile::Read(L"Datawire ClientRef", ClientRef);
	size_t size = wcstombs(NULL, ClientRef, 0);
	char* charClientRef = new char[size + 1];
	wcstombs( charClientRef, ClientRef, size + 1 );
	int iClientRef = atoi(charClientRef);
	delete charClientRef;
	iClientRef++;
	char newClientRef[7];
	sprintf(newClientRef, "%d", iClientRef);
	TCHAR saveClientRef [7];
	size_t copied, len;
	copied = 0;
	len = strlen(newClientRef)+1;
	mbstowcs_s(&copied,saveClientRef, len, newClientRef, _TRUNCATE);
	CDataFile::Save(L"Datawire ClientRef", saveClientRef);

	memset(charReturn, 0, SZ_DATAWIRE_CLIENT_REF+1);
	char * sconvert = PadLeft(newClientRef, 7, '0');
	strcpy(sRefNum , PadLeft(sconvert, 7, '0'));
	strcpy(charReturn, sconvert);
	strcat(charReturn, "V");
	if(CDataFile::Read(L"Datawire Project ID", buf))
		strcat(charReturn, alloc_tchar_to_char(buf));
	strcpy(sClientRef, charReturn);
	return charReturn;
}




//-----------------------------------------------------------------------------
//!	\brief  hardcode service id to "160"
//-----------------------------------------------------------------------------
char * CDataWire::GenerateServiceID()
{
	char * sServiceID = "160";
	return sServiceID;
}



//-----------------------------------------------------------------------------
//!	\brief  Reads file at filename location, returns the contents in a char *
//-----------------------------------------------------------------------------
char * CDataWire::ReadFileReturnContents(CString filename)
{
	CStringA fileloc(filename);
	FILE* f = fopen((const char *)fileloc, "r");

	// Determine file size
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);

	char* test = new char[fsize];
	memset(test, '\x0', fsize);
	fseek(f, 0, SEEK_SET);
	fread(test, sizeof(char), fsize, f);
	fclose(f);
	// JC Test Nov 8/15 DeleteFile(filename);

	return test;
}

//-----------------------------------------------------------------------------
//!	\brief  Utility function to pad input with value for len and return a new char
//-----------------------------------------------------------------------------
char * CDataWire::PadLeft(char * input, int len, char value)
{
	char * retval = new char[len];
	memset(retval, value, len);
	int inputlen = strlen(input);
	for(int i = 0; i < inputlen; i++)
	{
		retval[len-1-i] = input[inputlen-1-i];
	}
	return retval;
}

