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

#define dTIMEGETTIME timeGetTime() * 0.001 //시간 세기 단위 초
#define COLORKEY RGB(0, 0, 0)

//TODO : 코드 정리
/*전역 변수*/
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

int nState; //프로그램 상태
DWORD dw_LastScore, dw_itemTime, dw_ItemUse; //마지막 스코어 갱신 시간, 아이템 출연간격, 아이템 사용시작 시간
DWORD dw_LastGene, dw_LastSzUp; //마지막으로 적 늘린 시간
DWORD dw_main;

//게임온
EnemyClass Enemy;//적
int gScore, gBScore = 0, gScoreX; //점수, 베스트 스코어, 점수 배수
int gItemKind = 0; //화면에 있는 아이템 종류
int gnItem = 0; //입수한 아이템 여부및 종류 없을 땐 0임

//랭킹, 게임오버 관련
char gcName[256] = { 0 }; //기록갱신 시 이름 등록될 변수
BOOL gbRanking; //기록 갱신시 이름을 입력안하면 화면이 넘어가지 않게함. TRUE일동안 이름을 입력하고 엔터를 치면 FALSE됨
BOOL gb_Once = FALSE;

LPDIRECTSOUNDBUFFER g_pDS[2];//사운드 버퍼

/*윈도우 클래스 초기화 및 생성*/
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

	/*사운드 처리*/
	HHSound.Init(hWnd); //사운드 객체 초기화

	//메인 브금
	HHSound.LoadWave("sound/sixthsense.wav", &g_pDS[0]);
	HHSound.SetVolume(g_pDS[0], 100);
	//게임오버
	HHSound.LoadWave("sound/pianoboom.wav", &g_pDS[1]);
	HHSound.SetVolume(g_pDS[1], 85);

	res = DXW_Init();
	if (!res) return FALSE;

	////*파일 읽어와서 베스트 스코어 가져오기, 없으면 베스트 스코어 값 0임*////
	gBScore = RankBS();

	return TRUE;
}

/*메인 함수*/
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
			if (!GetMessage(&msg, NULL, 0, 0)) break; //WM_QUIT가 오면 빠져나감

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (gAppSet.bIsActive && !gAppSet.bIsPause) //메시지가 없고 윈도우가 활성화일 때만 메인루프를 돌림
		{
			MainLoop();
		}
		else WaitMessage();
	}
}

/*윈도우 프로시저 함수*/
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
		__WinDestroy(); //객체 해제
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
	if (nState == en_MO && wParam != VK_ESCAPE) //화면 전환 메인->게임방법
	{
		nState = en_GB;
		return;
	}
	else if (nState == en_GB && wParam != VK_ESCAPE) //화면 전환 게임방법->게임화면
	{
		nState = en_GO;
		HHSound.Stop(g_pDS[0]);
		HHSound.Play(g_pDS[0], TRUE);
		return;
	}
	else if (nState == en_Ranking && wParam != VK_ESCAPE) //화면 전환 랭킹->메인화면
	{
		nState = en_MO;
		HHSound.Play(g_pDS[0], TRUE); //메인음악 재생
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
		if (nState == en_GOver && !gbRanking) //화면 전환 게임오버->랭킹
		{
			nState = en_Ranking;
			//랭킹화면에서 쓸 변수라 초기화
			g_nSize = 0;
			gb_Once = FALSE;

			memset(gcName, 0, sizeof(gcName)); //다음판 입력을 위해 비움

			HHSound.Stop(g_pDS[1]);
			return;
		}
		
		if (nState == en_GO && gnItem) //아이템이 있을 때만 처리
		{
			ItemUse(gnItem); //아이템을 사용함
			gnItem = 0; //소유한 아이템 없게 함
		}

		break;
	case VK_RETURN: 
 		if (nState == en_GOver && gcName[0])
		{
			gBScore = RankUp(gScore, gcName); //랭크 등록및 1등값 받아와서 설정
			gbRanking = FALSE; //이름 입력 끝을 알림, 화면 넘어가는게 가능
			
		}
		break;
	}
}

