/***
 *** CSample.C
 ***
 *** An example program showing the use of LiveImport from purely straight C
 ***
 *** ----------
 ***
 *** Copyright 2009, Frontline Test Equipment, Inc
 ***
 *** ----------
 ***
 *** Tom Allebrandi (tallebrandi@fte.com/tom@ytram.com)
 *** Frontline Test Equipment, Inc
 ***/

/*******************************************************************************
 **
 ** Includes
 **/
#pragma	warning(disable : 4115)	// Avoid a warning from RpcAsync.h
#include	<windows.h>
#pragma	warning(disable : 4115)
#include	<stdio.h>
#include <direct.h>
#include "LiveImportAPI.h"  // Definitions of function pointers.

/*******************************************************************************
 **
 ** Forward Declarations
 **/
bool checkLiveImportConnection(void);

/*******************************************************************************
 **
 ** Local Data
 **/

// HCI Read_BD_ADDR
// Simulate the sending of an HCI Read_BD_ADDR command to a Bluetooth Device.
BYTE sampleFrame_cmd[]= {0x09,0x010,0x00};
// Command Complete Event
// Simulate the response from a Bluetooth Device to an HCI Read_BD_ADDR command.
BYTE sampleFrame_event[]= {0x0e,0x0a,0x01,0x09,0x10,0x00,0x82,0x14,0x01,0x5b,0x02,0x00};
// Connection Complete Event
// Simulate a Connection Complete reponse from a Bluetooth Device for an ACL connection. (This establishes the connection handle for the next two buttons.)
BYTE sampleFrame_event1[]= {0x03,0x0b,0x00,0x28,0x00,0x2d,0x18,0x00,0x5b,0x08,0x00,0x01,0x00};
// Send ACL Data
// Simulate sending ACL data from a Host to a remote Bluetooth Device.
BYTE sampleFrame_acl_send[]= {0x28,0x20,0x1c,0x00,0x18,0x00,0x40,0x00,0x4d,0x65,0x73,0x73,0x61,0x67,0x65,0x20,0x74,0x6f,0x20,0x72,0x65,0x6d,0x6f,0x74,0x65,0x20,0x64,0x65,0x76,0x69,0x63,0x65};
// Receive ACL Data
// Simulate receiving ACL data from a remote Bluetooth Device.
BYTE sampleFrame_acl_recv[]= {0x28,0x20,0x1e,0x00,0x1a,0x00,0x40,0x00,0x4d,0x65,0x73,0x73,0x61,0x67,0x65,0x20,0x66,0x72,0x6f,0x6d,0x20,0x72,0x65,0x6d,0x6f,0x74,0x65,0x20,0x64,0x65,0x76,0x69,0x63,0x65};

bool g_boolSendCannedMessage= false;


/*******************************************************************************
 **
 ** Program
 **/

