//------------------------------------------------------
//  File:  DRAW.CPP
//  Description: Demonstration basics of DirectDraw
//------------------------------------------------------
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <ctime>

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

char* pFileNames[] = {
	"ess1868f_Animated.bmp",
	"TVGA-9000C_2_coll.bmp",
	"3dfx_voodoo.bmp"
};

// Class for sprite
class Sprite {
public:
	LPDIRECTDRAWSURFACE pPicFrame; // Поверхность DirectDraw
	int x; // Start coordinates
	int y; //
	int x1; // Speed at axe X
	int y1; // Speed at axe Y
	int w; // Width of full picture, not a frame
	int h; // Height of full picture

	Sprite() {
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

class AnimatedSprite: public Sprite {
public:
	int framesHorizontal;
	int framesVertical;
	int currentFrame;
	int maxFrame;

	AnimatedSprite() {
		framesHorizontal = 2;
		framesVertical = 4;
		currentFrame = 0;
		maxFrame = 7;
		// 8 frames
	}
};

class SpriteCollection {
public:
	AnimatedSprite sprites[MAX_SPRITES];
};

SpriteCollection spriteCollection;

void ErrorHandle(HWND hwnd, LPCTSTR szError)
{
	char szErrorMessage[255];
	
	// Before message should correct to quit DirectDraw
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
	// NULL all interfaces
	pPrimarySurface=NULL;
	pBackBuffer=NULL;
	pDDPal=NULL;

	for (int i = 0; i < MAX_SPRITES; i++) {
		spriteCollection.sprites[i].pPicFrame = NULL; 
	}

	hMainWnd=hwnd;

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
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetDisplayMode");
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
	// Declaration of necessary structures and variables
	// for many  DirectDraw functions
	
	DDSURFACEDESC ddSurfaceDesc;
	DDSCAPS ddsCaps;
	HRESULT hRet;

	// Clearing structure from garbage and setting size
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);

	// Setting necessary fields of structure
	ddSurfaceDesc.dwFlags=DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddSurfaceDesc.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddSurfaceDesc.dwBackBufferCount=1;

	// Creating of surface
	hRet=pDD->CreateSurface(&ddSurfaceDesc,	&pPrimarySurface, NULL);
	if (hRet!=DD_OK)
		return (FALSE);

	// Creating secondary buffer
	ZeroMemory(&ddsCaps, sizeof(ddsCaps));
	ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
	hRet=pPrimarySurface->GetAttachedSurface(&ddsCaps, &pBackBuffer);
	if(hRet!=DD_OK)
		return (FALSE);

	// Creating offscreen surfaces for sprites
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
		ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);
		ddSurfaceDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddSurfaceDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
		ddSurfaceDesc.dwHeight=spriteCollection.sprites[i].h;
		ddSurfaceDesc.dwWidth=spriteCollection.sprites[i].w;
		hRet=pDD->CreateSurface(&ddSurfaceDesc, &spriteCollection.sprites[i].pPicFrame, NULL);
		if(hRet!=DD_OK)
			return (FALSE);
	}

	// Setting structure with color keys
	DDCOLORKEY ddColorKey;
	ddColorKey.dwColorSpaceLowValue=TRASPARENT_COLOR;
	ddColorKey.dwColorSpaceHighValue=TRASPARENT_COLOR;

	// Setting color keys for all surfaces
	for (i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].pPicFrame->SetColorKey(DDCKEY_SRCBLT, 
			&ddColorKey);
	}
	return (TRUE);
}
//---------------------------------------------------------
// Creating of palette
//
LPDIRECTDRAWPALETTE CreateDirectDrawPalette(LPDIRECTDRAW pDD)
{
	// Declaration of interfaces and structures 
	// for working with palette
	LPDIRECTDRAWPALETTE pDirectDrawPal;
	PALETTEENTRY palEntries[256];
	HRESULT hRet;
	LPRGBQUAD pColorTable;
	UINT uMemNeed=sizeof(RGBQUAD)*256;
	DWORD nBytesRead;

	// Opening graphics file containing palette
	// TODO Change to resource file
	HANDLE hFile=CreateFile(
		"C:\\Мои документы\\Flying_Cards_dx5_v3\\bitmaps\\ess1868f_animated.bmp", GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
 	if (hFile==INVALID_HANDLE_VALUE)
	{
		pDirectDrawPal=NULL;
		return (pDirectDrawPal);
	}

	// Allocation memory for color table
	pColorTable= (LPRGBQUAD)malloc(uMemNeed);
	
	// Setting file pointer to a start palette
	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),
		NULL, FILE_BEGIN);
	
	// Reading palette from file
	ReadFile(hFile, (LPVOID)pColorTable, uMemNeed, &nBytesRead, NULL);
	
	// Closing file
	CloseHandle(hFile);

	// Converting palette from RGBQUAD to RGBTRIPPLE
	for (int x=0;x<256;++x)
	{
		palEntries[x].peRed=pColorTable[x].rgbRed;
		palEntries[x].peBlue=pColorTable[x].rgbBlue;
		palEntries[x].peGreen=pColorTable[x].rgbGreen;
	}

	// Creating DirectDraw palette
	hRet=pDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		palEntries, &pDirectDrawPal, NULL);
	if (hRet!=DD_OK)
		pDirectDrawPal=NULL;

	// Freeing memory
	free(pColorTable);

	return (pDirectDrawPal);
}

//---------------------------------------------------------
// Search last index of a character in the array of char
int lastpos(char* text, char symbol)
{
	for (int i = strlen(text) - 1; i >= 0; i--)
	{
		if (text[i] == symbol)
		{
			return i;
		}
	}
	return -1;
}
//---------------------------------------------------------
// Cut of string
char *substring(char *str, int index, int length)
{
	char *result = new char[255];
	result[0] = '\0';
	// For clear result
	
	if (length < 0)
	{
		return result;
	}

	if (index < 0)
	{
		return result;
	}

	if (index + length > (int)strlen(str))
	{
		return result;
	}
	
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
	pDDPal=CreateDirectDrawPalette(pDD);
	if (pDDPal==NULL)
		return (FALSE);

	// Assignment of a palette to primary surface
	pPrimarySurface->SetPalette(pDDPal);

	// Loading graphics data from the files to the offscreen surfaces
	
	//if (!LoadBMP(pPicFrame, pFileName))
	//	return (FALSE);
	
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		//char fullpath[MAX_PATH];
		
		// Получить путь к исполняему файлу
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

		//if (!LoadBMP(spriteCollection.sprites[i].pPicFrame, fp))
		//{
		//	ErrorHandle(hMainWnd, fp);
		//	return (FALSE);
		//}
		if (!LoadBMPFromResource(
					spriteCollection.sprites[i].pPicFrame,
					101 + i))
		{
			ErrorHandle(hMainWnd, "Error loading from resource!");
			return (FALSE);
		}

	}
	return (TRUE);
}
//---------------------------------------------------------
//Загрузка изображения из BMP-файла
//
BOOL LoadBMP(LPDIRECTDRAWSURFACE pSurface, char* filename)
{
	//Объявление переменных, необходимых для чтения данных из BMP-файла
	BYTE* pBmp;
	DWORD dwBmpSize;
	DWORD dwFileLength;
	DWORD nBytesRead;

	BITMAPINFO* pBmpInfo;
	BYTE*		pPixels;

	HDC hdc;

	//Открытие файла с графическими данными
	HANDLE hFile=CreateFile(filename, GENERIC_READ,
			FILE_SHARE_READ, NULL,OPEN_EXISTING, 0, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{

		return (FALSE);
	}

	//Получение размера файла и размера данных
	dwFileLength=GetFileSize (hFile, NULL) ;
	dwBmpSize=dwFileLength-sizeof(BITMAPFILEHEADER);

	//Выделение памяти под данные
	pBmp=	(BYTE*) malloc(dwBmpSize);
	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);

	//Чтение файла с данными
	ReadFile(hFile, (LPVOID)pBmp, dwBmpSize, &nBytesRead, NULL);
	CloseHandle(hFile);

	pBmpInfo=(BITMAPINFO*)pBmp;
	pPixels=pBmp+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256;

	//Получение заголовка контекста устройства внеэкранной поверхности
	if ((pSurface->GetDC(&hdc)) == DD_OK)
	{
	//Копирование графических данных из памяти
	//на внеэкранную поверхность средствами GDI
		//StretchDIBits(hdc , 0, 0, FRAME_WIDTH, FRAME_HEIGHT, 0, 0, FRAME_WIDTH, FRAME_HEIGHT, pPixels, pBmpInfo, 0, SRCCOPY);
		StretchDIBits(hdc,
			0, 0, 64, 64,
			0, 0, 64, 64,
			pPixels, pBmpInfo, 0, SRCCOPY);
		pSurface->ReleaseDC(hdc);
	}
	//Освобождение памяти
	free(pBmp);

	return (TRUE);
}
//---------------------------------------------------------
//Загрузка изображения из ресурсов приложения
//
BOOL LoadBMPFromResource(LPDIRECTDRAWSURFACE pSurface, int resource)
{
	// Грузим битмап
	HBITMAP hBmp=LoadBitmap(NULL, MAKEINTRESOURCE(resource));

	HDC hdc;

	//Объявление переменных, необходимых для чтения данных
	BITMAPINFO bmi;
	
	// Пишем в заголовок структуры её же размер
	bmi.bmiHeader.biSize=sizeof(bmi.bmiHeader);
	
	// У нас 8-битное изображение
	bmi.bmiHeader.biBitCount = 8;
	
	bmi.bmiHeader.biHeight = 64;
	bmi.bmiHeader.biWidth = 64;

	// И 256 цветов
	//bmi.bmiColors = 256;

	bmi.bmiHeader.biClrUsed = 256;

	//Получение заголовка контекста устройства внеэкранной поверхности
	if ((pSurface->GetDC(&hdc)) == DD_OK)
	{
		// Здесь нужно прочитать биты из HBITMAP и
		// переписать в поверхность
		int ret=GetDIBits(hdc, hBmp, 0, 64, NULL, &bmi, DIB_PAL_COLORS);
		pSurface->ReleaseDC(hdc);
	}
	
	//Освобождение памяти
	free(hBmp);

	return (TRUE);
}
//---------------------------------------------------------
//Проверка кадра на "потерю"
//
void PrepareFrame()
{
	//Проверка на "потерю" первичной поверхности
	if (pPrimarySurface->IsLost())
	{
		//Восстановление первичной поверхности и вторичного буфера
		pPrimarySurface->Restore();
		pBackBuffer->Restore();

		//Проверка внеэкранных поверхностей на "потерю"
		//if (pPicFrame->IsLost())
		//{
			//Восстановление внеэкранных поверхностей
			//и заполнение их данными из файлов
		//	pPicFrame->Restore();
		//	LoadBMP(pPicFrame, pFileName);
		//}

		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame->IsLost())
			{
				//Восстановление внеэкранных поверхностей
				//и заполнение их данными из файлов
				spriteCollection.sprites[i].pPicFrame->Restore();
				//LoadBMP(spriteCollection.sprites[i].pPicFrame,
				//	pFileNames[i]);
				LoadBMPFromResource(
					spriteCollection.sprites[i].pPicFrame,
					101 + i);
			}
		}
	}
}
//---------------------------------------------------------
//Отчистка поверхности
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
//Вывод изображения на экран
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
			if (spriteCollection.sprites[i].x1 > 0) {
				spriteCollection.sprites[i].x1 = -r;
			}
			else {
				spriteCollection.sprites[i].x1 = r;
			}
		}
		if (spriteCollection.sprites[i].x > MAX_WIDTH - FRAME_WIDTH)
		{
			spriteCollection.sprites[i].x = MAX_WIDTH - FRAME_WIDTH;
		}
		if (spriteCollection.sprites[i].x < 0)
		{
			spriteCollection.sprites[i].x = 0;
		}

		if (spriteCollection.sprites[i].y > MAX_HEIGHT - FRAME_HEIGHT || spriteCollection.sprites[i].y < 0)	{
			
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
		SetRect(&rPic, w*(f%2), h*(f/2), w*(f%2+1), h*(f/2+1));
		
		// Copying data from offscreen surfaces to secondary buffer
		pBackBuffer->BltFast(
			spriteCollection.sprites[i].x,
			spriteCollection.sprites[i].y,
			spriteCollection.sprites[i].pPicFrame,
			&rPic,
			DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
		}
	}

	// Switching surfaces
	pPrimarySurface->Flip(NULL, DDFLIP_WAIT);
}

void NextTick() {
	for (int i = 0; i < MAX_SPRITES; i++) {
		spriteCollection.sprites[i].currentFrame++;
		if (spriteCollection.sprites[i].currentFrame > 
			spriteCollection.sprites[i].maxFrame)
			spriteCollection.sprites[i].currentFrame = 0;
	}
}