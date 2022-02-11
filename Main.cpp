// Defines

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define TRASPARENT_COLOR 0xE3

#define FRAME_HEIGHT 16
#define FRAME_WIDTH 32

#define ClassName "DX_Window"
#define AppName "Flying Cards"

//Характеристики таймера
//#define TIMER_ID	1
//#define TIMER_RATE	1 // moving
#define TIMER2_ID	2
#define TIMER2_RATE	64 // animation
//#define TIMER3_ID	3
//#define TIMER3_RATE	5000 // back color change
#define TIMERFPS_ID	4
#define TIMERFPS_RATE	1000 // fps timer

// Min and max speed
#define MAX_SPEED 2
#define MIN_SPEED 1

// Max number of sprites
#define MAX_SPRITES 3

//-------------------------------------------------------------------
// Includes

#include <windows.h>
#include <windowsx.h>

#include <ddraw.h>
#include <mmsystem.h>
#include <ctime>
#include <stdio.h>

#include "resource.h"

//-----------------------------------------------------------------------
//Classes
// Class for sprite


int ddWidth = 800; // Direct Draw width init
int ddHeight = 600; // Direct Draw height init
int ddDepth = 8; // Direct Draw bits depth init

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
	
	int xOld;
	int yOld;

	int xOld2;
	int yOld2;

	Sprite()
	{
		

		// Start position on a screen
		x = rand()%ddWidth - FRAME_WIDTH;
		y = rand()%ddHeight - FRAME_HEIGHT;
		
		// Start speeds
		x1 = 1;
		y1 = 1;
		
		// Size of a picture
		w = 64;
		h = 64;
	}

	void Restart()
	{
		x = rand()%ddWidth - FRAME_WIDTH;
		y = rand()%ddHeight - FRAME_HEIGHT;
	}

	void InverseMoveX()
	{
		int r = rand()%MAX_SPEED + MIN_SPEED;
		if (x1 > 0)
			x1 = -r;
		else
			x1 = r;
	}

	void InverseMoveY()
	{
		int r = rand()%MAX_SPEED + MIN_SPEED;
		if (y1 > 0)
			y1 = -r;
		else
			y1 = r;
	}

	void Move()
	{
		// 0 is the zero point for sprite
		// 1 is the zero point for label
		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		//x           screen 640x480           x
		//x    1...........................    x
		//x    .        label 64x16       .    x
		//x    ......0.....................    x
		//x    | 16 |. sprite 32x16 .| 16 |    x
		//x    |    |................|    |    x
		//x                                    x
		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

		int borderLeft = 16;
		int borderRight = ddWidth - 48;
		int borderBottom = 16;
		int borderTop = ddHeight - 16;

		xOld = x;
		yOld = y;

		//Increase coordinates by move values
		x += x1;
		y += y1;
		
		if (x > borderRight || x <= borderLeft)
			InverseMoveX();
		
		if (x > borderRight)
			x = borderRight;
		
		if (x <= borderLeft)
			x = borderLeft;
		
		if (y > borderTop || 
			y <= borderBottom)
			InverseMoveY();

		if (y > borderTop)
			y = borderTop;
		
		if (y <= borderBottom)
			y = borderBottom;
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
		currentFrame = rand()%8;
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


//----------------------------------------------------------------------
//Prototypes

BOOL InitDirectDraw (HWND hwnd, int width, int height, int depth);
void RemoveDirectDraw (void);

void DrawFrame(void);
void NextTick( void );
void MoveSprites(void);
void ChangeColor(void);
void ChangeBackColor(void);
void ReInitDirectDraw(int w, int h, int d);
BOOL FillSurface(LPDIRECTDRAWSURFACE pSurface, int color);

//Обработчики сообщений Windows
BOOL DX_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void DX_OnDestroy(HWND hwnd);
void DX_OnTimer(HWND hwnd, UINT id);
void DX_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void DX_OnMouse(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DX_OnIdle(HWND hwnd);

//Оконная процедура
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//HICON invisibleCursor;

BOOL CreateSurfaces();
BOOL PrepareSurfaces();
BOOL ClearSurface(LPDIRECTDRAWSURFACE pSurface);
BOOL LoadBMP(LPDIRECTDRAWSURFACE pSurface, char* filename);
BOOL LoadBMPFromResource(LPDIRECTDRAWSURFACE pSurface, int resource);
void ErrorHandle(HWND hwnd, LPCTSTR szError);
void DrawNumber(int n, int x3, int y3);
void MoveSprite(Sprite sprite);

//--------------------------------------------------------------------
// Global variables

bool info = false;

int frame_counter = 0;
int fpsCollector;
int fps;

//Флаг активности нашего приложения
BOOL	bActive;

int mouse_x = 0;
int mouse_y = 0;
int mouseCheck = 0;

//int ddWidth = 800; // Direct Draw width init
//int ddHeight = 600; // Direct Draw height init
//int ddDepth = 8; // Direct Draw bits depth init

//103 debug picture
//101 ess audiodrive 1869
//102 3dfx voodoo
//104 tvga 9000C
int idsResourcesForSprites[MAX_SPRITES] =
{ 101, 102, 104 };

//Use only for debug log
bool debugLog = true;

static HWND hMainWnd;

LPDIRECTDRAW pDD;

LPDIRECTDRAWSURFACE pPrimarySurface;
LPDIRECTDRAWSURFACE pBackBuffer;
LPDIRECTDRAWSURFACE pBackground;
LPDIRECTDRAWSURFACE labelSurface;
LPDIRECTDRAWSURFACE numbersSurface;

LPDIRECTDRAWPALETTE pDDPal;

char* pFileNames[] = 
{
	"ess1868f_Animated.bmp",
	"TVGA-9000C_2_coll.bmp",
	"3dfx_voodoo.bmp",
	"test.bmp"
};

int rPoses[] = {
	 0, 0,32,16,
	32, 0,64,16,
	 0,16,32,32,
	32,16,64,32,
	 0,32,32,48,
	32,32,64,48,
	 0,48,32,64,
	32,48,64,64
};

int labelPoses[] = {
	 0, 0,64,16,
	 0,16,64,32,
	 0,32,64,48,
	 0,48,64,64,
};

//
// 224 Palette R 255 G 0 B 0
// 28 Palette R 0 G 255 B 0
// 3 Palette R 0 G 0 B 255
// 2 Palette R 0 G 0 B 128
// 16 Palette R 0 G 128 B 0
// 128 Palette R 128 G 0 B 0
// 146 Palette R 128 G 128 B 128
// 73 Palette R 64 G 64 B 64
//int backColors[] = {3, 28, 224, 0, 2, 16, 128};
int backColors[] = {0, 128, 16, 2, 224, 28, 3, 128};
int backColor = 0;
//--------------------------------------------------------------------
// Main function

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wndClass;
	HWND hWnd;
	MSG msg;

	//BYTE CursorMaskAND[] = { 0xFF };
	//BYTE CursorMaskXOR[] = { 0x00 };
	//invisibleCursor = CreateCursor(NULL, 0,0,1,1, CursorMaskAND, CursorMaskXOR);
	
	//bool dropOut = true;
	//bool dropOut = false;

	// Обработка аргументов командной строки
	//char par1[80];
	//if (strlen(lpszCmdLine) >= 2)
	//{
		// Копируем два первых символа командной строки
	//	strncpy(par1, lpszCmdLine, 3);
		
		// Если программа запущена с этими параметрами, игнорировать
	//	if (strcmp(par1,"/p") == 0 || strcmp(par1,"/P") == 0)
	//		dropOut = true;
	//	if (strcmp(par1,"/c") == 0 || strcmp(par1,"/C") == 0)
	//		dropOut = true;
	//	if (strcmp(par1, "/s") == 0 || strcmp(par1, "/S") == 0)
	//		dropOut = false;
	//}
	//if (dropOut)
	//	return (msg.wParam);

	//Регистрация оконного класса
	wndClass.cbSize       =sizeof(wndClass);
	wndClass.style        =CS_HREDRAW|CS_VREDRAW;
	wndClass.lpfnWndProc  =WndProc;
	wndClass.cbClsExtra   =0;
	wndClass.cbWndExtra   =0;
	wndClass.hInstance    =hInst;
	wndClass.hIcon        =LoadIcon(NULL,IDI_WINLOGO);
	wndClass.hCursor      =LoadCursor(NULL,IDC_ARROW);
	wndClass.hbrBackground=NULL;//Обратите внимание!!!
	wndClass.lpszMenuName =NULL;
	wndClass.lpszClassName=ClassName;
	wndClass.hIconSm      =LoadIcon(NULL,IDI_WINLOGO);

	RegisterClassEx(&wndClass);

	//Создание окна на основе класса
	hWnd=CreateWindowEx(
		0,//WS_EX_TOPMOST,//Дополнительный стиль окна
		ClassName,	//Класс окна
		AppName,	//Текст заголовка
		WS_POPUP,	//Стиль окна
		0,0,		//Координаты X и Y
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),//Ширина и высота
		NULL,		//Дескриптор родительского окна
		NULL,		//Дескриптор меню
		hInst,		//Описатель экземпляра
		NULL);		//Дополнительные данные

	// Скрываем курсор
	ShowCursor(FALSE);

	DeleteFile("debug.log");

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	//Инициализация компонентов, связанных с DirectDraw
	if (!InitDirectDraw(hWnd, 800, 600, 8))
		return FALSE;

	//Главный цикл программы
	while (TRUE)
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if (msg.message==WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (bActive)//Только если приложение активно!
			{
				DX_OnIdle(hWnd);
			}
		}
	}
	return (msg.wParam);
}