int	main(
	int				argc,
	char ** argv)
{
	int				i;
	char			*pszConfiguration;
	int				status = ERROR_INVALID_FUNCTION;
	SYSTEMTIME		systemtime;
	char			szConfiguration[1024];
	char			szConnectionString[1024];
	char			szIniFile[_MAX_PATH];
	__int64			timestamp;
	char buffer[81];
	int ch;
	bool boolQuit= false;
	char* psz= NULL;
	bool boolSuccess= false;
	HRESULT hr= S_OK;
	char szError[1024];
	char cPath[_MAX_PATH+1];
	char cDir[_MAX_PATH];
	char cDrive[_MAX_DRIVE];

	printf( "Initializing...\n" );

	//	__asm int 3;

	for (i=1;i< argc;i++)
	{
		if (_strnicmp( argv[i],"/mem=", 5) == 0)
			psz= &argv[i][5];
		else if (_strnicmp(argv[i],"/canned", 7) == 0)
			g_boolSendCannedMessage= true;
	}

	/*
	* Set the name of the .INI file that is shared between FTS and a Live Import
	* data source
	*/
	_tcscpy_s(szIniFile, _countof(szIniFile), "..\\LiveImport.Ini");

	// Get the connection string from the shared .INI file
	if(NULL == psz)
		GetPrivateProfileString("General", "ConnectionString", "", szConnectionString, sizeof(szConnectionString),szIniFile);
	else
		_tcscpy_s(szConnectionString, _countof(szConnectionString), psz);

	if(_tcslen(szConnectionString) == 0)
	{
		fprintf(stderr,"[General]ConnectionString not found in %s.\n",szIniFile);
		MessageBox(NULL, "[General]ConnectionString not found !", "Error", MB_OK);
		return (status);
	}

	// Get the configuration data from the shared .INI file
	GetPrivateProfileSection("Configuration", szConfiguration, sizeof(szConfiguration),szIniFile);

	if (strlen(szConfiguration) == 0)
	{
		fprintf(stderr,"[Configuration] section not found in %s.\n",szIniFile);
		MessageBox(NULL, "[Configuration] section not found !", "Error", MB_OK);
		return (status);
	}

	/*
	* The return was a buffer of null terminated strings. Live Import wants a
	* single string in which each line terminated by a new line
	*/
	pszConfiguration = szConfiguration;
	while ((i=strlen(pszConfiguration)) != 0)
	{
		pszConfiguration[i] = '\n';	// Change null character to new line
		pszConfiguration += (i + 1);	// Move to the next string
	}

#if 0
	// Initialize the FTS LiveImport Server
	GetModuleFileName(GetModuleHandle(NULL), cPath, _MAX_PATH);

	_tsplitpath_s(cPath, cDrive, _countof(cDrive), cDir, _countof(cDir), NULL, 0, NULL, 0);
	_stprintf_s(cPath, _countof(cPath), _T("%s%sLiveImportAPI.dll"), cDrive, cDir);  // Reuse cPath.
	g_pszLibraryName= cPath;
#else
	_tcscpy_s(cPath, _countof(cPath), "..\\lib\\LiveImportAPI.dll");
	g_pszLibraryName= cPath;
#endif

	if(!LoadLiveImportAPIFunctions())
		return (ERROR_INVALID_FUNCTION);

	hr = g_pInitializeLiveImport(szConnectionString, szConfiguration, &boolSuccess);

	if(FAILED(hr))				// Status flag from the server itself
	{
		_stprintf_s(szError, _countof(szError), "LiveImport.Initialize() failed with status 0x%x.\n", hr);
		MessageBox(NULL, szError, "Error", MB_OK);
		return (status);
	}
	else if(!boolSuccess)
	{
		_tcscpy_s(szError, _countof(szError), "LiveImport.Initialize() failed to initialize.\n");
		MessageBox(NULL, szError, "Error", MB_OK);
		return (status);
	}

	// Send one frame of data
	if(checkLiveImportConnection())
	{
		if(g_boolSendCannedMessage)
		{
			GetLocalTime(&systemtime);
			SystemTimeToFileTime(&systemtime,(FILETIME*) &timestamp);
			if(FAILED(status= g_pSendFrame(
				sizeof(sampleFrame_cmd),
				sizeof(sampleFrame_cmd),
				sampleFrame_cmd,
				1,				// Bluetooth HCI Command
				0,				// Bluetooth Host
					//  See BT Virtual.Dec and the [Configuration] in
					//  the LiveImport.Ini file
				timestamp)))
			{
				MessageBox(NULL, "LiveImport_SendFrame failed!", "Error", MB_OK);
				return (status);
			}

			boolQuit= true;
		}

		while(!boolQuit)
		{	
			printf( "Enter a command (\"send\" or \"quit\"): " );

			/* Read in single line from "stdin": */
			for( i = 0; (i < 80) && ((ch = getchar()) != EOF) && (ch != '\n'); i++ )
				buffer[i] = (char)ch;

			/* Terminate string with null character: */
			buffer[i] = '\0';

			if(0 == _tcsicmp(buffer, "send"))
			{
				// Generate a timestamp
				GetLocalTime(&systemtime);
				SystemTimeToFileTime(&systemtime,(FILETIME*) &timestamp);

				// Send the frame
				if (FAILED(status= g_pSendFrame(
					sizeof(sampleFrame_cmd),
					sizeof(sampleFrame_cmd),
					sampleFrame_cmd,
					1,				// Bluetooth HCI Command
					0,				// Bluetooth Host
						//  See BT Virtual.Dec and the [Configuration] in
						//  the LiveImport.Ini file
					timestamp)))
				{
					fprintf(stderr,"LiveImport_SendFrame() failed to send data: Error %d.", status);
					MessageBox(NULL, "LiveImport_SendFrame failed!", "Error", MB_OK);
					return (status);
				}

				// Send the frame
				if (FAILED(status= g_pSendFrame(
					sizeof(sampleFrame_event),
					sizeof(sampleFrame_event),
					sampleFrame_event,
					8,				// Bluetooth HCI Event
					1,				// Bluetooth Controller
						//  See BT Virtual.Dec and the [Configuration] in
						//  the LiveImport.Ini file
					timestamp)))
				{
					fprintf(stderr,"LiveImport_SendFrame() failed to send data: Error %d.", status);
					MessageBox(NULL, "LiveImport_SendFrame failed!", "Error", MB_OK);
					return (status);
				}
				
				// Send the frame
				if (FAILED(status= g_pSendFrame(
					sizeof(sampleFrame_event1),
					sizeof(sampleFrame_event1),
					sampleFrame_event1,
					8,				// Bluetooth HCI Event
					1,				// Bluetooth Controller
						//  See BT Virtual.Dec and the [Configuration] in
						//  the LiveImport.Ini file
					timestamp)))
				{
					fprintf(stderr,"LiveImport_SendFrame() failed to send data: Error %d.", status);
					MessageBox(NULL, "LiveImport_SendFrame failed!", "Error", MB_OK);
					return (status);
				}
				
				// Send the frame
				if (FAILED(status= g_pSendFrame(
					sizeof(sampleFrame_acl_send),
					sizeof(sampleFrame_acl_send),
					sampleFrame_acl_send,
					2,				// Bluetooth HCI ACL
					0,				// Bluetooth Host
						//  See BT Virtual.Dec and the [Configuration] in
						//  the LiveImport.Ini file
					timestamp)))
				{
					fprintf(stderr,"LiveImport_SendFrame() failed to send data: Error %d.", status);
					MessageBox(NULL, "LiveImport_SendFrame failed!", "Error", MB_OK);
					return (status);
				}
				
				// Send the frame
				if (FAILED(status= g_pSendFrame(
					sizeof(sampleFrame_acl_recv),
					sizeof(sampleFrame_acl_recv),
					sampleFrame_acl_recv,
					2,				// Bluetooth HCI ACL
					1,				// Bluetooth Controller
						//  See BT Virtual.Dec and the [Configuration] in
						//  the LiveImport.Ini file
					timestamp)))
				{
					fprintf(stderr,"LiveImport_SendFrame() failed to send data: Error %d.", status);
					MessageBox(NULL, "LiveImport_SendFrame failed!", "Error", MB_OK);
					return (status);
				}
			}
			else if(0 == _tcsicmp(buffer, "quit"))
				boolQuit= true;
		}  // End while(!boolQuit)
	}

	if(NULL != g_pReleaseLiveImport)
	{
	g_pReleaseLiveImport();
	NullLiveImportFunctionPointers();
	}

	FreeLibrary(g_hLiveImportAPI);

	return(status);
}

