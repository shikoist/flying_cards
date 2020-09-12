//------------------------------------------------------
//  File:  DRAW.CPP
//  Description: Demonstration basics of DirectDraw
//------------------------------------------------------
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <ctime>
#include <stdio.h>

#include "draw.h"
#include "main.h"
#include "resource.h"

// Min and max speed
#define MAX_SPEED 2
#define MIN_SPEED 1

// Max number of sprites
#define MAX_SPRITES 3

static HWND hMainWnd;

LPDIRECTDRAW pDD;
LPDIRECTDRAWSURFACE pPrimarySurface;
LPDIRECTDRAWSURFACE pBackBuffer;
LPDIRECTDRAWPALETTE pDDPal;

char* pFileNames[] = 
{
	"ess1868f_Animated.bmp",
	"TVGA-9000C_2_coll.bmp",
	"3dfx_voodoo.bmp"
};

// Class for sprite
class Sprite
{
	public:
	LPDIRECTDRAWSURFACE pPicFrame; // Поверхность DirectDraw
	int x; // Start coordinates
	int y; //
	int x1; // Speed at axe X
	int y1; // Speed at axe Y
	int w; // Width of full picture, not a frame
	int h; // Height of full picture
	
	Sprite()
	{
		// Start position on a screen
		x = rand()%MAX_WIDTH - FRAME_WIDTH;
		y = rand()%MAX_HEIGHT - FRAME_HEIGHT;
		
		// Start speeds
		x1 = 1;
		y1 = 1;
		
		// Size of a picture
		w = 64;
		h = 64;
	}
};

class AnimatedSprite: public Sprite
{
	public:
	int framesHorizontal;
	int framesVertical;
	int currentFrame;
	int maxFrame;
	
	AnimatedSprite()
	{
		framesHorizontal = 2;
		framesVertical = 4;
		currentFrame = 0;
		maxFrame = 7;
		// 8 frames
	}
};

class SpriteCollection
{
public:
	AnimatedSprite sprites[MAX_SPRITES];
};

SpriteCollection spriteCollection;