//---------------------------------------------------------
//	Оконная процедура
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLE_MSG(hWnd, WM_CREATE, DX_OnCreate);
		HANDLE_MSG(hWnd, WM_DESTROY, DX_OnDestroy);
		HANDLE_MSG(hWnd, WM_TIMER, DX_OnTimer);
		HANDLE_MSG(hWnd, WM_ACTIVATE, DX_OnActivate);
		HANDLE_MSG(hWnd, WM_KEYDOWN, DX_OnKey);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, DX_OnMouse);
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

		/* Обработчики сообщений */
//---------------------------------------------------------

BOOL DX_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
//Установка таймера
	//if (!SetTimer(hwnd,TIMER_ID,TIMER_RATE,NULL))
	//	return (FALSE);
	if (!SetTimer(hwnd,TIMER2_ID,TIMER2_RATE,NULL))
		return (FALSE);
	//if (!SetTimer(hwnd,TIMER3_ID,TIMER3_RATE,NULL))
	//	return (FALSE);
	if (!SetTimer(hwnd,TIMERFPS_ID,TIMERFPS_RATE,NULL))
		return (FALSE);
	return (TRUE);
}

//---------------------------------------------------------
void DX_OnDestroy(HWND hwnd)
{
	//Убрать после себя
	//KillTimer(hwnd,TIMER_ID);
	KillTimer(hwnd,TIMER2_ID);
	//KillTimer(hwnd,TIMER3_ID);
	KillTimer(hwnd,TIMERFPS_ID);
	RemoveDirectDraw();
	PostQuitMessage(0);
}
//---------------------------------------------------------
void DX_OnTimer(HWND hwnd, UINT id)
{
	// Moving code
	//if (id == TIMER_ID)
		//if (bActive) 
		//	MoveSprites();
	
	if (id == TIMER2_ID)
		if (bActive)
		{
			NextTick();
			//DrawFrame();
		}
	// 
	//if (id == TIMER3_ID)
	//	if (bActive)
	//		ChangeColor();
	
	if (id == TIMERFPS_ID)
	{
		fps = fpsCollector;
		fpsCollector = 0;
	}
}
//---------------------------------------------------------

