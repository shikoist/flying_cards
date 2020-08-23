//------------------------------------------------------
//  Файл:  MAIN.H
//  Описание:  Главный модуль приложения DirectDraw
//------------------------------------------------------

#define ClassName "DX_Window"
#define AppName "DirectDraw Application"

BOOL InitDirectDraw (HWND hwnd);
void RemoveDirectDraw (void);

void DrawFrame(void);
void NextTick( void );