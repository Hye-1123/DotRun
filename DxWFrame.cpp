#include "DxWFrame.h"
#include "ddutil.h"
#include "LockPixelFx.h"
#include "FpsMng.h"
#include "HHSound.h"
#include "EnemyH.h"
#include "RankH.h"

#define HRES 640
#define VRES 480
#define HEDGE   6
#define VEDGE  27

#define FPS 0
#define BPP 32
#define BBC 2

#define _DISPLAYINFO

#define FNTSZ  24
#define FNTWH  16

#define dTIMEGETTIME timeGetTime() * 0.001 //�ð� ���� ���� ��
#define COLORKEY RGB(0, 0, 0)

//TODO : �ڵ� ����
/*���� ����*/
APP_SET     gAppSet;
DxDClass dxd;
PixelFx pfx;
CFpsMng gFpsMng;
CHHSound HHSound;

HFONT		gFContent, gFTitle;
HDC			gHdc;

//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE7 g_pDDSHero;
LPDIRECTDRAWSURFACE7 g_pDDSItem1, g_pDDSItem2;
POINT g_HeroPos;

enum State
{
	en_MO,
	en_GB,
	en_GO,
	en_GOver,
	en_Ranking
};

int nState; //���α׷� ����
DWORD dw_LastScore, dw_itemTime, dw_ItemUse; //������ ���ھ� ���� �ð�, ������ �⿬����, ������ ������ �ð�
DWORD dw_LastGene, dw_LastSzUp; //���������� �� �ø� �ð�
DWORD dw_main;

//���ӿ�
EnemyClass Enemy;//��
int gScore, gBScore = 0, gScoreX; //����, ����Ʈ ���ھ�, ���� ���
int gItemKind = 0; //ȭ�鿡 �ִ� ������ ����
int gnItem = 0; //�Լ��� ������ ���ι� ���� ���� �� 0��

//��ŷ, ���ӿ��� ����
char gcName[256] = { 0 }; //��ϰ��� �� �̸� ��ϵ� ����
BOOL gbRanking; //��� ���Ž� �̸��� �Է¾��ϸ� ȭ���� �Ѿ�� �ʰ���. TRUE�ϵ��� �̸��� �Է��ϰ� ���͸� ġ�� FALSE��
BOOL gb_Once = FALSE;

LPDIRECTSOUNDBUFFER g_pDS[2];//���� ����

/*������ Ŭ���� �ʱ�ȭ �� ����*/
bool __Init(HINSTANCE hInstance, int nCmdShow)
{
	BOOL res;

	gAppSet.bIsActive = FALSE;
	gAppSet.bIsDisplayInfo = FALSE;
	gAppSet.bIsPause = FALSE;

	dw_main= dTIMEGETTIME;
	/////
	WNDCLASS  wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = __WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NAME;
	wc.lpszClassName = NAME;
	RegisterClass(&wc);

	DWORD ExWinStyle, WinStyle;

#ifdef _DEBUG
	ExWinStyle = NULL;
	WinStyle = WS_POPUPWINDOW | WS_CAPTION;
#else
	ExWinStyle = WS_EX_TOPMOST;
	WinStyle = WS_POPUP;
#endif

	hWnd = CreateWindowEx(ExWinStyle,
		NAME,
		TITLE,
		WinStyle,
		0,
		0,
		HRES + HEDGE,
		VRES + VEDGE,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)	return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HANDLE hProc = ::GetCurrentProcess();
#ifdef _DEBUG
	::SetPriorityClass(hProc, NORMAL_PRIORITY_CLASS);
#else //Release
	::SetPriorityClass(hProc, HIGH_PRIORITY_CLASS);
#endif

	/*���� ó��*/
	HHSound.Init(hWnd); //���� ��ü �ʱ�ȭ

	//���� ���
	HHSound.LoadWave("sound/sixthsense.wav", &g_pDS[0]);
	HHSound.SetVolume(g_pDS[0], 100);
	//���ӿ���
	HHSound.LoadWave("sound/pianoboom.wav", &g_pDS[1]);
	HHSound.SetVolume(g_pDS[1], 85);

	res = DXW_Init();
	if (!res) return FALSE;

	////*���� �о�ͼ� ����Ʈ ���ھ� ��������, ������ ����Ʈ ���ھ� �� 0��*////
	gBScore = RankBS();

	return TRUE;
}