void DX_OnIdle(HWND hwnd)
{
	//Прорисовка кадра
	//MoveSprites();
	
	DrawFrame();
}
//---------------------------------------------------------
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	//При нажатии пробела прекратить работу программы
	if (vk==VK_SPACE || vk==VK_ESCAPE)
		DestroyWindow(hwnd);
	if (vk==0x42) // If press button B, change background
	{
		ChangeBackColor();
	}
	if (vk==0x31) // If press 1, 320 200 8
	{
		ddWidth = 320;
		ddHeight = 200;
		ddDepth = 8;
		
		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
	if (vk==0x32) // If press 2, 320 240 8
	{
		ddWidth = 320;
		ddHeight = 240;
		ddDepth = 8;
		
		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
	if (vk==0x33) // If press 3, 400 300 8
	{
		ddWidth = 400;
		ddHeight = 300;
		ddDepth = 8;

		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
	if (vk==0x34) // If press 4, 640 400 8
	{
		ddWidth = 640;
		ddHeight = 400;
		ddDepth = 8;

		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
	if (vk==0x35) // If press 5, 640 480 8
	{
		ddWidth = 640;
		ddHeight = 480;
		ddDepth = 8;

		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
	if (vk==0x36) // If press 6, 800 600 8
	{
		ddWidth = 800;
		ddHeight = 600;
		ddDepth = 8;

		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
	if (vk==0x37) // If press 7, 1024 768 8
	{
		ddWidth = 1024;
		ddHeight = 768;
		ddDepth = 8;

		RemoveDirectDraw();
		ReInitDirectDraw(ddWidth, ddHeight, ddDepth);

		if (!InitDirectDraw(hwnd, ddWidth, ddHeight, ddDepth))
			DestroyWindow(hwnd);
	}
}
//---------------------------------------------------------
void DX_OnMouse(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//При движении мыши прекратить работу программы
	int resx = mouse_x - GET_X_LPARAM(lParam);
	if (resx < 0) resx *= -1;
	
	int resy = mouse_y - GET_Y_LPARAM(lParam);
	if (resy < 0) resy *= -1;
	
	mouse_x = GET_X_LPARAM(lParam);
	mouse_y = GET_Y_LPARAM(lParam);
	
	if (mouseCheck > 0)
		if (resx > 1 || resy > 1)
			DestroyWindow(hwnd);
	
	mouseCheck++;
}
//---------------------------------------------------------
void DX_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	//Обновить флаг состояния активности
	if (state==WA_INACTIVE)
		bActive=FALSE;
	else
		bActive=TRUE;
}

//-------------------------------------------------------------











void ReInitDirectDraw(int w, int h, int d)
{
	ddWidth = w;
	ddHeight = h;
	ddDepth = d;
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

void Log(int v)
{
	if (debugLog)
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
}

void Log(char* dbg)
{
#ifdef _DEBUG
	if (debugLog)
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
#endif
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

		// Check background surface
		if (pBackground!=NULL)
		{
			// Releasing of primary surface
			pBackground->Release();
			pBackground = NULL;
		}


		// Remove label surface
		if (labelSurface!=NULL)
		{
			labelSurface->Release();
			labelSurface = NULL;
		}

		// Remove numbers surface
		if (numbersSurface!=NULL)
		{
			numbersSurface->Release();
			numbersSurface = NULL;
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
BOOL InitDirectDraw (HWND hwnd, int width, int height, int depth)
{
	Log("InitDirectDraw"); Log("\n");

	// NULL all interfaces
	pPrimarySurface = NULL;
	pBackBuffer = NULL;
	pBackground = NULL;
	labelSurface = NULL;
	numbersSurface = NULL;

	pDDPal = NULL;
	
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].pPicFrame = NULL; 
	}
	
	hMainWnd = hwnd;
	
	// Variable to return error codes
	HRESULT hRet;
	
	// Creating IDirectDraw interface
	Log("DirectDrawCreate start\n");
	hRet=DirectDrawCreate(NULL, &pDD, NULL);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd, "DirectDrawCreate");
		return (FALSE);
	}
	Log("DirectDrawCreate success\n");
	
	// Set exclusive fullscreen mode
	hRet=pDD->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetCooperativeLevel");
		return (FALSE);
	}
	Log("SetCooperativeLevel success\n");
	
	// Set display mode
	hRet=pDD->SetDisplayMode(width, height, depth);
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
	
	for (i = 0; i < MAX_SPRITES; i++)
	{
		// Damn && link to object
		Sprite& sprite = spriteCollection.sprites[i];
		sprite.Restart();
	}

	FillSurface(pBackground, backColor);
	FillSurface(pPrimarySurface, backColor);
	FillSurface(pBackBuffer, backColor);

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
	
	// Setting structure with color keys
	DDCOLORKEY ddColorKey;
	ddColorKey.dwColorSpaceLowValue = TRASPARENT_COLOR;
	ddColorKey.dwColorSpaceHighValue = TRASPARENT_COLOR;

	//Create surface for background
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
	ddSurfaceDesc.dwFlags = 
		DDSD_CAPS | 
		DDSD_HEIGHT | 
		DDSD_WIDTH;
	ddSurfaceDesc.ddsCaps.dwCaps = 
		DDSCAPS_OFFSCREENPLAIN;
	ddSurfaceDesc.dwHeight = ddHeight;
	ddSurfaceDesc.dwWidth = ddWidth;
	hRet=pDD->CreateSurface(
		&ddSurfaceDesc,
		&pBackground,
		NULL);
	if (hRet != DD_OK) return (FALSE);
	//pBackground->SetColorKey(DDCKEY_SRCBLT, &ddColorKey);
	//pBackground->SetPalette(pDDPal);

//Create surface for numbers
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
	ddSurfaceDesc.dwFlags = 
		DDSD_CAPS | 
		DDSD_HEIGHT | 
		DDSD_WIDTH;
	ddSurfaceDesc.ddsCaps.dwCaps = 
		DDSCAPS_OFFSCREENPLAIN;
	ddSurfaceDesc.dwHeight = 64;
	ddSurfaceDesc.dwWidth = 64;
	hRet=pDD->CreateSurface(
		&ddSurfaceDesc,
		&numbersSurface,
		NULL);
	if (hRet != DD_OK) return (FALSE);
	numbersSurface->SetColorKey(DDCKEY_SRCBLT, &ddColorKey);

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

		//Create labels surface
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
	ddSurfaceDesc.dwFlags = 
		DDSD_CAPS | 
		DDSD_HEIGHT | 
		DDSD_WIDTH;
	ddSurfaceDesc.ddsCaps.dwCaps = 
		DDSCAPS_OFFSCREENPLAIN;
	ddSurfaceDesc.dwHeight = 64;
	ddSurfaceDesc.dwWidth = 64;
	hRet=pDD->CreateSurface(
		&ddSurfaceDesc,
		&labelSurface,
		NULL);
	if (hRet != DD_OK) return (FALSE);
	labelSurface->SetColorKey(DDCKEY_SRCBLT, &ddColorKey);
	
	// Setting color keys for all surfaces
	for (i = 0; i < MAX_SPRITES; i++)
		spriteCollection.sprites[i].pPicFrame->SetColorKey(
			DDCKEY_SRCBLT, 
			&ddColorKey);
	return (TRUE);
}
//---------------------------------------------------------
// Creating of the palette from the palette file in resources
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
	UINT uMemNeed = sizeof(RGBQUAD) * 256;
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
	memcpy(pColorTable, paletteBytes + sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER), uMemNeed);
	
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
// Creating of the palette from the BMP file in resources
//
LPDIRECTDRAWPALETTE CreateDirectDrawPaletteFromResource(
	LPDIRECTDRAW pDD, const int resource)
{
	Log("CreatePaletteFromBMPResource"); Log("\n");

	// Declaration of interfaces and structures 
	// for working with palette
	LPDIRECTDRAWPALETTE pDirectDrawPal;
	PALETTEENTRY pe[256];
	HRESULT hRet;
	
	RGBQUAD Palette[256];
	BYTE *lpBMP;

	// Very hard to find how correctly to load binary resource!
	// Example here
	
	HRSRC rc = NULL;
	rc = FindResource(NULL, MAKEINTRESOURCE(resource), RT_BITMAP);
	if (rc == NULL)
	{
		Log("CreatePalette FindResource error\n");
		return NULL;
	}
	
	Log("CreatePalette FindResource success\n");

	HGLOBAL hgl = LoadResource(NULL, rc);
	lpBMP = (BYTE*)LockResource(hgl);
	memcpy(Palette, &lpBMP[sizeof(BITMAPINFOHEADER)], sizeof(Palette));
	
	Log("CreatePalette LoadResource success\n");
	
	int i;
	for (i = 0; i < 256; i++)
    {
        pe[i].peRed = Palette[i].rgbRed;
        pe[i].peGreen = Palette[i].rgbGreen;
        pe[i].peBlue = Palette[i].rgbBlue;
    }

	//for (i = 0; i < 256; i++)
	//{
	//	Log(i);
	//	Log(" Palette R ");Log(Palette[i].rgbRed);
	//	Log(" G ");Log(Palette[i].rgbGreen);
	//	Log(" B ");Log(Palette[i].rgbBlue);
	//	Log("\n");
	//}
	
	// Creating DirectDraw palette
	hRet = pDD->CreatePalette(
		//DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		DDPCAPS_8BIT,
		pe,
		&pDirectDrawPal,
		NULL
	);

	

	if (hRet != DD_OK)
	{
		pDirectDrawPal = NULL;
		ErrorHandle(hMainWnd, "CreatePalette error");
		Log("CreatePalette CreatePalette error\n");
		return NULL;
	}
	

	Log("CreatePalette CreatePalette success\n");

	
	// Free memory
	FreeResource(hgl);

	Log("FreeResource(hgl); success\n");

	// Why this collapses to heap damage?
	//free(Palette);

	Log("CreatePaletteFromBMPResource end\n");

	return (pDirectDrawPal);
}
//---------------------------------------------------------
// Creating of a palette from the BMP file
//
LPDIRECTDRAWPALETTE CreateDirectDrawPaletteFromFile(LPDIRECTDRAW pDD, char *filename)
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
		filename,
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
		sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER),
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
	HDC hdc;

	// Loading bitmap
	HBITMAP hBmp = LoadBitmap(NULL, MAKEINTRESOURCE(resource));
	//Log("hBMP size"); Log(sizeof(hBmp)); Log("\n");

	// Necessary variables
	
	BITMAPINFO* bmi;
	bmi = (LPBITMAPINFO) malloc(sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
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

	// Get display context
	if ((pSurface->GetDC(&hdc)) == DD_OK)
	{
		// Reading data from HBITMAP
		// Copying to surface
		int ret = GetDIBits(hdc, hBmp, 0, 64, NULL, bmi, DIB_PAL_COLORS);

		//LPDIRECTDRAWPALETTE pDirectDrawPal = CreateDirectDrawPaletteFromResource(
		//	pDD, IDB_BITMAP1);
		
		//pSurface->SetPalette(pDirectDrawPal);
		//pSurface->SetPalette(pDDPal);

		pSurface->ReleaseDC(hdc);
	}
	
	// Free memory
	//free(hBmp);
	DeleteObject(hBmp);
		
	return (TRUE);
}
//---------------------------------------------------------
// Load bitmap from resource
// 
BOOL LoadBMPFromResource_V2(LPDIRECTDRAWSURFACE pSurface, int resource)
{
	HRESULT             ddrval;
    HRSRC               hBMP;
    RGBQUAD             Palette[256];
    PALETTEENTRY        pe[256];
    DDSURFACEDESC       DDSDesc;
    LPSTR               lpBits;
    LPSTR               lpSrc;
    BYTE                *lpBMP;
    int                 i;

    hBMP=FindResource(NULL,MAKEINTRESOURCE(resource),RT_BITMAP);    
    if( hBMP == NULL )
    {
        return FALSE;
    }

    lpBMP=(BYTE *)LockResource(LoadResource(NULL, hBMP));
    
    memcpy(Palette,&lpBMP[sizeof(BITMAPINFOHEADER)],sizeof(Palette));

    FreeResource(hBMP);

    for(i=0;i<256;i++)
    {
        pe[i].peRed=Palette[i].rgbRed;
        pe[i].peGreen=Palette[i].rgbGreen;
        pe[i].peBlue=Palette[i].rgbBlue;
    }   

	LPDIRECTDRAWPALETTE lpDDPal = NULL;
	if (lpDDPal == NULL) {

		ddrval=pDD->CreatePalette(DDPCAPS_8BIT, pe, &lpDDPal, NULL);

		if(ddrval!=DD_OK)
		{
			return FALSE;
		}
    }

    //pSurface->SetPalette(pDDPal);
	pSurface->SetPalette(lpDDPal);

    DDSDesc.dwSize = sizeof(DDSDesc);
	DDSDesc.lPitch = 64;
	DDSDesc.dwHeight = 64;
	DDSDesc.dwWidth = 64;
	
    ddrval = pSurface->Lock(NULL, &DDSDesc, 0, NULL);
    if(ddrval != DD_OK)
    {
        return FALSE;
    }

    lpBits = (LPSTR)DDSDesc.lpSurface;
	//unsigned int memneed = sizeof(BITMAPINFOHEADER) - 128 + sizeof(Palette) + (64*64);
	unsigned int memneed = 
		sizeof(BITMAPINFOHEADER)
		+ sizeof(Palette)
		- DDSDesc.dwWidth
		+ (64 * 64);
	
    lpSrc = (LPSTR)(&lpBMP[memneed]);
	Log("sizeof(Palette) = "); Log(sizeof(Palette)); Log("\n");
	Log("sizeof(BITMAPFILEHEADER) = "); Log(sizeof(BITMAPFILEHEADER)); Log("\n");
	Log("sizeof(BITMAPINFOHEADER) = "); Log(sizeof(BITMAPINFOHEADER)); Log("\n");
	Log("sizeof(BITMAPINFO) = "); Log(sizeof(BITMAPINFO)); Log("\n");
	Log("Memory for sprite "); Log(memneed); Log(" allocated\n");
	for (i = 0; i < 64; i++)
    {
		memcpy(lpBits, lpSrc, 64);
        lpBits += DDSDesc.lPitch;
        lpSrc -= 64;
    }

    pSurface->Unlock(NULL);

    return TRUE;
}
//---------------------------------------------------------
// Preparing surfaces for displaying
BOOL PrepareSurfaces()
{
	// Creating DirectDraw palette
	//pDDPal = CreateDirectDrawPaletteFromResource(pDD);
	//pDDPal = CreateDirectDrawPaletteFromFile(pDD);
	pDDPal = CreateDirectDrawPaletteFromResource(pDD, IDB_BITMAP1);
	if (pDDPal == NULL) return (FALSE);
	
	// Assignment of a palette to primary surface
	pPrimarySurface->SetPalette(pDDPal);
	
	// Loading graphics data from the files to the offscreen surfaces
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
		
		/*
		if (!LoadBMP(spriteCollection.sprites[i].pPicFrame, pFileNames[i]))
		{
			ErrorHandle(hMainWnd, pFileNames[i]);
			return (FALSE);
		}
		*/
		
		
		//if (!LoadBMPFromResource_V2(spriteCollection.sprites[i].pPicFrame,	101 + i))
		if (!LoadBMPFromResource_V2(spriteCollection.sprites[i].pPicFrame,
			idsResourcesForSprites[i]))
		{
			ErrorHandle(hMainWnd, "Error loading from resource!");
			return (FALSE);
		}

		//Loading label bitmap
		if (!LoadBMPFromResource_V2(labelSurface,IDB_BITMAP5))
		{
			ErrorHandle(hMainWnd, "Error loading from resource!");
			return (FALSE);
		}

		//Loading numbers bitmap
		if (!LoadBMPFromResource_V2(numbersSurface,IDB_BITMAP6))
		{
			ErrorHandle(hMainWnd, "Error loading from resource!");
			return (FALSE);
		}
	}
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
		pBackground->Restore();
		
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame->IsLost())
			{
				// Restoring offscreen surfaces
				// Filling surfaces by data
				spriteCollection.sprites[i].pPicFrame->Restore();
				//LoadBMP(spriteCollection.sprites[i].pPicFrame,
				//	pFileNames[i]);
				LoadBMPFromResource_V2(
					spriteCollection.sprites[i].pPicFrame,
					idsResourcesForSprites[i]
				);
			}
		}

		if (labelSurface->IsLost())
		{
			labelSurface->Restore();
			LoadBMPFromResource_V2(labelSurface, IDB_BITMAP5);
		}

		if (numbersSurface->IsLost())
		{
			numbersSurface->Restore();
			LoadBMPFromResource_V2(numbersSurface, IDB_BITMAP6);
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
	
	UINT surfaceWidth=ddSurfaceDesc.lPitch;
	//UINT surfaceWidth=ddSurfaceDesc.dwWidth;
	UINT surfaceHeight=ddSurfaceDesc.dwHeight;
	
	char *buf = (char*)ddSurfaceDesc.lpSurface;
	ZeroMemory(buf, surfaceWidth * surfaceHeight);
	pSurface->Unlock(NULL);
	return (TRUE);
}
//---------------------------------------------------------
//Output data to display