void Log(int v)
{
	HANDLE hFile = CreateFile("debug.log", GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	DWORD bytesWritten = 0;

	char n[100];
	sprintf(n, "%d", v);

	SetFilePointer(hFile, 0, 0, FILE_END);
	WriteFile(hFile, n, strlen(n), &bytesWritten, 0);
	
	CloseHandle(hFile);
}

void Log(char* dbg)
{
	HANDLE hFile = CreateFile("debug.log", GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	DWORD bytesWritten = 0;

	SetFilePointer(hFile, 0, 0, FILE_END);
	WriteFile(hFile, dbg, strlen(dbg), &bytesWritten, 0);
	
	CloseHandle(hFile);
}

void ErrorHandle(HWND hwnd, LPCTSTR szError)
{
	char szErrorMessage[255];
	
	// We need to quit DirectDraw normally
	RemoveDirectDraw();
	
	// Hide main window
	ShowWindow(hwnd, SW_HIDE);
	
	// Show error message
	wsprintf(szErrorMessage, "Программа прервана\nОшибка в %s", szError);
	MessageBox(hwnd, szErrorMessage, AppName, MB_OK);
	
	DestroyWindow(hwnd);
}

void RemoveDirectDraw()
{
	// Check for existing IDirectDraw interface
	if (pDD != NULL)
	{
		// Check primary surface interface
		if (pPrimarySurface!=NULL)
		{
			// Releasing of primary surface
			pPrimarySurface->Release();
			pPrimarySurface = NULL;
		}
		
		// Checking sprites
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame != NULL)
			{
				spriteCollection.sprites[i].pPicFrame->Release();
				spriteCollection.sprites[i].pPicFrame = NULL;
			}
		}
		
		// Checking palette interface
		if(pDDPal!=NULL)
		{
			// Destroy palette interface
			pDDPal->Release();
			pDDPal=NULL;
		}
		
		// Destroy DirectDraw interface
		pDD->Release();
		pDD=NULL;
	}
}
//---------------------------------------------------------
// Initialization DirectDraw
//
BOOL InitDirectDraw (HWND hwnd)
{
	Log("InitDirectDraw"); Log("\n");

	// NULL all interfaces
	pPrimarySurface = NULL;
	pBackBuffer = NULL;
	pDDPal = NULL;
	
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].pPicFrame = NULL; 
	}
	
	hMainWnd = hwnd;
	
	// Variable to return error codes
	HRESULT hRet;
	
	// Creating IDirectDraw interface
	hRet=DirectDrawCreate(NULL, &pDD, NULL);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd, "DirectDrawCreate");
		return (FALSE);
	}
	
	// Set exclusive fullscreen mode
	hRet=pDD->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetCooperativeLevel");
		return (FALSE);
	}
	
	// Set display mode
	hRet=pDD->SetDisplayMode(MAX_WIDTH, MAX_HEIGHT, COLOR_DEPTH);
	if (hRet != DD_OK)
	{
		ErrorHandle(hMainWnd, "SetDisplayMode");
		return (FALSE);
	}
	
	// Calling function of creating surfaces
	if (!CreateSurfaces())
	{
		ErrorHandle(hMainWnd, "CreateSurfaces");
		return (FALSE);
	}
	
	// Prepare surfaces
	if (!PrepareSurfaces())
	{
		ErrorHandle(hMainWnd, "PrepareSurfaces");
		return (FALSE);
	}
	
	return (TRUE);
}
//---------------------------------------------------------
// Creating surfaces
//
BOOL CreateSurfaces()
{
	// Declaration structures and variables
	// for DirectDraw functions
	DDSURFACEDESC ddSurfaceDesc;
	DDSCAPS ddsCaps;
	HRESULT hRet;
	
	// Clear structure from garbage and set size
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);
	
	// Set fields of structure
	ddSurfaceDesc.dwFlags = 
		DDSD_CAPS | 
		DDSD_BACKBUFFERCOUNT;
	ddSurfaceDesc.ddsCaps.dwCaps = 
		DDSCAPS_PRIMARYSURFACE | 
		DDSCAPS_FLIP | 
		DDSCAPS_COMPLEX;
	ddSurfaceDesc.dwBackBufferCount = 1;
	
	// Creating of surface
	hRet = pDD->CreateSurface(&ddSurfaceDesc, &pPrimarySurface, NULL);
	if (hRet!=DD_OK)
		return (FALSE);
	
	// Creating secondary buffer
	ZeroMemory(&ddsCaps, sizeof(ddsCaps));
	ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
	hRet = pPrimarySurface->GetAttachedSurface(&ddsCaps, &pBackBuffer);
	if (hRet != DD_OK)
		return (FALSE);
	
	// Creating offscreen surfaces for sprites
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
		ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
		ddSurfaceDesc.dwFlags = 
			DDSD_CAPS | 
			DDSD_HEIGHT | 
			DDSD_WIDTH;
		ddSurfaceDesc.ddsCaps.dwCaps = 
			DDSCAPS_OFFSCREENPLAIN;
		ddSurfaceDesc.dwHeight = spriteCollection.sprites[i].h;
		ddSurfaceDesc.dwWidth = spriteCollection.sprites[i].w;
		hRet=pDD->CreateSurface(
			&ddSurfaceDesc,
			&spriteCollection.sprites[i].pPicFrame,
			NULL);
		if (hRet != DD_OK) return (FALSE);
	}
	
	// Setting structure with color keys
	DDCOLORKEY ddColorKey;
	ddColorKey.dwColorSpaceLowValue = TRASPARENT_COLOR;
	ddColorKey.dwColorSpaceHighValue = TRASPARENT_COLOR;
	
	// Setting color keys for all surfaces
	for (i = 0; i < MAX_SPRITES; i++)
		spriteCollection.sprites[i].pPicFrame->SetColorKey(
			DDCKEY_SRCBLT, 
			&ddColorKey
		);
	return (TRUE);
}
//---------------------------------------------------------
// Creating of the palette from the resource
//
LPDIRECTDRAWPALETTE CreateDirectDrawPaletteFromResource(LPDIRECTDRAW pDD)
{
	Log("CreatePaletteFormResource"); Log("\n");

	// Declaration of interfaces and structures 
	// for working with palette
	LPDIRECTDRAWPALETTE pDirectDrawPal;
	PALETTEENTRY palEntries[256];
	HRESULT hRet;
	LPRGBQUAD pColorTable;
	UINT uMemNeed = sizeof(RGBQUAD)*256;
	int i = 0;

	// Very hard to find how correctly to load binary resource!
	// Example here
	HRSRC rc = NULL;
	rc = FindResource(NULL, MAKEINTRESOURCE(IDR_PALETTE1), RT_RCDATA);
	HGLOBAL hgl = LoadResource(NULL, rc);
	BYTE *paletteBytes = (BYTE*)LockResource(hgl);
	FreeResource(hgl);
	
	// Allocation memory for color table
	pColorTable = (LPRGBQUAD) malloc (uMemNeed);
	
	// Copy palette data
	// Offset 24 bytes because palette saved as a file
	memcpy(pColorTable, paletteBytes + 24, uMemNeed);
	
	// Logging
	//Log("pColorTable ");Log(sizeof(pColorTable));Log("\n");
	
	//Log("paletteBytes ");Log(sizeof(paletteBytes));Log("\n");
	
	//Log("uMemNeed ");Log(uMemNeed);Log("\n");

	for (i = 0; i < 256; i++)
	{
		Log(i);
		Log(" pColorTable r ");Log(pColorTable[i].rgbRed);
		Log(" g ");Log(pColorTable[i].rgbGreen);
		Log(" b ");Log(pColorTable[i].rgbBlue);
		Log("\n");
	}
	
	// Converting palette from RGBQUAD to RGBTRIPPLE
	for (i = 0; i < 256; i++)
	{
		palEntries[i].peRed = pColorTable[i].rgbRed;
		palEntries[i].peGreen = pColorTable[i].rgbGreen;
		palEntries[i].peBlue = pColorTable[i].rgbBlue;
	}
	
	// Creating DirectDraw palette
	hRet = pDD->CreatePalette(
		DDPCAPS_8BIT |
		DDPCAPS_ALLOW256,
		palEntries,
		&pDirectDrawPal,
		NULL
	);
	if (hRet != DD_OK) pDirectDrawPal = NULL;
	
	// Free memory
	free(pColorTable);
	
	return (pDirectDrawPal);
}
//---------------------------------------------------------
// Creating of a palette from the BMP file
//
LPDIRECTDRAWPALETTE CreateDirectDrawPaletteFromFile(LPDIRECTDRAW pDD)
{
	Log("CreatePaletteFromFile"); Log("\n");

	// Declaration of interfaces and structures 
	// for working with palette
	LPDIRECTDRAWPALETTE pDirectDrawPal;
	PALETTEENTRY palEntries[256];
	HRESULT hRet;
	LPRGBQUAD pColorTable;
	UINT uMemNeed = sizeof(RGBQUAD)*256;
	int i = 0;
	DWORD nBytesRead;
	
	// Opening graphics file containing palette
	HANDLE hFile = CreateFile(
		"C:\\Projects\\Flying_Cards_dx5_v3\\bitmaps\\ess1868f_animated.bmp",
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		pDirectDrawPal = NULL;
		return (pDirectDrawPal);
	}

	// Allocation memory for color table
	pColorTable = (LPRGBQUAD) malloc (uMemNeed);
	
	// Setting file pointer to a start palette
	SetFilePointer(
		hFile,
		sizeof (BITMAPFILEHEADER) + 
		sizeof (BITMAPINFOHEADER),
		NULL,
		FILE_BEGIN
	);
	
	// Reading palette from file
	ReadFile(hFile, (LPVOID)pColorTable, uMemNeed, &nBytesRead, NULL);
	
	// Closing file
	CloseHandle(hFile);
	
	// Logging
	Log("pColorTable from BMP ");Log(sizeof(pColorTable));Log("\n");
	Log("uMemNeed ");Log(uMemNeed);Log("\n");
	for (i = 0; i < 256; i++)
	{
		Log(i);
		Log(" pColorTable r ");Log(pColorTable[i].rgbRed);
		Log(" g ");Log(pColorTable[i].rgbGreen);
		Log(" b ");Log(pColorTable[i].rgbBlue);
		Log("\n");
	}

	// Converting palette from RGBQUAD to RGBTRIPPLE
	for (i = 0; i < 256; i++)
	{
		palEntries[i].peRed = pColorTable[i].rgbRed;
		palEntries[i].peBlue = pColorTable[i].rgbBlue;
		palEntries[i].peGreen = pColorTable[i].rgbGreen;
	}
	
	// Creating DirectDraw palette
	hRet = pDD->CreatePalette(
		DDPCAPS_8BIT |
		DDPCAPS_ALLOW256,
		palEntries,
		&pDirectDrawPal,
		NULL
	);
	if (hRet != DD_OK)
		pDirectDrawPal = NULL;
	
	// Free memory now in another place, see RemoveDirectDraw()
	free(pColorTable);
	
	return (pDirectDrawPal);
}