bool checkLiveImportConnection(void)
{
	bool boolConnectionIsRunning= false;
	char* pszMessage = NULL;
	int iCount= 0;
	DWORD dwStatus= 0;

// Walk through the variables and return the state
//	__asm int 3;
	while(!boolConnectionIsRunning && (iCount < 100))	// Wait no more than 100 seconds
	{
		dwStatus= g_pIsAppReady(&boolConnectionIsRunning);
		if(FAILED(dwStatus))
		{
			pszMessage= "Unspecified internal error in Live Import interface.";
			break;
		}
		
		if(!boolConnectionIsRunning)
		{
			Sleep(1000);
			iCount++;
		}
	}
	
	if((100 == iCount) && !boolConnectionIsRunning)
		pszMessage= "FTS is not ready to receive data via Live Import.";
	
	if(boolConnectionIsRunning && g_boolSendCannedMessage)
	{
		pszMessage= "FTS is ready to receive data via Live Import.  Start capture now.";
		MessageBox(NULL, pszMessage, "Success", MB_OK);
		pszMessage= NULL;
	}
	
	// Display a message if there is a problem.
	if(pszMessage != NULL)
	{
		fprintf(stderr,"%s\n",pszMessage);
		MessageBox(NULL, pszMessage, "Error", MB_OK);
	}
	
	return((bool) ((NULL == pszMessage)?true:false));	// Return true for "all is well"
}