/*객체 해제*/
void __WinDestroy()
{
	__DestroyFont(); //폰트 해제
	Enemy.Clear();

	//Dx객체 해제
	dxd.DxD_ObjRelease();

	//메모리누수 확인
	_CrtDumpMemoryLeaks();

	TRACE0("__WinDestroy\n");
}

BOOL DXW_Init(void)
{
	BOOL bRval;
	RECT RClient = {0, 0, HRES, VRES};

	/*DD객체 생성*/
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

	////주인공 이미지 가져와서 표면에 적재(오프스크린)
	g_pDDSHero = DDLoadBitmap(dxd.DxD_getDDobj(), "hero.bmp", 0, 0);
	if (!g_pDDSHero) return ___Error("DD_Init()", __FILE__, __LINE__);
	DDSetColorKey(g_pDDSHero, COLORKEY);

	////아이템 이미지 가져와서 표면에 적재(오프스크린), 2개임(별, 하트)
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
/*메인 루프*/
void MainLoop()
{
	HRESULT hRet;
	
	_FillSurface(dxd.DxD_getBackSur(), 0); //화면 클리어

	GameCode(nState); //게임 처리 코드

#ifdef _DISPLAYINFO
	__DisplayInfo();
#endif // _DISPLAYINFO

	dxd.DxD_BltFlip();//화면 전환

	gFpsMng.FrameWaiting();
}

/*현재 디플레이 정보를 출력함*/
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
	lpDDSDest->GetDC(&gHdc); //HDC는 전역변수
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

/*문자를 출력함*/
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

	va_start(argptr, format);	//받아온 문자열 포맷에 변수를 읽어
	vsprintf(buf, format, argptr); //버퍼에 문자열과 변수를 알맞게 넣는다
	va_end(argptr);

	SetBkMode(gHdc, TRANSPARENT);	//뒷배경이 투명하게
	HFONT* pOldFont = (HFONT*)SelectObject(gHdc, Font);

	//본래 문자 출력
	SetTextColor(gHdc, Color);
	TextOut(gHdc, x, y, buf, strlen(buf));

	SelectObject(gHdc, pOldFont);
}

