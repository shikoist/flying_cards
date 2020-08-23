//------------------------------------------------------
//  Файл:  DRAW.H
//  Описание:  Демонстрирует основы DirectDraw
//------------------------------------------------------

#define MAX_WIDTH 640
#define MAX_HEIGHT 480
#define COLOR_DEPTH 8
#define TRASPARENT_COLOR 0xE3

#define FRAME_HEIGHT 16
#define FRAME_WIDTH 32

BOOL CreateSurfaces();
BOOL PrepareSurfaces();
BOOL ClearSurface(LPDIRECTDRAWSURFACE pSurface);
BOOL LoadBMP(LPDIRECTDRAWSURFACE pSurface, char* filename);
