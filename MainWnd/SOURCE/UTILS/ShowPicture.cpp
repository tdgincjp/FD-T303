// ShowPicture.cpp: implementation of the CShowPicture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShowPicture.h"

CShowPicture::CShowPicture()
{
}

CShowPicture::~CShowPicture()
{
	RemovePictures();

}

//////////////////////////////////////////////////////////////////////
// 
void CShowPicture::AddPicture(HWND hWnd,int ResourceHandle,int PositionX,int PositionY)
{
	if(PictureLoaded)
		return;
	HRSRC res = FindResource(GetModuleHandle(NULL),MAKEINTRESOURCE(ResourceHandle),"BINARY");
	if (res) 
	{
		HGLOBAL mem = LoadResource(GetModuleHandle(NULL), res);
		void *data = LockResource(mem);
		size_t sz = SizeofResource(GetModuleHandle(NULL), res);
		/* Note: delete the memory , otherwise leak the memory */
		Picture.Picture = LoadPicture(hWnd, (unsigned char *)data, sz, &Picture.PictureWitdh, &Picture.PictureHeight);
		if (Picture.Picture != NULL)
		{
			Picture.PositionX=PositionX;
			Picture.PositionY=PositionY;
			Picture.hWnd=hWnd;
			PictureLoaded = TRUE;
		}
		else
		{
			FreeMyMemory(data);
		}
	}
}

void CShowPicture::AddPicture(HWND hWnd,unsigned char *data, unsigned int sz,int PositionX,int PositionY)
{
	if(PictureLoaded)
		return;
	Picture.Picture = LoadPicture(hWnd, (unsigned char *)data, sz, &Picture.PictureWitdh, &Picture.PictureHeight);
	if (Picture.Picture != NULL)
	{
		Picture.PositionX=PositionX;
		Picture.PositionY=PositionY;
		Picture.hWnd=hWnd;
		PictureLoaded = TRUE;
	}
	else
	{
		FreeMyMemory(data);
	}


}

HWND hWnd;                     // The main window handle

HWND hWndInputPanel = NULL;    // The SIP
HWND hWndTaskBar    = NULL;    // The TaskBar
HWND hWndSipButton  = NULL;    // The SIP Button

BOOL mode = false;             // Our current window mode.  
                               //  True = Fullscreen
                               //  False - Windowed (Startup Default)








void InitFullScreen (void)
{
    hWndInputPanel = FindWindow(TEXT("SipWndClass"), NULL);
    hWndSipButton = FindWindow(TEXT("MS_SIPBUTTON"), NULL);
    hWndTaskBar = FindWindow(TEXT("HHTaskBar"), NULL);
}







void ToggleFullScreen()
{
    RECT rtDesktop;

    if (mode)
    {
        if(hWndTaskBar != NULL)        
        ShowWindow(hWndTaskBar, SW_SHOW);
        //if(hWndInputPanel != NULL)    
        ShowWindow(hWndInputPanel, SW_SHOW);
        //Never forcibly show the input panel
        if(hWndSipButton != NULL)    
        ShowWindow(hWndSipButton, SW_SHOW);

        if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rtDesktop, NULL) == 1)
            SetWindowPos(hWnd,HWND_TOPMOST,0,0,rtDesktop.right - 
        rtDesktop.left,rtDesktop.bottom - rtDesktop.top, SWP_SHOWWINDOW);

        mode = false;
    }
    else
    {
        if (hWndTaskBar != NULL)    ShowWindow(hWndTaskBar, SW_HIDE);
        if (hWndInputPanel != NULL)    ShowWindow(hWndInputPanel, SW_HIDE);
        if (hWndSipButton != NULL)    ShowWindow(hWndSipButton, SW_HIDE);

        SetWindowPos(hWnd,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);

        mode = true;
    }
}



