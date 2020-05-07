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

/*������ ����(API)*/
bool __Init(HINSTANCE hinstance, int nCmdShow);
LRESULT CALLBACK __WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void __WinKeyDown(HWND hwnd, WPARAM wParam);
void __WinDestroy(void);
BOOL DXW_Init(void);
void MainLoop(void);
void __DisplayInfo();

///////////////////////////////////////////////////////////////
/*ȭ�鿡 �׸��� ����*/
void __GetDC(LPDIRECTDRAWSURFACE7 lpDDSDest);
void __ReleaseDC(LPDIRECTDRAWSURFACE7 lpDDSDest);
HFONT __CreateFont(LPCTSTR FontName, int nWidth, int nWeight);
void __DestroyFont(void);
void __PutText(int x, int y, COLORREF Color, LPCTSTR pString, HFONT Font);
void __PutTextf(int x, int y, COLORREF Color, HFONT Font, const char *format, ...);

void _FillSurface(LPDIRECTDRAWSURFACE7 lpDDSDest, DWORD dwColor);
void _FillRectEx(LPDIRECTDRAWSURFACE7 lpDDSDest, LPRECT lpRect, DWORD dwColor);


////////////////////////////���Ӱ��� �߰��ڵ�
void GameCode(int state);
void GameInit(); //���� ������ �� �ѹ��� �����ϴ� �ڵ�
inline void LoopEnemy(int count); //���� ������ ������ �ѹ��� ������
//�浹�ڵ�
inline BOOL HeroCrash(const POINT heroPos, const RECT EnePos);

//������ ���� �ڵ�
void ItemCode();
BOOL GetItems(const POINT heroPos, const POINT ItemPos);
void ItemDraw(int Kind, int x, int y);
void ItemUse(int ItemKind);

inline void RankingView(); //��ŷȭ�鿡�� �Ѹ��� �ڵ�
