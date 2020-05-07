#include "DxDClass.h"

DxDClass::DxDClass()
{
}

DxDClass::~DxDClass()
{

}

/*DD객체 선언 생성*/
BOOL DxDClass::DD_Init(DWORD Wid, DWORD Hei, DWORD BPP, DWORD BufCount)
{
	BOOL b_Res;

#ifdef _DEBUG
	b_Res = DxD_InitWin(Wid, Hei); //노말모드로 DD객체 생성
	if (!b_Res)
	{
		b_Res = DxD_InitFull(Wid, Hei, BPP, BufCount); //풀 스크린 모드로 DD객체 생성
		if (!b_Res) return FALSE;
	}
#else //릴리즈모드
	b_Res = DxD_InitFull(Wid, Hei, BPP, BufCount);
	if (!b_Res) return FALSE;
#endif

	return TRUE;
}

/*풀스크린 모드 DD객체 생성, 삼중 버퍼링 & Flip*/
BOOL DxDClass::DxD_InitFull(DWORD Hres, DWORD Vres, DWORD BPP, DWORD bBufCount)
{
	/*DD객체 생성*/
	hResult = DirectDrawCreateEx(NULL, (void**)&DD_Obj, IID_IDirectDraw7, NULL);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*디스플레이 협력 설정*/
	hResult = DD_Obj->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*디스플레이 설정*/
	hResult = DD_Obj->SetDisplayMode(Hres, Vres, BPP, 0, 0);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*표면 기술 설정 및 1차 표면 생성*/
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
	ddsd.dwBackBufferCount = bBufCount;

	hResult = DD_Obj->CreateSurface(&ddsd, &Primary_Sur, NULL);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*2차 표면 능력 설정 및 2차 표면 생성*/
	ZeroMemory(&ddcaps, sizeof(ddcaps));
	ddcaps.dwCaps = DDSCAPS_BACKBUFFER;

	hResult = Primary_Sur->GetAttachedSurface(&ddcaps, &Back_Sur);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	___Trace("Full mode\n");
	b_full = TRUE;

	return TRUE;
}

/*노말 모드 DD객체 생성, 더블 버퍼링 & Blt*/
BOOL DxDClass::DxD_InitWin(DWORD Hres, DWORD Vres)
{
	/*DD객체 생성*/
	hResult = DirectDrawCreateEx(NULL, (void**)&DD_Obj, IID_IDirectDraw7, NULL);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*디스플레이 협력 설정*/
	hResult = DD_Obj->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*1차 표면 기술 설정 및 생성*/
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	hResult = DD_Obj->CreateSurface(&ddsd, &Primary_Sur, NULL);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	/*2차 표면 능력 설정 및 2차 표면 생성*/
	Back_Sur = DxD_CreateSurface(Hres, Vres, TRUE);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	___Trace("Normal mode\n");
	b_full = FALSE;

	return TRUE;
}

/*오프스크린 생성*/
LPDIRECTDRAWSURFACE7 DxDClass::DxD_CreateSurface(DWORD dwWidth, DWORD dwHeight, BOOL bIsVram)
{
	/*표면 기술 설정*/
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	if (bIsVram)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = dwWidth;
	ddsd.dwHeight = dwHeight;

	/*표면 생성*/
	hResult = DD_Obj->CreateSurface(&ddsd, &Off_Sur, NULL);
	if (hResult == DDERR_OUTOFVIDEOMEMORY)
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		hResult = DD_Obj->CreateSurface(&ddsd, &Off_Sur, NULL);
	}
	if (hResult != DD_OK) { DDERRCHK(hResult); return NULL; }

	return Off_Sur;
}

/*클리핑, 클리퍼 설정, 리스트 설정, 윈도우 클리핑*/
BOOL DxDClass::DxD_CSetClipper(LPDIRECTDRAWSURFACE7 lpDDS, RECT rc)
{
	/*클리퍼 객체 생성*/
	if (!DD_Clipper)
	{
		hResult = DirectDrawCreateClipper(0, &DD_Clipper, NULL);
		if (hResult != DD_OK) return DDERRCHK(hResult);
	}

	/*RGNDATA 설정*/
	rgd = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER) + sizeof(RECT));

	//rgdHeader설정
	rgd->rdh.dwSize = sizeof(rgd->rdh);
	rgd->rdh.iType = RDH_RECTANGLES;
	rgd->rdh.nCount = 1;
	rgd->rdh.nRgnSize = sizeof(RECT);
	SetRect(&(rgd->rdh.rcBound), rc.left, rc.top, rc.right, rc.bottom);
	SetRect((RECT*)(rgd->Buffer), rc.left, rc.top, rc.right, rc.bottom);

	/*클립리스트 설정*/
	hResult = DD_Clipper->SetClipList(rgd, 0);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	hResult = lpDDS->SetClipper(DD_Clipper);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	free(rgd); //메모리 할당 해제
	DD_Clipper->Release();
	DD_Clipper = NULL;

	return TRUE;
}

