//------------------------------------------------------
//  Файл:  DRAW.CPP
//  Описание: Демонстрирует основы DirectDraw
//------------------------------------------------------
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <ctime>

#include "draw.h"
#include "main.h"

//Максимальная скорость
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

//Координаты
//int x = 100;
//int y = 100;

//Приращение
//int x1 = - 5;
//int y1 = 1;



class Sprite {
public:
	LPDIRECTDRAWSURFACE pPicFrame; // Поверхность DirectDraw
	int x; // Начальные координаты
	int y; //
	int x1; // Скорость по оси X
	int y1; // Скорость по оси Y
	int w; // Ширина полной картинки, не кадра
	int h; // Высота полной картинки

	Sprite() {
		x = rand()%MAX_WIDTH - FRAME_WIDTH; // Начальное положение на экране
		y = rand()%MAX_HEIGHT - FRAME_HEIGHT;
		x1 = 1; // Начальные скорости
		y1 = 1;
		w = 64; // Размер картинки по щирине
		h = 64; // Размер картинки по высоте
	}
};

class AnimatedSprite: public Sprite {
public:
	int framesHorizontal; // Кадров по горизонтали
	int framesVertical; // Кадров по вертикали
	int currentFrame; // Текущий кадр
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
//Функция, вызываемая в случае ошибки
//
void ErrorHandle(HWND hwnd, LPCTSTR szError)
{
	//Переменная, которая будет содержать строку с текстом ошибки
	char szErrorMessage[255];
	//Перед тем как вывести сообщение,
	//корректно завершим работу DirectDraw
	RemoveDirectDraw();
	//Скроем основное окно
	ShowWindow(hwnd, SW_HIDE);
	//Выведем сообщение об ошибке
	wsprintf(szErrorMessage, "Программа прервана\nОшибка в %s", szError);
	MessageBox(hwnd, szErrorMessage, AppName, MB_OK);
	//Уничтожим окно
	DestroyWindow(hwnd);
}
//---------------------------------------------------------
//Отчистка всех интерфейсов, связанных с DirectDraw
//
void RemoveDirectDraw()
{
	//Проверяем, существует ли интерфейс IDirectDraw7
	if (pDD != NULL)
	{
		//Проверяем, существует ли интерфейс первичной поверхности
		if (pPrimarySurface!=NULL)
		{
			//Уничтожаем интерфейс первичной поверхности
			pPrimarySurface->Release();
			pPrimarySurface = NULL;
		}
		//Проверка на существование внеэкранных поверхностей
		//if (pPicFrame!=NULL)
		//{
			//Уничтожение внеэкранных поверхностей
		//	pPicFrame->Release();
		//	pPicFrame=NULL;
		//}
		
		// Проверка спрайтов
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			if (spriteCollection.sprites[i].pPicFrame != NULL)
			{
				spriteCollection.sprites[i].pPicFrame->Release();
				spriteCollection.sprites[i].pPicFrame = NULL;
			}
		}

		//Проверка на существование интерфейса палитры
		if(pDDPal!=NULL)
		{
			//Уничтожение интерфейса палитры
			pDDPal->Release();
			pDDPal=NULL;
		}
		//Уничтожение интерфейса DirectDraw
		pDD->Release();
		pDD=NULL;
	}
}
//---------------------------------------------------------
//Инициализация DirectDraw
//
BOOL InitDirectDraw (HWND hwnd)
{
	
	

	//Обнуляем все интерфейсы
	pPrimarySurface=NULL;
	pBackBuffer=NULL;
	pDDPal=NULL;

	//pPicFrame=NULL;

	for (int i = 0; i < MAX_SPRITES; i++) {
		spriteCollection.sprites[i].pPicFrame = NULL; 
	}

	hMainWnd=hwnd;

	//Переменная для возвращаемых кодов
	HRESULT hRet;

	//Создание интерфейса IDirectDraw7
	//hRet=DirectDrawCreateEx(NULL, (VOID**)&pDD, IID_IDirectDraw7, NULL);
	hRet=DirectDrawCreate(NULL, &pDD, NULL);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd, "DirectDrawCreate");
		return (FALSE);
	}

	//Установка эксклюзивного режима кооперации
	hRet=pDD->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetCooperativeLevel");
		return (FALSE);
	}

	//Установка необходимого видеорежима
	//hRet=pDD->SetDisplayMode(MAX_WIDTH, MAX_HEIGHT, COLOR_DEPTH, NULL, NULL);
	hRet=pDD->SetDisplayMode(MAX_WIDTH, MAX_HEIGHT, COLOR_DEPTH);
	if (hRet!=DD_OK)
	{
		ErrorHandle(hMainWnd,"SetDisplayMode");
		return (FALSE);
	}

	//Вызов функции создания поверхностей
	if (!CreateSurfaces())
	{
		ErrorHandle(hMainWnd, "CreateSurfaces");
		return (FALSE);
	}

	//Вызов функции подготовки поверхностей к работе
	if (!PrepareSurfaces())
	{
		ErrorHandle(hMainWnd, "PrepareSurfaces");
		return (FALSE);
	}

	return (TRUE);
}
//---------------------------------------------------------
//Создание поверхностей
//
BOOL CreateSurfaces()
{
	//Объявление необходимых для многих функций DirectDraw
	//структур и переменных
	DDSURFACEDESC ddSurfaceDesc;
	DDSCAPS ddsCaps;
	HRESULT hRet;

	//Отчистка структуры от "мусора" и установка поля ее размера
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);

	//Установка необходимых полей структуры
	ddSurfaceDesc.dwFlags=DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddSurfaceDesc.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddSurfaceDesc.dwBackBufferCount=1;

	//Создание поверхности
	hRet=pDD->CreateSurface(&ddSurfaceDesc,	&pPrimarySurface, NULL);
	if (hRet!=DD_OK)
		return (FALSE);

	//Создание вторичного буфера
	ZeroMemory(&ddsCaps, sizeof(ddsCaps));
	ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
	hRet=pPrimarySurface->GetAttachedSurface(&ddsCaps, &pBackBuffer);
	if(hRet!=DD_OK)
		return (FALSE);

	//Создание внеэкранных поверхностей
	//ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	//ddSurfaceDesc.dwSize=sizeof(ddSurfaceDesc);
	//ddSurfaceDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	//ddSurfaceDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
	//ddSurfaceDesc.dwHeight=64;
	//ddSurfaceDesc.dwWidth=64;
	//hRet=pDD->CreateSurface(&ddSurfaceDesc, &pPicFrame, NULL);
	//if(hRet!=DD_OK)
	//	return (FALSE);
	
	// То же самое для спрайтов
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

	//Установка параметров структуры с "цветовыми ключами"
	DDCOLORKEY ddColorKey;
	ddColorKey.dwColorSpaceLowValue=TRASPARENT_COLOR;
	ddColorKey.dwColorSpaceHighValue=TRASPARENT_COLOR;

	//Установка "цветовых ключей" для всех внеэкранных поверхностей
	//pPicFrame->SetColorKey(DDCKEY_SRCBLT, &ddColorKey);
	for (i = 0; i < MAX_SPRITES; i++)
	{
		spriteCollection.sprites[i].pPicFrame->SetColorKey(DDCKEY_SRCBLT, 
			&ddColorKey);
	}
	return (TRUE);
}
//---------------------------------------------------------
//Создание палитры
//
LPDIRECTDRAWPALETTE CreateDirectDrawPalette(LPDIRECTDRAW pDD)
{
	//Объявление интерфейсов и структур для работы с палитрой
	LPDIRECTDRAWPALETTE pDirectDrawPal;
	PALETTEENTRY palEntries[256];
	HRESULT hRet;
	LPRGBQUAD pColorTable;
	UINT uMemNeed=sizeof(RGBQUAD)*256;
	DWORD nBytesRead;

	//Открытие графического файла, содержащего палитру
	HANDLE hFile=CreateFile(
		pFileNames[0], GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
 	if (hFile==INVALID_HANDLE_VALUE)
	{
		pDirectDrawPal=NULL;
		return (pDirectDrawPal);
	}

	//Выделение памяти под файловую палитру
	pColorTable= (LPRGBQUAD)malloc(uMemNeed);
	//Установка указателя файла на начало палитры
	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),
		NULL, FILE_BEGIN);
	//Чтение палитры из файла
	ReadFile(hFile, (LPVOID)pColorTable, uMemNeed, &nBytesRead, NULL);
	//Закрытие графического файла
	CloseHandle(hFile);

	//Перевод палитры из RGBQUAD в RGBTRIPPLE
	for (int x=0;x<256;++x)
	{
		palEntries[x].peRed=pColorTable[x].rgbRed;
		palEntries[x].peBlue=pColorTable[x].rgbBlue;
		palEntries[x].peGreen=pColorTable[x].rgbGreen;
	}

	//Создание палитры DirectDraw
	hRet=pDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		palEntries, &pDirectDrawPal, NULL);
	if (hRet!=DD_OK)
		pDirectDrawPal=NULL;

	//Освобождение памяти
	free(pColorTable);

	return (pDirectDrawPal);
}
//---------------------------------------------------------
//Подготовка поверхностей к выводу
BOOL PrepareSurfaces()
{
	//Создание палитры DirectDraw
	pDDPal=CreateDirectDrawPalette(pDD);
	if (pDDPal==NULL)
		return (FALSE);

	//Присваивание палитры первичной поверхности
	pPrimarySurface->SetPalette(pDDPal);

	//Загрузка графических данных из файлов на внеэкранные поверхности
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
		return (FALSE);

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
				LoadBMP(spriteCollection.sprites[i].pPicFrame,
					pFileNames[i]);
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

	//Подготовка поверхностей
	PrepareFrame();
	ClearSurface(pBackBuffer);

	//Установка размеров копируемого блока данных
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

	// А теперь то же самое для спрайтов
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

	//Копирование графических данных с внеэкранной поверхности
	//на вторичный буфер
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

	//Переключение поверхностей
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