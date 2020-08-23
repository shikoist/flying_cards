//------------------------------------------------------
//  ����:  DRAW.CPP
//  ��������: ������������� ������ DirectDraw
//------------------------------------------------------
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <ctime>

#include "draw.h"
#include "main.h"

//������������ ��������
#define MAX_SPEED 2
#define MIN_SPEED 1

#define MAX_SPRITES 2

static HWND hMainWnd;

LPDIRECTDRAW pDD;
LPDIRECTDRAWSURFACE pPrimarySurface;
//LPDIRECTDRAWSURFACE pPicFrame;
LPDIRECTDRAWSURFACE pBackBuffer;
LPDIRECTDRAWPALETTE pDDPal;

//char* pFileName = "ess1868f.bmp";
char* pFileNames[MAX_SPRITES] = {
	"ess1868f_Animated.bmp",
	"TVGA-9000C_2_coll.bmp"
};

//����������
//int x = 100;
//int y = 100;

//����������
//int x1 = - 5;
//int y1 = 1;



class Sprite {
public:
	LPDIRECTDRAWSURFACE pPicFrame; // ����������� DirectDraw
	int x; // ��������� ����������
	int y; //
	int x1; // �������� �� ��� X
	int y1; // �������� �� ��� Y
	int w; // ������ ������ ��������, �� �����
	int h; // ������ ������ ��������

	Sprite() {
		x = rand()%MAX_WIDTH - FRAME_WIDTH; // ��������� ��������� �� ������
		y = rand()%MAX_HEIGHT - FRAME_HEIGHT;
		x1 = 1; // ��������� ��������
		y1 = 1;
		w = 64; // ������ �������� �� ������
		h = 64; // ������ �������� �� ������
	}
};

class AnimatedSprite: public Sprite {
public:
	int framesHorizontal; // ������ �� �����������
	int framesVertical; // ������ �� ���������
	int currentFrame; // ������� ����
	int maxFrame;

	AnimatedSprite() {
		framesHorizontal = 2;
		framesVertical = 4;
		currentFrame = 0;
		maxFrame = 7;
	}
};

class SpriteCollection {
public:
	AnimatedSprite sprites[MAX_SPRITES];
};

SpriteCollection spriteCollection;

//spriteCollection.sprites[0].x = 50;

