//------------------------------------------------------
//	Файл:		MAIN.CPP
//	Описание:	Главный модуль приложения DirectDraw
//------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include "main.h"

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

//Характеристики таймера
#define TIMER_ID	1
#define TIMER_RATE	20 // moving
#define TIMER2_ID	2
#define TIMER2_RATE	20 // animation
#define TIMER3_ID	3
#define TIMER3_RATE	5000 // back color change

//HICON invisibleCursor;

//Флаг активности нашего приложения
BOOL	bActive;

int mouse_x = 0;
int mouse_y = 0;
int mouseCheck = 0;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wndClass;
	HWND hWnd;
	MSG msg;

	//BYTE CursorMaskAND[] = { 0xFF };
	//BYTE CursorMaskXOR[] = { 0x00 };
	//invisibleCursor = CreateCursor(NULL, 0,0,1,1, CursorMaskAND, CursorMaskXOR);
	
	// Обработка аргументов командной строки
	char par1[80];
	if (strlen(lpszCmdLine) >= 2)
	{
		// Копируем два первых символа командной строки
		strncpy(par1, lpszCmdLine, 2);
		
		// Если программа запущена с этими параметрами, игнорировать
		if (strcmp(par1,"/p") == 0 || strcmp(par1,"/P") == 0)
			return (msg.wParam);
		if (strcmp(par1,"/c") == 0 || strcmp(par1,"/C") == 0)
			return (msg.wParam);
	}

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

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	//Инициализация компонентов, связанных с DirectDraw
	if (!InitDirectDraw(hWnd))
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
	if (!SetTimer(hwnd,TIMER_ID,TIMER_RATE,NULL))
		return (FALSE);
	if (!SetTimer(hwnd,TIMER2_ID,TIMER2_RATE,NULL))
		return (FALSE);
	if (!SetTimer(hwnd,TIMER3_ID,TIMER3_RATE,NULL))
		return (FALSE);
	return (TRUE);
}

//---------------------------------------------------------
void DX_OnDestroy(HWND hwnd)
{
	//Убрать после себя
	KillTimer(hwnd,TIMER_ID);
	RemoveDirectDraw();
	PostQuitMessage(0);
}
//---------------------------------------------------------
void DX_OnTimer(HWND hwnd, UINT id)
{
	if (id == TIMER_ID)
		if (bActive) 
			MoveSprites();
		
	// Moving code
	if (id == TIMER2_ID)
		if (bActive)
			NextTick();
	// 
	if (id == TIMER3_ID)
		if (bActive)
			ChangeColor();
}
//---------------------------------------------------------

void DX_OnIdle(HWND hwnd)
{
	//Прорисовка кадра
	DrawFrame();
}
//---------------------------------------------------------
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	//При нажатии пробела прекратить работу программы
	//if (vk==VK_SPACE || vk==VK_ESCAPE)
		DestroyWindow(hwnd);
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