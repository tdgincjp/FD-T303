
#include <msxml2.h>
#include "..\\COMM\\SSLCon.h"
#include "..\\defs\\struct.h"

class CDataWire 
{
public:
	CDataWire(HWND hwnd);  
	~CDataWire();
	char sClientRef[15];
	char sRefNum[8];
	BOOL CDataWire::GenerateDwWrapper(FD_Rec *pFDRec);	// Public method called by RapidConnect.cpp to wrap the payload
												 			// If DID is empty then PerformSrsProcess() is called.
	
	///Utility Functions
	char * CDataWire::GenerateAuth();		// Read from db, GID + MID|TID eg. 1000100000001|TID
	char * CDataWire::GenerateDID();		// Read from db, if blank perform all service discovery and registration
	char * CDataWire::GenerateAppKey(); 	// Hardcoded to provide key we decide for our program (plus version number from db)
	char * CDataWire::GenerateServiceID();		// Return the Service ID param
	char * CDataWire::GenerateTID();
	char * CDataWire::GenerateMID();
	char * CDataWire::GenerateGID();
	char * CDataWire::PadLeft(char * input, int len, char value);
	
	char * CDataWire::GenerateClientRef(); // Read unique number from db, increment, save


	char* error;
	int DataWireError;
	CString StatusCode;

	private:
	FD_Rec * pFDTranRec;
	HWND m_hWnd;
	//SRS Functions
	bool   CDataWire::PerformSrsProcess();							// Private entry point used when the SRS needs to be used to setup a new terminal
	bool   CDataWire::PerformServiceDiscovery();					//performs an HTTPS GET and saves the SRS URL to db (has self contained CSslConnection)
	BOOL   CDataWire::ParseServiceDiscoveryResp(CString sResponse);	// Parse response, save SRS URL
	char * CDataWire::SendHttpsRequest(CSslConnection, char *);  	//Here we create the connection to SRS URL, send the request
	char * CDataWire::SerializeRegistrationRequest();				//create registration request
	char * CDataWire::SerializeActivationRequest();					//creates the activation request
	bool   CDataWire::HttpsSendSrsRequest(bool isRegistration);		//logic to retry registration until successful
	BOOL   CDataWire::ParseRegistration(CString sResponse);			//used in HttpsSendSrsRequest
																	// Save the DID, Save the TxnURL1 and TxnURL2	
	BOOL   CDataWire::ParseActivation(CString sResponse);			//used in HttpsSendSrsRequest
																	// return true and we are ready to process transactions!
	
	char * CDataWire::ReadFileReturnContents(CString filename);

	char charReturn[SZ_DATAWIRE_CLIENT_REF+1];
};