/*표면을 지정한 색으로 채움*/
void _FillSurface(LPDIRECTDRAWSURFACE7 lpDDSDest, DWORD dwColor)
{
	DDBLTFX ddbltfx; //블리트 하는데 필요한 정보를 담는 구조체

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
//게임 진행관련코드
void GameCode(int state)
{
	static BOOL b_press = TRUE; //메인화면 반짝이는 문자
	static BOOL b_once = TRUE; //게임시작시 한번만 수행하기 위함
	static BOOL b_Record; //기록 갱신시 축하문자열 출력위함
	BOOL b_Crash = FALSE; //충돌여부
	int n_ECount; //적개수
	
	switch (nState) //화면 상태
	{
	case en_MO : //메인 화면 complete
		__GetDC(dxd.DxD_getBackSur());
		__PutText(200, 100, WHITE, "DOT RUN", gFTitle); //제목 출력

		if (b_press) //press any key 깜빡
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
		__PutText(150, 100, WHITE, "How to Play?", gFTitle); //제목 출력

		__PutText(200, 200, WHITE, "Move : ←↑↓→, Arrow Keys", gFContent);
		__PutText(200, 230, WHITE, "Using Item : Space Bar", gFContent);

		__ReleaseDC(dxd.DxD_getBackSur());

		break;
	////////////// 게임화면 ////////////////
	case en_GO : 	
		if (b_once)//게임 시작할 때 한번만 실행하는 코드
		{
			GameInit();
			b_once = FALSE;
			b_Record = FALSE;
		}

		////////*적 관련 코드*/////////
		//적을 늘리는 코드
		if (dTIMEGETTIME - dw_LastGene > 5.000)
		{
			Enemy.Generation(Enemy.GetCount());
			dw_LastGene = dTIMEGETTIME;
		}
		//적 커지는 코드
		if (dTIMEGETTIME - dw_LastSzUp > 17.000)
		{
			Enemy.SzUp();
			dw_LastSzUp = dTIMEGETTIME;
		}

		n_ECount = Enemy.GetCount();
		LoopEnemy(n_ECount); //적 움직이는거랑 그리는 함수, 루프문 줄이려고 한번에 돌림

		////*플레이어 관련 코드*////
		//Hero 이동 코드(화면 밖으로 못나가게 처리했음)
		if (::GetKeyState(VK_LEFT) & 0x80 && g_HeroPos.x > 0) g_HeroPos.x -= 5;
		if (::GetKeyState(VK_RIGHT) & 0x80 && g_HeroPos.x + 20 < HRES) g_HeroPos.x += 5;
		if (::GetKeyState(VK_UP) & 0x80 && g_HeroPos.y > 0) g_HeroPos.y -= 5;
		if (::GetKeyState(VK_DOWN) & 0x80 && g_HeroPos.y + 20 < VRES) g_HeroPos.y += 5;

		pfx.__PutSprite(dxd.DxD_getBackSur(), g_pDDSHero, g_HeroPos.x, g_HeroPos.y);

		//충돌 코드
		for (int i = 0; i < n_ECount; i++)
		{
			b_Crash = HeroCrash(g_HeroPos, Enemy.GetPos(i)); //충돌확인, 좌표로 확인함, 충돌했으면 TRUE
			if (b_Crash) //충돌확인하고 게임 끝냄
			{
				_FillSurface(dxd.DxD_getBackSur(), 0);
				nState = en_GOver;
				HHSound.Stop(g_pDS[0]);
				break;
			}
		}

		/////////////*아이템 처리 코드*/////////////
		ItemCode();

		/////*점수 코드*/////
		if (dTIMEGETTIME - dw_LastScore > 1.000) //1초마다 득점
		{
			gScore += 10 * gScoreX; //평소 초당 10점, 아이템1 별 사용시 초당 20점
			dw_LastScore = dTIMEGETTIME;
		}
		__GetDC(dxd.DxD_getBackSur());
		__PutTextf(10, 10, WHITE, gFContent, "Score : %d", gScore); //점수 출력
		__ReleaseDC(dxd.DxD_getBackSur());

		break;
		
		//////////////*게임 오버 화면*/////////////
	case en_GOver :
		__GetDC(dxd.DxD_getBackSur());
		__PutText(150, 100, WHITE, "GAME OVER", gFTitle); //gameover 출력

		/*랭킹 점수 갱신 확인 및 밑작업*/
		if (!b_once && gScore != 0) //한번만 해도 되는거, 0점인데 기록이 없어서 뉴레코드 뜨는거 방지.
		{
			gbRanking = RankingCompare(gScore); //랭킹순위에 들면 True, 이름입력에 관한거로 이름입력 완료시 false가됨
			g_nSize = 0; 
			b_Record = gbRanking; //신기록 달성시만 TRUE
		}

		if (b_Record) //점수 갱신시 메인루프 돌면서 계속 해야하는거
		{
			__PutText(270, 230, RED, "NEW RECORD!!", gFContent);
			__PutText(270, 250, WHITE, "Nickname : ", gFContent);
			__PutTextf(360, 250, WHITE, gFContent, "%s", gcName);
		}
		
		__PutTextf(270, 300, WHITE, gFContent, "Score : %d", gScore); //점수 출력
		__PutText(270, 350, WHITE, "Press Space...", gFContent);
		__ReleaseDC(dxd.DxD_getBackSur());

		/*게임관련 변수들 게임 재스타트할 경우를 대비한 초기화*/
		if (!b_once) //한번만 처리
		{
			g_HeroPos = { 0 }; //플레이어 위치 초기화
			Enemy.Clear(); //적 초기화
			b_Crash = FALSE; //충돌여부
			gItemKind = 0; //화면에 있는 아이템
			gnItem = 0; //먹은 아이템
			b_once = TRUE; //게임 재시작할때 한번수행하는 코드 재실행하기 위함
			
			HHSound.Play(g_pDS[1], FALSE);
		}
		break;

		/////////* 랭킹화면 부분추가 *//////////
	case en_Ranking : 
		__GetDC(dxd.DxD_getBackSur());
		__PutTextf(200, 20, WHITE, gFTitle, "RANKING");
		RankingView();
		
		__ReleaseDC(dxd.DxD_getBackSur());
		break;
	}

}

/*게임 시작시 이것저것 세팅하는 코드*/
void GameInit()
{
	DWORD dwgm;

	//플레이어블 초기화, 점수랑 위치
	gScore = 0;
	gScoreX = 10;
	g_HeroPos.x = 310;
	g_HeroPos.y = 220;

	/*적 초기화*/
	Enemy.Init(); //변수 초기화
	Enemy.Generation(0);

	/*시간관리*/
	dwgm = dTIMEGETTIME;
	dw_LastScore = dwgm; //점수를 마지막으로 얻은 시간
	dw_itemTime = dwgm; //아이템 출연 시간
	dw_LastGene = dwgm; //적이 마지막으로 생성한 시간
	dw_LastSzUp = dwgm; //적크기 갱신 시간
}

/*반복문 처리 하는 코드, 적 무브, 드로우*/
void LoopEnemy(int count)
{
	int i;

	//이렇게 함으로써 포문이 하나 줄어듬.
	for (i = 0; i < count; i++)
	{
		Enemy.Move(i, g_HeroPos);
		Enemy.Draw(i, dxd.DxD_getBackSur()); //적을 그림
	}

}

/////////////*충돌코드*///////////////////////
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
	static POINT Item_Pos; //화면 상 아이템 위치
	RECT ItemInven;

	if (dTIMEGETTIME - dw_itemTime > 20.000) //20초마다 아이템 타임 
	{
		if (gItemKind == 0)//안먹은 아이템 있으면 더이상 나타나지 않음 없을때 0임
		{
			gItemKind = rand() % 2 + 1; //아이템 종류 랜덤으로 돌리기 위해서/변수는 현재 화면에 있는 아이템의 종류
			Item_Pos.x = rand() % (HRES - 20); //화면 안넘기 위해서 
			Item_Pos.y = rand() % (VRES - 20);
		}
		dw_itemTime = dTIMEGETTIME;
	}

	if (gItemKind) //화면에 그릴 아이템이 있으면
	{
		ItemDraw(gItemKind, Item_Pos.x, Item_Pos.y);//아이템을 그림

		if (GetItems(g_HeroPos, Item_Pos) && !gnItem) //충돌했을 경우 TRUE, 소유중인 아이템이 없을 경우 TRUE
		{
			gnItem = gItemKind; //먹은 아이템 종류를 저장함
			gItemKind = 0;
		}
	}

	////먹은 아이템 화면에 그리기
	if (gnItem)
	{
		SetRect(&ItemInven, 610, 0, 630, 20);

		_FillRectEx(dxd.DxD_getBackSur(), &ItemInven, RGB(255, 255, 255));
		ItemDraw(gnItem, 610, 0);
	}

	//아이템1효과 처리
	if (dTIMEGETTIME - dw_ItemUse > 10.000 && gScoreX != 1) //10초
	{
		gScoreX = 1; //점수 배수 원상 복귀
	}
	//아이템2효과 처리
	else if (dTIMEGETTIME - dw_ItemUse > 3.000 && Enemy.GetCount() == 0) //3초정도 아무적도 없음
	{
		Enemy.Generation(0);
	}
}

/*플레이어가 아이템을 먹는(충돌) 코드*/
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

//아이템 그리기
void ItemDraw(int Kind, int x, int y)
{
	switch (Kind)
	{
	case 1: //별, 일정시간동안 점수 두배로 얻음
		pfx.__PutSprite(dxd.DxD_getBackSur(), g_pDDSItem1, x, y);
		break;
	case 2: //하트, 적을 모두 날려줌
		pfx.__PutSprite(dxd.DxD_getBackSur(), g_pDDSItem2, x, y);
		break;
	}
}

//아이템 사용
void ItemUse(int ItemKind)
{
	switch (ItemKind)
	{
	case 1 ://별, 일정시간동안 점수 두배로 얻음
		dw_ItemUse = dTIMEGETTIME;
		gScoreX = 2;//점수 배수를 늘림
		break;
	case 2 : //하트, 적을 모두 날려줌
		gScore += Enemy.GetCount() * 10; //적하나당 10점씩쳐서 점수 추가함
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