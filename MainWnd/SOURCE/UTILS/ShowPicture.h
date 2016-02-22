#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
typedef struct PICTURES
{
	void *Picture;		// pointer to the picture
	long PictureWitdh;	// picture witdh (in pixels)
	long PictureHeight; // picture height (in pixels)
	int PositionX;		// the X coordinate of the picture on the Window
	int PositionY;		// the Y coordinate of the picture on the Window
	HWND hWnd;			// the handle of the window where the picture is printed on
} Pictures,*pPictures;

class CShowPicture  
{
public:
	void RepaintPictures();
	CShowPicture();
	virtual ~CShowPicture();

	void AddPicture(HWND hWnd, unsigned char * data, unsigned int sz, int PositionX, int PositionY);
}