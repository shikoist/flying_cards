//------------------------------------------------------
//	����:		MAIN.CPP
//	��������:	������� ������ ���������� DirectDraw
//------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include "main.h"

//����������� ��������� Windows
BOOL DX_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void DX_OnDestroy(HWND hwnd);
void DX_OnTimer(HWND hwnd, UINT id);
void DX_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void DX_OnMouse(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DX_OnIdle(HWND hwnd);

//������� ���������
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//�������������� �������
#define TIMER_ID	1
#define TIMER_RATE	20 // moving
#define TIMER2_ID	2
#define TIMER2_RATE	20 // animation
#define TIMER3_ID	3
#define TIMER3_RATE	5000 // back color change

//HICON invisibleCursor;

//���� ���������� ������ ����������
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
	
	// ��������� ���������� ��������� ������
	char par1[80];
	if (strlen(lpszCmdLine) >= 2)
	{
		// �������� ��� ������ ������� ��������� ������
		strncpy(par1, lpszCmdLine, 2);
		
		// ���� ��������� �������� � ����� �����������, ������������
		if (strcmp(par1,"/p") == 0 || strcmp(par1,"/P") == 0)
			return (msg.wParam);
		if (strcmp(par1,"/c") == 0 || strcmp(par1,"/C") == 0)
			return (msg.wParam);
	}

	//����������� �������� ������
	wndClass.cbSize       =sizeof(wndClass);
	wndClass.style        =CS_HREDRAW|CS_VREDRAW;
	wndClass.lpfnWndProc  =WndProc;
	wndClass.cbClsExtra   =0;
	wndClass.cbWndExtra   =0;
	wndClass.hInstance    =hInst;
	wndClass.hIcon        =LoadIcon(NULL,IDI_WINLOGO);
	wndClass.hCursor      =LoadCursor(NULL,IDC_ARROW);
	wndClass.hbrBackground=NULL;//�������� ��������!!!
	wndClass.lpszMenuName =NULL;
	wndClass.lpszClassName=ClassName;
	wndClass.hIconSm      =LoadIcon(NULL,IDI_WINLOGO);

	RegisterClassEx(&wndClass);

	//�������� ���� �� ������ ������
	hWnd=CreateWindowEx(
		0,//WS_EX_TOPMOST,//�������������� ����� ����
		ClassName,	//����� ����
		AppName,	//����� ���������
		WS_POPUP,	//����� ����
		0,0,		//���������� X � Y
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),//������ � ������
		NULL,		//���������� ������������� ����
		NULL,		//���������� ����
		hInst,		//��������� ����������
		NULL);		//�������������� ������

	// �������� ������
	ShowCursor(FALSE);

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	//������������� �����������, ��������� � DirectDraw
	if (!InitDirectDraw(hWnd))
		return FALSE;

	//������� ���� ���������
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
			if (bActive)//������ ���� ���������� �������!
			{
				DX_OnIdle(hWnd);
			}
		}
	}
	return (msg.wParam);
}


//---------------------------------------------------------
//	������� ���������
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

		/* ����������� ��������� */
//---------------------------------------------------------
BOOL DX_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
//��������� �������
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
	//������ ����� ����
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
	//���������� �����
	DrawFrame();
}
//---------------------------------------------------------
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	//��� ������� ������� ���������� ������ ���������
	//if (vk==VK_SPACE || vk==VK_ESCAPE)
		DestroyWindow(hwnd);
}
//---------------------------------------------------------
void DX_OnMouse(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//��� �������� ���� ���������� ������ ���������
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
	//�������� ���� ��������� ����������
	if (state==WA_INACTIVE)
		bActive=FALSE;
	else
		bActive=TRUE;
}