/*���� �Լ�*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG msg;

	if (!__Init(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HHSound.Play(g_pDS[0], TRUE);

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0)) break; //WM_QUIT�� ���� ��������

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (gAppSet.bIsActive && !gAppSet.bIsPause) //�޽����� ���� �����찡 Ȱ��ȭ�� ���� ���η����� ����
		{
			MainLoop();
		}
		else WaitMessage();
	}
}

/*������ ���ν��� �Լ�*/
LRESULT CALLBACK __WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int len = 0;

	switch (message)
	{
	case WM_KEYDOWN: __WinKeyDown(hwnd, wParam); break;
	case WM_CHAR:
		if (nState == en_GOver && gbRanking)
		{
			if (wParam >= 65 && wParam <= 90 || wParam >= 97 && wParam <= 122)
			{
				len = strlen(gcName);
				gcName[len] = (char)wParam;
				gcName[len + 1] = '\0';
			}
		}
		break;
	case WM_ACTIVATEAPP: gAppSet.bIsActive = wParam;   break;
	case WM_MOUSEMOVE:
		break;
	case WM_DESTROY:
		__WinDestroy(); //��ü ����
		PostQuitMessage(0);
		break;
	case WM_SETCURSOR:
		if (1)
		{
			SetCursor(NULL);
			return TRUE;
		}
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void __WinKeyDown(HWND hwnd, WPARAM wParam)
{
	if (nState == en_MO && wParam != VK_ESCAPE) //ȭ�� ��ȯ ����->���ӹ��
	{
		nState = en_GB;
		return;
	}
	else if (nState == en_GB && wParam != VK_ESCAPE) //ȭ�� ��ȯ ���ӹ��->����ȭ��
	{
		nState = en_GO;
		HHSound.Stop(g_pDS[0]);
		HHSound.Play(g_pDS[0], TRUE);
		return;
	}
	else if (nState == en_Ranking && wParam != VK_ESCAPE) //ȭ�� ��ȯ ��ŷ->����ȭ��
	{
		nState = en_MO;
		HHSound.Play(g_pDS[0], TRUE); //�������� ���
		return;
	}

	switch (wParam)
	{
	case VK_ESCAPE:
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	case VK_F1:	break;
	case VK_F2: break;
	case VK_F3: break;
	case VK_F4:	break;
	case VK_F5: break;
	case VK_F6: break;
	case VK_F7: break;
	case VK_F8: break;
	case VK_F9: break;
	case VK_F10: break;
	case VK_TAB: break;
	case VK_BACK: break;
	case VK_END: break;
	case VK_HOME: break;
	case VK_DELETE: break;
	case VK_SPACE: 
		if (nState == en_GOver && !gbRanking) //ȭ�� ��ȯ ���ӿ���->��ŷ
		{
			nState = en_Ranking;
			//��ŷȭ�鿡�� �� ������ �ʱ�ȭ
			g_nSize = 0;
			gb_Once = FALSE;

			memset(gcName, 0, sizeof(gcName)); //������ �Է��� ���� ���

			HHSound.Stop(g_pDS[1]);
			return;
		}
		
		if (nState == en_GO && gnItem) //�������� ���� ���� ó��
		{
			ItemUse(gnItem); //�������� �����
			gnItem = 0; //������ ������ ���� ��
		}

		break;
	case VK_RETURN: 
 		if (nState == en_GOver && gcName[0])
		{
			gBScore = RankUp(gScore, gcName); //��ũ ��Ϲ� 1� �޾ƿͼ� ����
			gbRanking = FALSE; //�̸� �Է� ���� �˸�, ȭ�� �Ѿ�°� ����
			
		}
		break;
	}
}

/*��ü ����*/
void __WinDestroy()
{
	__DestroyFont(); //��Ʈ ����
	Enemy.Clear();

	//Dx��ü ����
	dxd.DxD_ObjRelease();

	//�޸𸮴��� Ȯ��
	_CrtDumpMemoryLeaks();

	TRACE0("__WinDestroy\n");
}

BOOL DXW_Init(void)
{
	BOOL bRval;
	RECT RClient = {0, 0, HRES, VRES};

	/*DD��ü ����*/
	bRval = dxd.DD_Init(HRES, VRES, BPP, BBC);
	if (!bRval) return FALSE;

#ifdef _DEBUG
	bRval = dxd.DxD_SetWindowClipper(dxd.DxD_getPrimarySur(), hWnd);
	pfx.__CheckRGBBit(dxd.DxD_getPrimarySur());
	if (!bRval) return FALSE;
#endif // _DEBUG
	bRval = dxd.DxD_CSetClipper(dxd.DxD_getBackSur(), RClient);
	if (!bRval) return FALSE;

	pfx.SetClientSz(HRES, VRES);

	////���ΰ� �̹��� �����ͼ� ǥ�鿡 ����(������ũ��)
	g_pDDSHero = DDLoadBitmap(dxd.DxD_getDDobj(), "hero.bmp", 0, 0);
	if (!g_pDDSHero) return ___Error("DD_Init()", __FILE__, __LINE__);
	DDSetColorKey(g_pDDSHero, COLORKEY);

	////������ �̹��� �����ͼ� ǥ�鿡 ����(������ũ��), 2����(��, ��Ʈ)
	g_pDDSItem1 = DDLoadBitmap(dxd.DxD_getDDobj(), "item1.bmp", 0, 0);
	if (!g_pDDSItem1) return ___Error("DD_Init()", __FILE__, __LINE__);
	DDSetColorKey(g_pDDSItem1, COLORKEY);
	g_pDDSItem2 = DDLoadBitmap(dxd.DxD_getDDobj(), "item2.bmp", 0, 0);
	if (!g_pDDSItem2) return ___Error("DD_Init()", __FILE__, __LINE__);
	DDSetColorKey(g_pDDSItem2, COLORKEY);

	gFpsMng.SetFPS(FPS);
	gFTitle = __CreateFont("Bondoni MT Black", 60, FW_BOLD);
	gFContent = __CreateFont("Impact", FNTSZ, FNTWH);

#ifdef _DEBUG
	gAppSet.bIsDisplayInfo = TRUE;
#endif

	srand((unsigned)time(NULL));

	TRACE0("DXW_Init()\n");
	return TRUE;
}

struct mainLoop {};
/*���� ����*/
void MainLoop()
{
	HRESULT hRet;
	
	_FillSurface(dxd.DxD_getBackSur(), 0); //ȭ�� Ŭ����

	GameCode(nState); //���� ó�� �ڵ�

#ifdef _DISPLAYINFO
	__DisplayInfo();
#endif // _DISPLAYINFO

	dxd.DxD_BltFlip();//ȭ�� ��ȯ

	gFpsMng.FrameWaiting();
}

/*���� ���÷��� ������ �����*/
void __DisplayInfo()
{
	if (gAppSet.bIsDisplayInfo)
	{
		DWORD dwFps = gFpsMng.GetFPS();

		__GetDC(dxd.DxD_getBackSur());
		{
			__PutTextf(10, 30, WHITE, gFContent, "FPS %d", dwFps);
			__PutTextf(10, 50, WHITE, gFContent, "PixelFormat %d", pfx.getRGBFormat());

		}
		__ReleaseDC(dxd.DxD_getBackSur());
	}
}

struct DCfontPrint{};
void __GetDC(LPDIRECTDRAWSURFACE7 lpDDSDest)
{
	lpDDSDest->GetDC(&gHdc); //HDC�� ��������
}

void __ReleaseDC(LPDIRECTDRAWSURFACE7 lpDDSDest)
{
	lpDDSDest->ReleaseDC(gHdc);
}

HFONT __CreateFont(LPCTSTR FontName, int nWidth, int nWeight)
{
	HFONT Font;
	
	Font = CreateFont(nWidth, 0,
		0, 0,
		nWeight,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY,
		VARIABLE_PITCH,
		FontName);

	return Font;
}

void __DestroyFont()
{
	if (gFContent)
	{
		DeleteObject(gFContent);
		gFContent = NULL;
	}

	if (gFTitle)
	{
		DeleteObject(gFTitle);
		gFTitle = NULL;
	}
}

/*���ڸ� �����*/
void __PutText(int x, int y, COLORREF Color, LPCTSTR pString, HFONT Font)
{
	SetBkMode(gHdc, TRANSPARENT);
	HFONT* pOldFont = (HFONT*)SelectObject(gHdc, Font);
	SetTextColor(gHdc, Color);
	TextOut(gHdc, x, y, pString, strlen(pString));
	SelectObject(gHdc, pOldFont);
}

void __PutTextf(int x, int y, COLORREF Color, HFONT Font, const char *format, ...)
{
	va_list argptr;
	char    buf[512];

	va_start(argptr, format);	//�޾ƿ� ���ڿ� ���˿� ������ �о�
	vsprintf(buf, format, argptr); //���ۿ� ���ڿ��� ������ �˸°� �ִ´�
	va_end(argptr);

	SetBkMode(gHdc, TRANSPARENT);	//�޹���� �����ϰ�
	HFONT* pOldFont = (HFONT*)SelectObject(gHdc, Font);

	//���� ���� ���
	SetTextColor(gHdc, Color);
	TextOut(gHdc, x, y, buf, strlen(buf));

	SelectObject(gHdc, pOldFont);
}

/*ǥ���� ������ ������ ä��*/
void _FillSurface(LPDIRECTDRAWSURFACE7 lpDDSDest, DWORD dwColor)
{
	DDBLTFX ddbltfx; //��Ʈ �ϴµ� �ʿ��� ������ ��� ����ü

	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(DDBLTFX);
	ddbltfx.dwFillColor = dwColor;

	HRESULT ddrval = lpDDSDest->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

void _FillRectEx(LPDIRECTDRAWSURFACE7 lpDDSDest, LPRECT lpRect, DWORD dwColor)
{
	DDBLTFX ddbltfx;

	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = dwColor;

	HRESULT ddrval = lpDDSDest->Blt(lpRect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

struct GameCode {};
//���� ��������ڵ�
void GameCode(int state)
{
	static BOOL b_press = TRUE; //����ȭ�� ��¦�̴� ����
	static BOOL b_once = TRUE; //���ӽ��۽� �ѹ��� �����ϱ� ����
	static BOOL b_Record; //��� ���Ž� ���Ϲ��ڿ� �������
	BOOL b_Crash = FALSE; //�浹����
	int n_ECount; //������
	
	switch (nState) //ȭ�� ����
	{
	case en_MO : //���� ȭ�� complete
		__GetDC(dxd.DxD_getBackSur());
		__PutText(200, 100, WHITE, "DOT RUN", gFTitle); //���� ���

		if (b_press) //press any key ����
		{
			__PutText(260, 350, WHITE, "Press Any Key", gFContent);

			if (dTIMEGETTIME - dw_main > 1.000)
			{
				b_press = FALSE;
			}
		}
		else if (!b_press)
		{
			if (dTIMEGETTIME - dw_main > 2.000)
			{
				b_press = TRUE;
				dw_main = dTIMEGETTIME;
			}
		}

		if (gBScore)
		{
			__PutTextf(250, 200, WHITE, gFContent, "Best Score : %d", gBScore);
		}

		__ReleaseDC(dxd.DxD_getBackSur());
		break;
	case en_GB:
		__GetDC(dxd.DxD_getBackSur());
		__PutText(150, 100, WHITE, "How to Play?", gFTitle); //���� ���

		__PutText(200, 200, WHITE, "Move : �����, Arrow Keys", gFContent);
		__PutText(200, 230, WHITE, "Using Item : Space Bar", gFContent);

		__ReleaseDC(dxd.DxD_getBackSur());

		break;
	////////////// ����ȭ�� ////////////////
	case en_GO : 	
		if (b_once)//���� ������ �� �ѹ��� �����ϴ� �ڵ�
		{
			GameInit();
			b_once = FALSE;
			b_Record = FALSE;
		}

		////////*�� ���� �ڵ�*/////////
		//���� �ø��� �ڵ�
		if (dTIMEGETTIME - dw_LastGene > 5.000)
		{
			Enemy.Generation(Enemy.GetCount());
			dw_LastGene = dTIMEGETTIME;
		}
		//�� Ŀ���� �ڵ�
		if (dTIMEGETTIME - dw_LastSzUp > 17.000)
		{
			Enemy.SzUp();
			dw_LastSzUp = dTIMEGETTIME;
		}

		n_ECount = Enemy.GetCount();
		LoopEnemy(n_ECount); //�� �����̴°Ŷ� �׸��� �Լ�, ������ ���̷��� �ѹ��� ����

		////*�÷��̾� ���� �ڵ�*////
		//Hero �̵� �ڵ�(ȭ�� ������ �������� ó������)
		if (::GetKeyState(VK_LEFT) & 0x80 && g_HeroPos.x > 0) g_HeroPos.x -= 5;
		if (::GetKeyState(VK_RIGHT) & 0x80 && g_HeroPos.x + 20 < HRES) g_HeroPos.x += 5;
		if (::GetKeyState(VK_UP) & 0x80 && g_HeroPos.y > 0) g_HeroPos.y -= 5;
		if (::GetKeyState(VK_DOWN) & 0x80 && g_HeroPos.y + 20 < VRES) g_HeroPos.y += 5;

		pfx.__PutSprite(dxd.DxD_getBackSur(), g_pDDSHero, g_HeroPos.x, g_HeroPos.y);

		//�浹 �ڵ�
		for (int i = 0; i < n_ECount; i++)
		{
			b_Crash = HeroCrash(g_HeroPos, Enemy.GetPos(i)); //�浹Ȯ��, ��ǥ�� Ȯ����, �浹������ TRUE
			if (b_Crash) //�浹Ȯ���ϰ� ���� ����
			{
				_FillSurface(dxd.DxD_getBackSur(), 0);
				nState = en_GOver;
				HHSound.Stop(g_pDS[0]);
				break;
			}
		}

		/////////////*������ ó�� �ڵ�*/////////////
		ItemCode();

		/////*���� �ڵ�*/////
		if (dTIMEGETTIME - dw_LastScore > 1.000) //1�ʸ��� ����
		{
			gScore += 10 * gScoreX; //��� �ʴ� 10��, ������1 �� ���� �ʴ� 20��
			dw_LastScore = dTIMEGETTIME;
		}
		__GetDC(dxd.DxD_getBackSur());
		__PutTextf(10, 10, WHITE, gFContent, "Score : %d", gScore); //���� ���
		__ReleaseDC(dxd.DxD_getBackSur());

		break;
		
		//////////////*���� ���� ȭ��*/////////////
	case en_GOver :
		__GetDC(dxd.DxD_getBackSur());
		__PutText(150, 100, WHITE, "GAME OVER", gFTitle); //gameover ���

		/*��ŷ ���� ���� Ȯ�� �� ���۾�*/
		if (!b_once && gScore != 0) //�ѹ��� �ص� �Ǵ°�, 0���ε� ����� ��� �����ڵ� �ߴ°� ����.
		{
			gbRanking = RankingCompare(gScore); //��ŷ������ ��� True, �̸��Է¿� ���Ѱŷ� �̸��Է� �Ϸ�� false����
			g_nSize = 0; 
			b_Record = gbRanking; //�ű�� �޼��ø� TRUE
		}

		if (b_Record) //���� ���Ž� ���η��� ���鼭 ��� �ؾ��ϴ°�
		{
			__PutText(270, 230, RED, "NEW RECORD!!", gFContent);
			__PutText(270, 250, WHITE, "Nickname : ", gFContent);
			__PutTextf(360, 250, WHITE, gFContent, "%s", gcName);
		}
		
		__PutTextf(270, 300, WHITE, gFContent, "Score : %d", gScore); //���� ���
		__PutText(270, 350, WHITE, "Press Space...", gFContent);
		__ReleaseDC(dxd.DxD_getBackSur());

		/*���Ӱ��� ������ ���� �罺ŸƮ�� ��츦 ����� �ʱ�ȭ*/
		if (!b_once) //�ѹ��� ó��
		{
			g_HeroPos = { 0 }; //�÷��̾� ��ġ �ʱ�ȭ
			Enemy.Clear(); //�� �ʱ�ȭ
			b_Crash = FALSE; //�浹����
			gItemKind = 0; //ȭ�鿡 �ִ� ������
			gnItem = 0; //���� ������
			b_once = TRUE; //���� ������Ҷ� �ѹ������ϴ� �ڵ� ������ϱ� ����
			
			HHSound.Play(g_pDS[1], FALSE);
		}
		break;

		/////////* ��ŷȭ�� �κ��߰� *//////////
	case en_Ranking : 
		__GetDC(dxd.DxD_getBackSur());
		__PutTextf(200, 20, WHITE, gFTitle, "RANKING");
		RankingView();
		
		__ReleaseDC(dxd.DxD_getBackSur());
		break;
	}

}

/*���� ���۽� �̰����� �����ϴ� �ڵ�*/
void GameInit()
{
	DWORD dwgm;

	//�÷��̾�� �ʱ�ȭ, ������ ��ġ
	gScore = 0;
	gScoreX = 10;
	g_HeroPos.x = 310;
	g_HeroPos.y = 220;

	/*�� �ʱ�ȭ*/
	Enemy.Init(); //���� �ʱ�ȭ
	Enemy.Generation(0);

	/*�ð�����*/
	dwgm = dTIMEGETTIME;
	dw_LastScore = dwgm; //������ ���������� ���� �ð�
	dw_itemTime = dwgm; //������ �⿬ �ð�
	dw_LastGene = dwgm; //���� ���������� ������ �ð�
	dw_LastSzUp = dwgm; //��ũ�� ���� �ð�
}

/*�ݺ��� ó�� �ϴ� �ڵ�, �� ����, ��ο�*/
void LoopEnemy(int count)
{
	int i;

	//�̷��� �����ν� ������ �ϳ� �پ��.
	for (i = 0; i < count; i++)
	{
		Enemy.Move(i, g_HeroPos);
		Enemy.Draw(i, dxd.DxD_getBackSur()); //���� �׸�
	}

}

/////////////*�浹�ڵ�*///////////////////////
BOOL HeroCrash(const POINT heroPos, const RECT EnePos)
{
	RECT RHero;

	SetRect(&RHero, heroPos.x, heroPos.y, heroPos.x + 17, heroPos.y + 17);

	if (RHero.left <= EnePos.right && RHero.right >= EnePos.left)
	{
		if (RHero.top <= EnePos.bottom && RHero.bottom >= EnePos.top)
		{
			return TRUE;
		}
	}

	return FALSE;
}

struct ItemCode{};
void ItemCode()
{
	static POINT Item_Pos; //ȭ�� �� ������ ��ġ
	RECT ItemInven;

	if (dTIMEGETTIME - dw_itemTime > 20.000) //20�ʸ��� ������ Ÿ�� 
	{
		if (gItemKind == 0)//�ȸ��� ������ ������ ���̻� ��Ÿ���� ���� ������ 0��
		{
			gItemKind = rand() % 2 + 1; //������ ���� �������� ������ ���ؼ�/������ ���� ȭ�鿡 �ִ� �������� ����
			Item_Pos.x = rand() % (HRES - 20); //ȭ�� �ȳѱ� ���ؼ� 
			Item_Pos.y = rand() % (VRES - 20);
		}
		dw_itemTime = dTIMEGETTIME;
	}

	if (gItemKind) //ȭ�鿡 �׸� �������� ������
	{
		ItemDraw(gItemKind, Item_Pos.x, Item_Pos.y);//�������� �׸�

		if (GetItems(g_HeroPos, Item_Pos) && !gnItem) //�浹���� ��� TRUE, �������� �������� ���� ��� TRUE
		{
			gnItem = gItemKind; //���� ������ ������ ������
			gItemKind = 0;
		}
	}

	////���� ������ ȭ�鿡 �׸���
	if (gnItem)
	{
		SetRect(&ItemInven, 610, 0, 630, 20);

		_FillRectEx(dxd.DxD_getBackSur(), &ItemInven, RGB(255, 255, 255));
		ItemDraw(gnItem, 610, 0);
	}

	//������1ȿ�� ó��
	if (dTIMEGETTIME - dw_ItemUse > 10.000 && gScoreX != 1) //10��
	{
		gScoreX = 1; //���� ��� ���� ����
	}
	//������2ȿ�� ó��
	else if (dTIMEGETTIME - dw_ItemUse > 3.000 && Enemy.GetCount() == 0) //3������ �ƹ����� ����
	{
		Enemy.Generation(0);
	}
}

/*�÷��̾ �������� �Դ�(�浹) �ڵ�*/
BOOL GetItems(const POINT heroPos, const POINT ItemPos)
{
	RECT RHero, RItem;
	BOOL bVer = FALSE, bHor = FALSE;

	SetRect(&RHero, heroPos.x, heroPos.y, heroPos.x + 18, heroPos.y + 18);
	SetRect(&RItem, ItemPos.x, ItemPos.y, ItemPos.x + 18, ItemPos.y + 18);

	if (RHero.left <= RItem.right && RHero.right >= RItem.left)
	{
		bHor = TRUE;
	}

	if (RHero.top <= RItem.bottom && RHero.bottom >= RItem.top)
	{
		bVer = TRUE;
	}

	if (bHor && bVer)
	{
		return TRUE;
	}

	return FALSE;
}

//������ �׸���
void ItemDraw(int Kind, int x, int y)
{
	switch (Kind)
	{
	case 1: //��, �����ð����� ���� �ι�� ����
		pfx.__PutSprite(dxd.DxD_getBackSur(), g_pDDSItem1, x, y);
		break;
	case 2: //��Ʈ, ���� ��� ������
		pfx.__PutSprite(dxd.DxD_getBackSur(), g_pDDSItem2, x, y);
		break;
	}
}

//������ ���
void ItemUse(int ItemKind)
{
	switch (ItemKind)
	{
	case 1 ://��, �����ð����� ���� �ι�� ����
		dw_ItemUse = dTIMEGETTIME;
		gScoreX = 2;//���� ����� �ø�
		break;
	case 2 : //��Ʈ, ���� ��� ������
		gScore += Enemy.GetCount() * 10; //���ϳ��� 10�����ļ� ���� �߰���
		Enemy.Clear();
		dw_ItemUse = dTIMEGETTIME;
		break;
	}
}

struct RankView{};
void RankingView()
{
	static Ranking ranking[10];

	int i, y = 100;

	if (!gb_Once)
	{
		gb_Once = RankingReading(ranking);
	}
	else
	{
		for (i = 0; i < g_nSize; i++)
		{
			__PutTextf(30, y + (20 * i), WHITE, gFContent, "No.%d \t %d \t %s\n",
				i + 1, ranking[i].score, ranking[i].name);
		}
	}

}