//---------------------------------------------------------
//Fill surfaces by color
//
BOOL FillSurface(LPDIRECTDRAWSURFACE pSurface, int color)
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
	
	UINT surfaceWidth=ddSurfaceDesc.lPitch;
	//UINT surfaceWidth=ddSurfaceDesc.dwWidth;
	UINT surfaceHeight=ddSurfaceDesc.dwHeight;
	unsigned int surf2 = surfaceWidth * surfaceHeight;
	char *buf=(char*)ddSurfaceDesc.lpSurface;
	ZeroMemory(buf, surf2);
	unsigned int i;
	for (i = 0; i < surf2; i++)
	{
		buf[i] = backColors[color];
		if (color != 0 && color != 7)
		{
			//Borders
			if (i%surfaceWidth==0|| //Left
				i%surfaceWidth==surfaceWidth-1|| //Right
				i<surfaceWidth|| //Top
				i>surfaceWidth*surfaceHeight-surfaceWidth) //Bottom
					buf[i]=(char)255;
		}
		else if (color == 7)
		{
			buf[i]=0;
			if (i%2==0)
				buf[i]=(char)146;
			//Borders
			if (i%surfaceWidth==0|| //Left
				i%surfaceWidth==surfaceWidth-1|| //Right
				i<surfaceWidth|| //Top
				i>surfaceWidth*surfaceHeight-surfaceWidth) //Bottom
					buf[i]=(char)255;
		}
		else if (color == 0)
		{
			buf[i]=0;
			
		}
	}
	pSurface->Unlock(NULL);
	return (TRUE);
}
//---------------------------------------------------------



