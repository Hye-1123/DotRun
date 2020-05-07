#include "DxDClass.h"

#define NAME "DotRun"
#define TITLE "DOT RUN"

#define WHITE    RGB(255,255,255)
#define BLACK    RGB(  0,  0,  0)
#define RED      RGB(255,  0,  0)
#define GREEN    RGB(  0,255,  0)
#define BLUE     RGB(  0,  0,255)

///////////////////////////////////////

#ifndef __APP_SET__

typedef struct tagAPP_SET
{
	BOOL bIsPause;	
	BOOL bIsActive;
	BOOL bIsDisplayInfo;
} APP_SET;

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////

class _DXWFRAME_H_ {} ;

/*윈도우 관련(API)*/
bool __Init(HINSTANCE hinstance, int nCmdShow);
LRESULT CALLBACK __WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void __WinKeyDown(HWND hwnd, WPARAM wParam);
void __WinDestroy(void);
BOOL DXW_Init(void);
void MainLoop(void);
void __DisplayInfo();

///////////////////////////////////////////////////////////////
/*화면에 그리기 위한*/
void __GetDC(LPDIRECTDRAWSURFACE7 lpDDSDest);
void __ReleaseDC(LPDIRECTDRAWSURFACE7 lpDDSDest);
HFONT __CreateFont(LPCTSTR FontName, int nWidth, int nWeight);
void __DestroyFont(void);
void __PutText(int x, int y, COLORREF Color, LPCTSTR pString, HFONT Font);
void __PutTextf(int x, int y, COLORREF Color, HFONT Font, const char *format, ...);

void _FillSurface(LPDIRECTDRAWSURFACE7 lpDDSDest, DWORD dwColor);
void _FillRectEx(LPDIRECTDRAWSURFACE7 lpDDSDest, LPRECT lpRect, DWORD dwColor);


////////////////////////////게임관련 추가코드
void GameCode(int state);
void GameInit(); //게임 시작할 때 한번만 실행하는 코드
inline void LoopEnemy(int count); //적들 루프문 돌릴거 한번에 돌린다
//충돌코드
inline BOOL HeroCrash(const POINT heroPos, const RECT EnePos);

//아이템 관련 코드
void ItemCode();
BOOL GetItems(const POINT heroPos, const POINT ItemPos);
void ItemDraw(int Kind, int x, int y);
void ItemUse(int ItemKind);

inline void RankingView(); //랭킹화면에서 뿌리는 코드