//---------------------------------------------------------
//�������, ���������� � ������ ������
//
void ErrorHandle(HWND hwnd, LPCTSTR szError)
{
	//����������, ������� ����� ��������� ������ � ������� ������
	char szErrorMessage[255];
	//����� ��� ��� ������� ���������,
	//��������� �������� ������ DirectDraw
	RemoveDirectDraw();
	//������ �������� ����
	ShowWindow(hwnd, SW_HIDE);
	//������� ��������� �� ������
	wsprintf(szErrorMessage, "��������� ��������\n������ � %s", szError);
	MessageBox(hwnd, szErrorMessage, AppName, MB_OK);
	//��������� ����
	DestroyWindow(hwnd);
}
//---------------------------------------------------------
//�������� ���� �����������, ��������� � DirectDraw
//
void RemoveDirectDraw()
{
	//���������, ���������� �� ��������� IDirectDraw7
	if (pDD != NULL)
	{
		//���������, ���������� �� ��������� ��������� �����������
		if (pPrimarySurface!=NULL)
		{
			//���������� ��������� ��������� �����������
			pPrimarySurface->Release();
			pPrimarySurface = NULL;
		}
		//�������� �� ������������� ����������� ������������
		//if (pPicFrame!=NULL)
		//{
			//����������� ����������� ������������
		//	pPicFrame->Release();
		//	pPicFrame=NULL;
		//}
		
		// �������� ��������
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame != NULL)
			{
				spriteCollection.sprites[i].pPicFrame->Release();
				spriteCollection.sprites[i].pPicFrame = NULL;
			}
		}

		//�������� �� ������������� ���������� �������
		if(pDDPal!=NULL)
		{
			//����������� ���������� �������
			pDDPal->Release();
			pDDPal=NULL;
		}
		//����������� ���������� DirectDraw
		pDD->Release();
		pDD=NULL;
	}
}
//---------------------------------------------------------
//������������� DirectDraw
//
BOOL InitDirectDraw (HWND hwnd)
{
	
	

	//�������� ��� ����������
	pPrimarySurface=NULL;
	pBackBuffer=NULL;
	pDDPal=NULL;

	//pPicFrame=NULL;

	for (int i = 0; i < MAX_SPRITES; i++) {
		spriteCollection.sprites[i].pPicFrame = NULL; 
	}

	hMainWnd=hwnd;

	//���������� ��� ������������ �����
	HRESULT hRet;

	//�������� ���������� IDirectDraw7
	//hRet=DirectDrawCreateEx(NULL, (VOID**)&pDD, IID_IDirectDraw7, NULL);
	hRet=DirectDrawCreate(NULL, &pDD, NULL);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd, "DirectDrawCreate");
		return (FALSE);
	}

	//��������� ������������� ������ ����������
	hRet=pDD->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetCooperativeLevel");
		return (FALSE);
	}

	//��������� ������������ �����������
	//hRet=pDD->SetDisplayMode(MAX_WIDTH, MAX_HEIGHT, COLOR_DEPTH, NULL, NULL);
	hRet=pDD->SetDisplayMode(MAX_WIDTH, MAX_HEIGHT, COLOR_DEPTH);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetDisplayMode");
		return (FALSE);
	}

	//����� ������� �������� ������������
	if (!CreateSurfaces())
	{
		ErrorHandle(hMainWnd, "CreateSurfaces");
		return (FALSE);
	}

	//����� ������� ���������� ������������ � ������
	if (!PrepareSurfaces())
	{
		ErrorHandle(hMainWnd, "PrepareSurfaces");
		return (FALSE);
	}

	return (TRUE);
}
//---------------------------------------------------------
//�������� ������������
//
BOOL CreateSurfaces()
{
	//���������� ����������� ��� ������ ������� DirectDraw
	//�������� � ����������
	DDSURFACEDESC ddSurfaceDesc;
	DDSCAPS ddsCaps;
	HRESULT hRet;

	//�������� ��������� �� "������" � ��������� ���� �� �������
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);

	//��������� ����������� ����� ���������
	ddSurfaceDesc.dwFlags=DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddSurfaceDesc.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddSurfaceDesc.dwBackBufferCount=1;

	//�������� �����������
	hRet=pDD->CreateSurface(&ddSurfaceDesc,	&pPrimarySurface, NULL);
	if (hRet!=DD_OK)
		return (FALSE);

	//�������� ���������� ������
	ZeroMemory(&ddsCaps, sizeof(ddsCaps));
	ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
	hRet=pPrimarySurface->GetAttachedSurface(&ddsCaps, &pBackBuffer);
	if(hRet!=DD_OK)
		return (FALSE);

	//�������� ����������� ������������
	//ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	//ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);
	//ddSurfaceDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	//ddSurfaceDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
	//ddSurfaceDesc.dwHeight=64;
	//ddSurfaceDesc.dwWidth=64;
	//hRet=pDD->CreateSurface(&ddSurfaceDesc, &pPicFrame, NULL);
	//if(hRet!=DD_OK)
	//	return (FALSE);
	
	// �� �� ����� ��� ��������
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

	//��������� ���������� ��������� � "��������� �������"
	DDCOLORKEY ddColorKey;
	ddColorKey.dwColorSpaceLowValue=TRASPARENT_COLOR;
	ddColorKey.dwColorSpaceHighValue=TRASPARENT_COLOR;

	//��������� "�������� ������" ��� ���� ����������� ������������
	//pPicFrame->SetColorKey(DDCKEY_SRCBLT, &ddColorKey);
	for (i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].pPicFrame->SetColorKey(DDCKEY_SRCBLT, 
			&ddColorKey);
	}
	return (TRUE);
}
//---------------------------------------------------------
//�������� �������
//
LPDIRECTDRAWPALETTE CreateDirectDrawPalette(LPDIRECTDRAW pDD)
{
	//���������� ����������� � �������� ��� ������ � ��������
	LPDIRECTDRAWPALETTE pDirectDrawPal;
	PALETTEENTRY palEntries[256];
	HRESULT hRet;
	LPRGBQUAD pColorTable;
	UINT uMemNeed=sizeof(RGBQUAD)*256;
	DWORD nBytesRead;

	//�������� ������������ �����, ����������� �������
	HANDLE hFile=CreateFile(
		pFileNames[0], GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
 	if (hFile==INVALID_HANDLE_VALUE)
	{
		pDirectDrawPal=NULL;
		return (pDirectDrawPal);
	}

	//��������� ������ ��� �������� �������
	pColorTable= (LPRGBQUAD)malloc(uMemNeed);
	//��������� ��������� ����� �� ������ �������
	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),
		NULL, FILE_BEGIN);
	//������ ������� �� �����
	ReadFile(hFile, (LPVOID)pColorTable, uMemNeed, &nBytesRead, NULL);
	//�������� ������������ �����
	CloseHandle(hFile);

	//������� ������� �� RGBQUAD � RGBTRIPPLE
	for (int x=0;x<256;++x)
	{
		palEntries[x].peRed=pColorTable[x].rgbRed;
		palEntries[x].peBlue=pColorTable[x].rgbBlue;
		palEntries[x].peGreen=pColorTable[x].rgbGreen;
	}

	//�������� ������� DirectDraw
	hRet=pDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		palEntries, &pDirectDrawPal, NULL);
	if (hRet!=DD_OK)
		pDirectDrawPal=NULL;

	//������������ ������
	free(pColorTable);

	return (pDirectDrawPal);
}
//---------------------------------------------------------
//���������� ������������ � ������
BOOL PrepareSurfaces()
{
	//�������� ������� DirectDraw
	pDDPal=CreateDirectDrawPalette(pDD);
	if (pDDPal==NULL)
		return (FALSE);

	//������������ ������� ��������� �����������
	pPrimarySurface->SetPalette(pDDPal);

	//�������� ����������� ������ �� ������ �� ����������� �����������
	//if (!LoadBMP(pPicFrame, pFileName))
	//	return (FALSE);
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		if (!LoadBMP(spriteCollection.sprites[i].pPicFrame, pFileNames[i]))
			return (FALSE);
	}
	return (TRUE);
}
//---------------------------------------------------------
//�������� ����������� �� BMP-�����
//
BOOL LoadBMP(LPDIRECTDRAWSURFACE pSurface, char* filename)
{
	//���������� ����������, ����������� ��� ������ ������ �� BMP-�����
	BYTE* pBmp;
	DWORD dwBmpSize;
	DWORD dwFileLength;
	DWORD nBytesRead;

	BITMAPINFO* pBmpInfo;
	BYTE*		pPixels;

	HDC hdc;

	//�������� ����� � ������������ �������
	HANDLE hFile=CreateFile(filename, GENERIC_READ,
			FILE_SHARE_READ, NULL,OPEN_EXISTING, 0, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return (FALSE);

	//��������� ������� ����� � ������� ������
	dwFileLength=GetFileSize (hFile, NULL) ;
	dwBmpSize=dwFileLength-sizeof(BITMAPFILEHEADER);

	//��������� ������ ��� ������
	pBmp=	(BYTE*) malloc(dwBmpSize);
	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);

	//������ ����� � �������
	ReadFile(hFile, (LPVOID)pBmp, dwBmpSize, &nBytesRead, NULL);
	CloseHandle(hFile);

	pBmpInfo=(BITMAPINFO*)pBmp;
	pPixels=pBmp+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256;

	//��������� ��������� ��������� ���������� ����������� �����������
	if ((pSurface->GetDC(&hdc)) == DD_OK)
	{
	//����������� ����������� ������ �� ������
	//�� ����������� ����������� ���������� GDI
		//StretchDIBits(hdc , 0, 0, FRAME_WIDTH, FRAME_HEIGHT, 0, 0, FRAME_WIDTH, FRAME_HEIGHT, pPixels, pBmpInfo, 0, SRCCOPY);
		StretchDIBits(hdc,
			0, 0, 64, 64,
			0, 0, 64, 64,
			pPixels, pBmpInfo, 0, SRCCOPY);
		pSurface->ReleaseDC(hdc);
	}
	//������������ ������
	free(pBmp);

	return (TRUE);
}
//---------------------------------------------------------
//�������� ����� �� "������"
//
void PrepareFrame()
{
	//�������� �� "������" ��������� �����������
	if (pPrimarySurface->IsLost())
	{
		//�������������� ��������� ����������� � ���������� ������
		pPrimarySurface->Restore();
		pBackBuffer->Restore();

		//�������� ����������� ������������ �� "������"
		//if (pPicFrame->IsLost())
		//{
			//�������������� ����������� ������������
			//� ���������� �� ������� �� ������
		//	pPicFrame->Restore();
		//	LoadBMP(pPicFrame, pFileName);
		//}

		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame->IsLost())
			{
				//�������������� ����������� ������������
				//� ���������� �� ������� �� ������
				spriteCollection.sprites[i].pPicFrame->Restore();
				LoadBMP(spriteCollection.sprites[i].pPicFrame,
					pFileNames[i]);
			}
		}
	}
}
//---------------------------------------------------------
//�������� �����������
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
//����� ����������� �� �����
//
void DrawFrame()
{
	RECT rPic;

	//���������� ������������
	PrepareFrame();
	ClearSurface(pBackBuffer);

	//��������� �������� ����������� ����� ������
	//SetRect(&rPic, 0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	
	//x += x1;
	//y += y1;
	
	//if (x > MAX_WIDTH - FRAME_WIDTH || x < 0)
	//{
		
	//	int r = rand()%MAX_SPEED + MIN_SPEED;
		
		//if (x1 > 0) {
		//	x1 = -r;
		//}
		//else {
		//	x1 = r;
		//}
	//}
	//if (x > MAX_WIDTH - FRAME_WIDTH)
	//{
		//x = MAX_WIDTH - FRAME_WIDTH;
	//}
	//if (x < 0)
	//{
		//x = 0;
	//}

	//if (y > MAX_HEIGHT - FRAME_HEIGHT || y < 0)	{
		
		//int r = rand()%MAX_SPEED + MIN_SPEED;
		//if (y1 > 0)
		//	y1 = -r;
		//else
		//	y1 = r;
	//}
	//if (y > MAX_HEIGHT - FRAME_HEIGHT)
	//	y = MAX_HEIGHT - FRAME_HEIGHT;

	//if (y < 0)
	//	y = 0;

	// � ������ �� �� ����� ��� ��������
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

	//����������� ����������� ������ � ����������� �����������
	//�� ��������� �����
	//pBackBuffer->BltFast(
	//	x,
	//	y,
	//	pPicFrame,
	//	&rPic,
	//	DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);

	for (i = 0; i < MAX_SPRITES; i++)
	{
		switch (spriteCollection.sprites[i].currentFrame)
		{
			case 0: {
				SetRect(&rPic,
					0, 0, 
					FRAME_WIDTH, FRAME_HEIGHT);
				break;
			}
			case 1: {
				SetRect(&rPic,
					FRAME_WIDTH, 0,
					FRAME_WIDTH * 2, FRAME_HEIGHT);
				break;
			}
			case 2: {
				SetRect(&rPic,
					0, FRAME_HEIGHT,
					FRAME_WIDTH, FRAME_HEIGHT * 2);
				break;
			}
			case 3: {
				SetRect(&rPic,
					FRAME_WIDTH, FRAME_HEIGHT,
					FRAME_WIDTH * 2, FRAME_HEIGHT * 2);
				break;
			}
			case 4: {
				SetRect(&rPic,
					0, FRAME_HEIGHT * 2,
					FRAME_WIDTH, FRAME_HEIGHT * 3);
				break;
			}
			case 5: {
				SetRect(&rPic,
					FRAME_WIDTH, FRAME_HEIGHT * 2, 
					FRAME_WIDTH * 2, FRAME_HEIGHT * 3);
				break;
			}
			case 6: {
				SetRect(&rPic, 
					0, FRAME_HEIGHT * 3,
					FRAME_WIDTH, FRAME_HEIGHT * 4);
				break;
			}
			case 7: {
				SetRect(&rPic,
					FRAME_WIDTH, FRAME_HEIGHT * 3,
					FRAME_WIDTH * 2, FRAME_HEIGHT * 4);
				break;
			}
		}
		

		pBackBuffer->BltFast(
			spriteCollection.sprites[i].x,
			spriteCollection.sprites[i].y,
			spriteCollection.sprites[i].pPicFrame,
			&rPic,
			DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
	}

	//������������ ������������
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