//---------------------------------------------------------
// Search last index of a character in the array of char
int lastpos(char* text, char symbol)
{
	for (int i = strlen(text) - 1; i >= 0; i--)
		if (text[i] == symbol)
			return i;
	return -1;
}
//---------------------------------------------------------
// Cut of string
char *substring(char *str, int index, int length)
{
	char *result = new char[255];
	// For a clear result
	result[0] = '\0';
		
	if (length < 0)
		return result;
	
	if (index < 0)
		return result;
	
	if (index + length > (int)strlen(str))
		return result;
	
	int j = 0;
	for (int i = index; i < index + length; i++)
	{
		result[j] = str[i];
		j++;
	}
	result[j] = '\0';
	
	return result;
}
//---------------------------------------------------------
// Preparing surfaces for displaying
BOOL PrepareSurfaces()
{
	// Creating DirectDraw palette
	pDDPal = CreateDirectDrawPaletteFromResource(pDD);
	//pDDPal = CreateDirectDrawPaletteFromFile(pDD);
	if (pDDPal == NULL)
		return (FALSE);
	
	// Assignment of a palette to primary surface
	pPrimarySurface->SetPalette(pDDPal);
	
	// Loading graphics data from the files to the offscreen surfaces
	
	//if (!LoadBMP(pPicFrame, pFileName))
	//	return (FALSE);
	
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		//char fullpath[MAX_PATH];
		
		// Get absolute path to the executable
		//GetModuleFileName(NULL, fullpath, MAX_PATH);
		
		// Вычислить позицию последнего "\"
		//int f = lastpos(fullpath, '\\');
		
		// Обрезать путь до текущего каталога
		//char* fp = new char[255];
		//fp = substring(fullpath, 0, f);
		
		// Прибавить к пути папку Bitmaps
		//strcat(fp, "\\bitmaps\\");
		
		// Прибавить к пути название картинки
		//strcat(fp, pFileNames[i]);
		
		//if (!LoadBMP(spriteCollection.sprites[i].pPicFrame, pFileNames[i]))
		//{
		//	ErrorHandle(hMainWnd, pFileNames[i]);
		//	return (FALSE);
		//}
		
		if (!LoadBMPFromResource(spriteCollection.sprites[i].pPicFrame,	101 + i))
		{
			ErrorHandle(hMainWnd, "Error loading from resource!");
			return (FALSE);
		}
		
	}
	return (TRUE);
}
//---------------------------------------------------------
// Load bitmap from BMP file
//
BOOL LoadBMP(LPDIRECTDRAWSURFACE pSurface, char* filename)
{
	// Declaration of variables
	BYTE* pBmp;
	DWORD dwBmpSize;
	DWORD dwFileLength;
	DWORD nBytesRead;
	
	BITMAPINFO* pBmpInfo;
	BYTE*		pPixels;
	
	HDC hdc;
	
	// Open file in read mode
	HANDLE hFile = CreateFile(filename, GENERIC_READ,
		FILE_SHARE_READ, NULL,OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return (FALSE);
	
	// Get sizes of data
	dwFileLength = GetFileSize (hFile, NULL) ;
	dwBmpSize = dwFileLength - sizeof(BITMAPFILEHEADER);
	
	// Allocating memory
	pBmp = (BYTE*) malloc(dwBmpSize);

	// Set file pointer to end of bitmap header (graphics data)
	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);
	
	// Read the file
	ReadFile(hFile, (LPVOID)pBmp, dwBmpSize, &nBytesRead, NULL);
	CloseHandle(hFile);
	Log("BMP size "); Log(nBytesRead); Log("\n");
	
	pBmpInfo = (BITMAPINFO*)pBmp;
	pPixels = pBmp + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;
	
	// Check surface for device context
	if ((pSurface->GetDC(&hdc)) == DD_OK)
	{
		// Copy data to device context
		StretchDIBits(hdc,
			0, 0, 64, 64,
			0, 0, 64, 64,
			pPixels, pBmpInfo, 0, SRCCOPY);
		pSurface->ReleaseDC(hdc);
	}
	
	// Free memory
	free(pBmp);
	
	return (TRUE);
}
//---------------------------------------------------------
// Load bitmap from resource
//
BOOL LoadBMPFromResource(LPDIRECTDRAWSURFACE pSurface, int resource)
{
	HBITMAP hBmp = LoadBitmap(NULL, MAKEINTRESOURCE(resource));
	//Log("hBMP size"); Log(sizeof(hBmp)); Log("\n");

	HDC hdc;
	
	// Necessary variables
	//struct {
	//	BITMAPINFOHEADER bmiHeader;
	//	RGBQUAD bmiColor[256];
	//} bmi;
	//memset(&bmi, 0, sizeof(bmi));
	
	BITMAPINFO* bmi;
	bmi = (LPBITMAPINFO) malloc(sizeof(BITMAPINFO) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	// Set size in header by size of header
	// We have 8 bit image
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	bmi->bmiHeader.biWidth = 64;
	bmi->bmiHeader.biHeight = 64;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 8;
	bmi->bmiHeader.biCompression = BI_RGB;
	// All first 6 fields should be initialized... Damn! So many hours wasted!
	//bmi->bmiHeader.biSizeImage = 4096;
	//bmi->bmiHeader.biClrUsed = 256;
	//bmi->bmiHeader.biClrImportant = 0;

	// Loading palette data from resource
	//HRSRC rc = NULL;
	//rc = FindResource(NULL, MAKEINTRESOURCE(IDR_PALETTE1), RT_RCDATA);
	//HGLOBAL hgl = LoadResource(NULL, rc);
	//BYTE *paletteBytes = (BYTE*)LockResource(hgl); // Dont forget, it contains +24 bytes of header
	//FreeResource(hgl);
	
	// Allocation memory for color table
	//LPRGBQUAD pColorTable;
	//pColorTable = (LPRGBQUAD) malloc (uMemNeed);
	
	// Copy palette data
	//memcpy(pColorTable, paletteBytes + 24, uMemNeed);

	//for (int i = 0; i < 256; i++)
	//{
	//	bmi->bmiColors[i].rgbRed =			pColorTable[i].rgbRed;
	//	bmi->bmiColors[i].rgbGreen =		pColorTable[i].rgbGreen;
	//	bmi->bmiColors[i].rgbBlue =			pColorTable[i].rgbBlue;
	//	bmi->bmiColors[i].rgbReserved =		pColorTable[i].rgbReserved;
	//}

	// Get display context
	if ((pSurface->GetDC(&hdc)) == DD_OK)
	{
		// Reading data from HBITMAP
		// Copying to surface
		int ret = GetDIBits(hdc, hBmp, 0, 64, NULL, bmi, DIB_PAL_COLORS);

		pSurface->SetPalette(pDDPal);

		pSurface->ReleaseDC(hdc);
	}
	
	// Free memory
	free(hBmp);
	
	
	return (TRUE);
}
//---------------------------------------------------------
//Check surfaces on lost
//
void PrepareFrame()
{
	//Check primary surface on lost
	if (pPrimarySurface->IsLost())
	{
		//Restore primary surface and back buffer
		pPrimarySurface->Restore();
		pBackBuffer->Restore();
		
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame->IsLost())
			{
				// Restoring offscreen surfaces
				// Filling surfaces by data
				spriteCollection.sprites[i].pPicFrame->Restore();
				//LoadBMP(spriteCollection.sprites[i].pPicFrame,
				//	pFileNames[i]);
				LoadBMPFromResource(
					spriteCollection.sprites[i].pPicFrame,
					101 + i
				);
			}
		}
	}
}
//---------------------------------------------------------
//Clearing surfaces
//
BOOL ClearSurface(LPDIRECTDRAWSURFACE pSurface)
{
	DDSURFACEDESC ddSurfaceDesc;
	HRESULT hRet;
	
	ZeroMemory(&ddSurfaceDesc,sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);
	
	hRet=pSurface->Lock(NULL,&ddSurfaceDesc,
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
	if(FAILED(hRet))
	{
		return (FALSE);
	}
	
	//UINT surfaceWidth=ddSurfaceDesc.lPitch;
	UINT surfaceWidth=ddSurfaceDesc.dwWidth;
	UINT surfaceHeight=ddSurfaceDesc.dwHeight;
	
	char *buf=(char*)ddSurfaceDesc.lpSurface;
	ZeroMemory(buf, surfaceWidth * surfaceHeight);
	pSurface->Unlock(NULL);
	return (TRUE);
}
//---------------------------------------------------------
//Output data to display
//
void DrawFrame()
{
	RECT rPic;
	
	// Prepare surfaces
	PrepareFrame();
	ClearSurface(pBackBuffer);
	
	// Moving sprites
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].x += spriteCollection.sprites[i].x1;
		spriteCollection.sprites[i].y += spriteCollection.sprites[i].y1;
		
		if (spriteCollection.sprites[i].x > MAX_WIDTH - FRAME_WIDTH || 
			spriteCollection.sprites[i].x < 0)
		{
			int r = rand()%MAX_SPEED + MIN_SPEED;
			if (spriteCollection.sprites[i].x1 > 0)
				spriteCollection.sprites[i].x1 = -r;
			else
				spriteCollection.sprites[i].x1 = r;
		}
		if (spriteCollection.sprites[i].x > MAX_WIDTH - FRAME_WIDTH)
			spriteCollection.sprites[i].x = MAX_WIDTH - FRAME_WIDTH;
		
		if (spriteCollection.sprites[i].x < 0)
			spriteCollection.sprites[i].x = 0;
		
		if (spriteCollection.sprites[i].y > MAX_HEIGHT - FRAME_HEIGHT || 
			spriteCollection.sprites[i].y < 0)
		{
			int r = rand()%MAX_SPEED + MIN_SPEED;
			if (spriteCollection.sprites[i].y1 > 0)
				spriteCollection.sprites[i].y1 = -r;
			else
				spriteCollection.sprites[i].y1 = r;
		}
		if (spriteCollection.sprites[i].y > MAX_HEIGHT - FRAME_HEIGHT)
			spriteCollection.sprites[i].y = MAX_HEIGHT - FRAME_HEIGHT;
		
		if (spriteCollection.sprites[i].y < 0)
			spriteCollection.sprites[i].y = 0;
	}
	
	// Setting rectangles for copying data
	int w = FRAME_WIDTH;
	int h = FRAME_HEIGHT;
	for (i = 0; i < MAX_SPRITES; i++) {
		// f can be 0-7
		int f = spriteCollection.sprites[i].currentFrame;
		
		// Select frame from picture
		// BMP:
		// 0 | 1
		// —————
		// 2 | 3
		// —————
		// 4 | 5
		// —————
		// 6 | 7
		
		SetRect(&rPic, w*(f%2), h*(f/2), w*(f%2+1), h*(f/2+1));
		
		// Copying data from offscreen surfaces to secondary buffer
		pBackBuffer->BltFast(
			spriteCollection.sprites[i].x,
			spriteCollection.sprites[i].y,
			spriteCollection.sprites[i].pPicFrame,
			&rPic,
			DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT
		);
		
	}
	
	// Switching surfaces
	pPrimarySurface->Flip(NULL, DDFLIP_WAIT);
}

void NextTick()
{
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].currentFrame++;
		if (spriteCollection.sprites[i].currentFrame > 
			spriteCollection.sprites[i].maxFrame)
			spriteCollection.sprites[i].currentFrame = 0;
	}
}