void ChangeBackColor()
{
	backColor++;
	if (backColor > 7) backColor = 0;

	if (backColor == 0)
		info = false;
	else
		info = true;

	FillSurface(pBackground, backColor);
	FillSurface(pPrimarySurface, backColor);
	FillSurface(pBackBuffer, backColor);
}

void DrawFrame()
{
	frame_counter++;
	fpsCollector++;

	RECT rPic;
	RECT labelRect;
	
	// Prepare surfaces
	//PrepareFrame();
	//ClearSurface(pBackBuffer);
	//FillSurface(pBackBuffer, backColor);
	

	//RECT rClear;
	//SetRect(&rClear,0,0,ddWidth,ddHeight);
	//pBackBuffer->BltFast(0,0,pBackground,&rClear,	
	//	DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);

	//MoveSprites();

	// Setting rectangles for copying data
	int w = FRAME_WIDTH;
	int h = FRAME_HEIGHT;
	int i = 0;

	// Clear previous draws
	for (i = 0; i < MAX_SPRITES; i++)
	{
		// Damn && link to object
		Sprite& sprite = spriteCollection.sprites[i];

		RECT rClear;
		SetRect(&rClear,
			sprite.xOld-16,
			sprite.yOld-16,
			sprite.xOld+48,
			sprite.yOld+16);
		pBackBuffer->BltFast(
			sprite.xOld-16,
			sprite.yOld-16,
			pBackground,
			&rClear,	
			DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
	}

	for (i = 0; i < MAX_SPRITES; i++)
	{
		// Damn && link to object
		Sprite& sprite = spriteCollection.sprites[i];
		
		sprite.Move();

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
		
		//Left-top pos
		int x1 = rPoses[f*4 + 0];
		int y1 = rPoses[f*4 + 1];

		//Right-bottom pos
		int x2 = rPoses[f*4 + 2];
		int y2 = rPoses[f*4 + 3];

		SetRect(&rPic, x1, y1, x2, y2);
		



		// Copying data from offscreen surfaces to secondary buffer
		pBackBuffer->BltFast(
			spriteCollection.sprites[i].x,
			spriteCollection.sprites[i].y,
			spriteCollection.sprites[i].pPicFrame,
			&rPic,
			DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT
		);
		
		//Draw labels
		//Left-top pos
		if (idsResourcesForSprites[i] == 101)
			f = 1; // 0 1 2
		if (idsResourcesForSprites[i] == 102)
			f = 0; // 0 1 2
		if (idsResourcesForSprites[i] == 104)
			f = 2; // 0 1 2
		
		//0 3dfx Voodoo
		//1 ESS AudioDrive 1869
		//2 TVGA 9000C
		//101 ess audiodrive 1869
		//102 3dfx voodoo
		//104 tvga 9000C
		//int idsResourcesForSprites[MAX_SPRITES] = { 101, 102, 104, 101, 102, 104, 101, 102, 104, 101 };

		x1 = labelPoses[f*4 + 0];
		y1 = labelPoses[f*4 + 1];

		//Right-bottom pos
		x2 = labelPoses[f*4 + 2];
		y2 = labelPoses[f*4 + 3];

		SetRect(&labelRect, x1, y1, x2, y2);




		// Copy label to backbuffer
		pBackBuffer->BltFast(
			spriteCollection.sprites[i].x - 16,
			spriteCollection.sprites[i].y - 16,
			labelSurface,
			&labelRect,
			DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT
		);

		
		
	}

	if (info)
	{
		// Draw FPS
		DrawNumber(fps, ddWidth - 9 * 8, ddHeight - 12 * 4);

		// How many frames drawn
		DrawNumber(frame_counter, ddWidth - 9 * 8, ddHeight - 12 * 3);

		// Write resolution in right bottom corner
		DrawNumber(ddWidth, ddWidth - 9 * 8, ddHeight - 12 * 2);
		DrawNumber(ddHeight, ddWidth - 5 * 8, ddHeight - 12 * 2);
	}
	
	// Switching surfaces
	pPrimarySurface->Flip(NULL, DDFLIP_WAIT);
}

void DrawNumber(int n, int x3, int y3)
{
	int nRect[] = {
		0, 0, 7, 12, //0
		7, 0, 14, 12, //1
		14, 0, 21, 12, //2
		21, 0, 28, 12, //3
		28, 0, 35, 12, //4
		0, 11, 7, 24, //5
		7, 11, 14, 24, //6
		14, 11, 21, 24, //7
		21, 11, 28, 24, //8
		28, 11, 35, 24 //9
	};

	char text[8];
	sprintf(text, "%d", n);

	for (int i = 0; i < 8; i++)
	{
		// Clear
		RECT cRect;
		SetRect(
			&cRect,
			x3 + i * 8,
			y3,
			x3 + i * 8 + 8,
			y3 + 12);
		pBackBuffer->BltFast( // Where to draw
			x3 + i * 8,
			y3,
			pBackground,
			&cRect,
			DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT
		);

		//char '0' equal 48
		int c = (int)text[i]-48;

		RECT numberRect;
		SetRect(
			&numberRect,
			nRect[c*4],
			nRect[c*4+1],
			nRect[c*4+2],
			nRect[c*4+3]
		); // Select number
	
		

		// Copy number
		pBackBuffer->BltFast( // Where to draw
			x3 + i * 8,
			y3,
			numbersSurface,
			&numberRect,
			DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT
		);
	}
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

void ChangeColor()
{
	backColor = backColors[rand()%4];
}

void InverseMoveX(int i)
{
	int r = rand()%MAX_SPEED + MIN_SPEED;
	if (spriteCollection.sprites[i].x1 > 0)
		spriteCollection.sprites[i].x1 = -r;
	else
		spriteCollection.sprites[i].x1 = r;
}

void InverseMoveY(int i)
{
	int r = rand()%MAX_SPEED + MIN_SPEED;
	if (spriteCollection.sprites[i].y1 > 0)
		spriteCollection.sprites[i].y1 = -r;
	else
		spriteCollection.sprites[i].y1 = r;
}

void InverseMoveX(Sprite sprite)
{
	int r = rand()%MAX_SPEED + MIN_SPEED;
	if (sprite.x1 > 0)
		sprite.x1 = -r;
	else
		sprite.x1 = r;
}

void InverseMoveY(Sprite sprite)
{
	int r = rand()%MAX_SPEED + MIN_SPEED;
	if (sprite.y1 > 0)
		sprite.y1 = -r;
	else
		sprite.y1 = r;
}

// Moving sprites
void MoveSprites()
{
	// 0 is the zero point for sprite
	// 1 is the zero point for label
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	//x           screen 640x480           x
	//x    1...........................    x
	//x    .        label 64x16       .    x
	//x    ......0.....................    x
	//x    | 16 |. sprite 32x16 .| 16 |    x
	//x    |    |................|    |    x
	//x                                    x
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	int borderLeft = 16;
	int borderRight = ddWidth - 48;
	int borderBottom = 16;
	int borderTop = ddHeight - 16;

	for (int i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].xOld = spriteCollection.sprites[i].x;
		spriteCollection.sprites[i].yOld = spriteCollection.sprites[i].y;

		//Increase coordinates by move values
		spriteCollection.sprites[i].x += spriteCollection.sprites[i].x1;
		spriteCollection.sprites[i].y += spriteCollection.sprites[i].y1;
		
		if (spriteCollection.sprites[i].x > borderRight || 
			spriteCollection.sprites[i].x <= borderLeft)
		{
			InverseMoveX(i);
		}
		if (spriteCollection.sprites[i].x > borderRight)
			spriteCollection.sprites[i].x = borderRight;
		
		if (spriteCollection.sprites[i].x <= borderLeft)
			spriteCollection.sprites[i].x = borderLeft;
		
		if (spriteCollection.sprites[i].y > borderTop || 
			spriteCollection.sprites[i].y <= borderBottom)
		{
			InverseMoveY(i);
		}

		if (spriteCollection.sprites[i].y > borderTop)
			spriteCollection.sprites[i].y = borderTop;
		
		if (spriteCollection.sprites[i].y <= borderBottom)
			spriteCollection.sprites[i].y = borderBottom;
	}
}

// Moving sprites
void MoveSprite(Sprite sprite)
{
	// 0 is the zero point for sprite
	// 1 is the zero point for label
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	//x           screen 640x480           x
	//x    1...........................    x
	//x    .        label 64x16       .    x
	//x    ......0.....................    x
	//x    | 16 |. sprite 32x16 .| 16 |    x
	//x    |    |................|    |    x
	//x                                    x
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	int borderLeft = 16;
	int borderRight = ddWidth - 48;
	int borderBottom = 16;
	int borderTop = ddHeight - 16;

	sprite.xOld = sprite.x;
	sprite.yOld = sprite.y;

	//Increase coordinates by move values
	sprite.x += sprite.x1;
	sprite.y += sprite.y1;
	
	if (sprite.x > borderRight || sprite.x <= borderLeft)
		InverseMoveX(sprite);
	
	if (sprite.x > borderRight)
		sprite.x = borderRight;
	
	if (sprite.x <= borderLeft)
		sprite.x = borderLeft;
	
	if (sprite.y > borderTop || 
		sprite.y <= borderBottom)
		InverseMoveY(sprite);

	if (sprite.y > borderTop)
		sprite.y = borderTop;
	
	if (sprite.y <= borderBottom)
		sprite.y = borderBottom;

}