BOOL DxDClass::DxD_SetClipperList(LPDIRECTDRAWSURFACE7 lpDDS, LPRECT rcClipList, int nCount)
{
	RECT bc;
	int i;

	/*클리퍼 객체 생성*/
	if (!DD_Clipper)
	{
		hResult = DirectDrawCreateClipper(0, &DD_Clipper, NULL);
		if (hResult != DD_OK) return DDERRCHK(hResult);
	}

	/*RGNDATA 설정*/
	rgd = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER) + sizeof(RECT) * nCount);

	//rgdHeader설정
	rgd->rdh.dwSize = sizeof(rgd->rdh);
	rgd->rdh.iType = RDH_RECTANGLES;
	rgd->rdh.nCount = nCount;
	rgd->rdh.nRgnSize = sizeof(RECT) * nCount;

	//Boundary 설정
	SetRect(&bc, rcClipList[0].left, rcClipList[0].top, rcClipList[0].right, rcClipList[0].bottom);

	for (i = 1; i < nCount - 1; i++)
	{
		if (bc.left > rcClipList[i].left)
			bc.left = rcClipList[i].left;

		if (bc.right < rcClipList[i].right)
			bc.right = rcClipList[i].right;

		if (bc.top > rcClipList[i].top)
			bc.top = rcClipList[i].top;

		if (bc.bottom < rcClipList[i].bottom)
			bc.bottom = rcClipList[i].bottom;
	}

	SetRect(&(rgd->rdh.rcBound), bc.left, bc.top, bc.right, bc.bottom);
	memcpy(rgd->Buffer, rcClipList, sizeof(RECT) * nCount);

	/*클립리스트 설정*/
	hResult = DD_Clipper->SetClipList(rgd, 0);
	if (hResult != DD_OK) return DDERRCHK(hResult);
	hResult = lpDDS->SetClipper(DD_Clipper);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	free(rgd); //메모리 할당 풀어줌
	DD_Clipper->Release();
	DD_Clipper = NULL;

	return TRUE;
}

BOOL DxDClass::DxD_SetWindowClipper(LPDIRECTDRAWSURFACE7 lpDDS, HWND hwnd)
{
	/*클리퍼 객체 생성*/
	if (!DD_Clipper)
	{
		hResult = DirectDrawCreateClipper(0, &DD_Clipper, NULL);
		if (hResult != DD_OK) return DDERRCHK(hResult);
	}

	/*클립핑 설정*/
	hResult = DD_Clipper->SetHWnd(0, hwnd);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	hResult = lpDDS->SetClipper(DD_Clipper);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	DD_Clipper->Release();
	DD_Clipper = NULL;

	return TRUE;
}

/*더블 버퍼링 또는 플립*/
void DxDClass::DxD_BltFlip()
{
	if (Primary_Sur->IsLost() == DDERR_SURFACELOST) Primary_Sur->Restore();

#ifdef _DEBUG
	DD_Obj->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
	hResult = DxD_DBLT();
#else //Release
	hResult = Primary_Sur->Flip(NULL, DDFLIP_WAIT);
#endif
	if (hResult == DDERR_SURFACELOST) DD_Obj->RestoreAllSurfaces();
	if (hResult != DD_OK) { DDERRCHK(hResult); return; }

}

/*더블 버퍼링을 위한 블리트*/
BOOL DxDClass::DxD_DBLT()
{
	RECT rc;
	POINT pt = { 0 };

	/*윈도우 작업 공간 크기와 화면에서의 위치, 위치에 맞게 좌표 옮김*/
	GetClientRect(hWnd, &rc);
	ClientToScreen(hWnd, &pt);
	OffsetRect(&rc, pt.x, pt.y);

	hResult = Primary_Sur->Blt(&rc, Back_Sur, NULL, DDBLT_WAIT, NULL);
	if (hResult != DD_OK) return DDERRCHK(hResult);

	return hResult;
}

LPDIRECTDRAWSURFACE7 DxDClass::DxD_getPrimarySur()
{
	return Primary_Sur;
}
LPDIRECTDRAWSURFACE7 DxDClass::DxD_getBackSur()
{
	return Back_Sur;
}

LPDIRECTDRAW7 DxDClass::DxD_getDDobj()
{
	return DD_Obj;
}

void DxDClass::DxD_ObjRelease()
{
	if (DD_Obj != NULL)
	{
		if (Primary_Sur != NULL)
		{
			Primary_Sur->Release();
			Primary_Sur = NULL;
		}

		if (Off_Sur != NULL)
		{
			if (Back_Sur == Off_Sur) //
			{
				___Trace0("같을때");
				Back_Sur = NULL;
			}

			Off_Sur->Release();
			Off_Sur = NULL;
		}

		if (Back_Sur != NULL && !b_full) //풀모드가 아닐때
		{
			Back_Sur->Release();
			Back_Sur = NULL;
		}

		DD_Obj->Release();
		DD_Obj = NULL;
	}
}