#include "LockPixelFx.h"

#define RGB565 565
#define RGB888 888

PixelFx::PixelFx()
{
	
}

PixelFx::~PixelFx()
{
}

/*RGB 포맥하고 몇비트인지 알아냄*/
void PixelFx::__CheckRGBBit(LPDIRECTDRAWSURFACE7 lpDDS)
{
	DDSURFACEDESC2 ddsd;
	
	::ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_PIXELFORMAT;
	HRESULT hRet = lpDDS->GetSurfaceDesc(&ddsd);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	m_dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	switch (ddsd.ddpfPixelFormat.dwRBitMask) //R마스크로 포맷을 알아냄
	{
	case 0x0000F800: m_dwPixelFormat = RGB565; break;    // 11111000 00000000
	case 0x00FF0000: m_dwPixelFormat = RGB888; break;    // 11111111 00000000 00000000
	}

}

/*----------------------------------------------------------------------------*/
/*락처리*/
inline BOOL PixelFx::__Lock(LPDIRECTDRAWSURFACE7 pDDSDest)
{
	::ZeroMemory(&m_ddsd, sizeof(DDSURFACEDESC2));
	m_ddsd.dwSize = sizeof(DDSURFACEDESC2);
	HRESULT hRet = pDDSDest->Lock(NULL, &m_ddsd, DDLOCK_WAIT, NULL);
	if (hRet != DD_OK) { return DDERRCHK(hRet); }

	return TRUE;
}

/*언락*/
inline BOOL PixelFx::__Unlock(LPDIRECTDRAWSURFACE7 pDDSDest)
{
	HRESULT hRet = pDDSDest->Unlock(NULL);
	if (hRet != DD_OK) { return DDERRCHK(hRet); }

	return TRUE;
}

//// Blt() 이미지
/*백버퍼 전체에 보낸 표면의 전체를 블리트함*/
void PixelFx::_BltSurface(LPDIRECTDRAWSURFACE7 lpDestSur, LPDIRECTDRAWSURFACE7 lpSrcSur) 
{
	DDBLTFX ddbltfx;

	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(DDBLTFX);

	HRESULT ddrval = lpDestSur->Blt(NULL, lpSrcSur, NULL, DDBLT_WAIT, &ddbltfx);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*표면채로 백버퍼에 블리트함, 클리핑X, 컬러키X*/
void PixelFx::__PutImage(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	//// 표면의 크기 정보를 얻는다 ( 임시코드 )
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	/////
	RECT rcDest = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	HRESULT ddrval = lpDDSDst->Blt(&rcDest, lpDDSSrc, NULL, DDBLT_WAIT, NULL);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*락안하고 표면채로 블리트함, 클리핑x, 컬러키 O*/
void PixelFx::__PutSprite(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	// 표면의 크기 정보를 얻는다
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	RECT rcDest = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	HRESULT ddrval = lpDDSDst->Blt(&rcDest, lpDDSSrc, NULL, DDBLT_KEYSRC | DDBLT_WAIT, NULL); //컬러키 처리
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*이미지를 블리트함, 클리핑 O, 컬러키 X*/
void PixelFx::__PutImageEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	//// 표면의 크기 정보를 얻는다 ( 임시코드 )
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	//// 스크린좌표의 RECT를 구한다
	RECT rcDst = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	RECT rcSrc = { 0, 0, ddsd.dwWidth, ddsd.dwHeight };

	//// 스크린 밖으로 나가면 출력하지 않는다
	if (x >= HRES || y >= VRES) return;
	if (rcDst.right <= 0 || rcDst.bottom <= 0) return;

	//// 스프라이트가 스크린 밖으로 나간 만큼 출력하지 않는다
	//// 오프 스크린의 사각영역을 감소 시켜 클리핑을 구현한다	

	//// 수평 조정
	if (x < 0)
	{
		// 좌변을 줄인다. rcSrc.left += 넘어간 크기, abs(x)
		rcSrc.left -= x;
		rcDst.left = x = 0;
	}
	else if (rcDst.right >= HRES)
	{
		// 우변을 줄인다. rcSrc.right -= 넘어간 크기
		int nOverSize = (rcDst.right - HRES);
		rcSrc.right -= nOverSize;
		rcDst.right -= nOverSize;
	}
	//// 수직 조정
	if (y < 0)
	{
		// 상변을 줄인다. rcSrc.top += 넘어간 크기, abs(y)
		rcSrc.top -= y;
		rcDst.top = y = 0;
	}
	else if (rcDst.bottom >= VRES)
	{
		// 하변을 줄인다. rcSrc.bottom -= 넘어간 크기 
		int nOverSize = (rcDst.bottom - VRES);
		rcSrc.bottom -= nOverSize;
		rcDst.bottom -= nOverSize;
	}

	//그결과를 2차표면에 블리트
	HRESULT ddrval = lpDDSDst->Blt(&rcDst, lpDDSSrc, &rcSrc, DDBLT_WAIT, NULL);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*이미지를 블리트함, 클리핑 O, 컬러키 O*/
void PixelFx::__PutSpriteEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	//// 표면의 크기 정보를 얻는다 ( 임시코드 )
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	//// 스크린좌표의 RECT를 구한다
	RECT rcDst = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	RECT rcSrc = { 0, 0, ddsd.dwWidth, ddsd.dwHeight };

	//// 스크린 밖으로 나가면 출력하지 않는다
	if (x >= HRES || y >= VRES) return;
	if (rcDst.right <= 0 || rcDst.bottom <= 0) return;

	//// 스프라이트가 스크린 밖으로 나간 만큼 출력하지 않는다
	//// 오프 스크린의 사각영역을 감소 시켜 클리핑을 구현한다	

	//// 수평 조정
	if (x < 0)
	{
		// 좌변을 줄인다. rcSrc.left += 넘어간 크기, abs(x)
		rcSrc.left -= x;
		rcDst.left = x = 0;
	}
	else if (rcDst.right >= HRES)
	{
		// 우변을 줄인다. rcSrc.right -= 넘어간 크기
		int nOverSize = (rcDst.right - HRES);
		rcSrc.right -= nOverSize;
		rcDst.right -= nOverSize;
	}
	//// 수직 조정
	if (y < 0)
	{
		// 상변을 줄인다. rcSrc.top += 넘어간 크기, abs(y)
		rcSrc.top -= y;
		rcDst.top = y = 0;
	}
	else if (rcDst.bottom >= VRES)
	{
		// 하변을 줄인다. rcSrc.bottom -= 넘어간 크기 
		int nOverSize = (rcDst.bottom - VRES);
		rcSrc.bottom -= nOverSize;
		rcDst.bottom -= nOverSize;
	}

	//블리트
	HRESULT ddrval = lpDDSDst->Blt(&rcDst, lpDDSSrc, &rcSrc, DDBLT_KEYSRC | DDBLT_WAIT, NULL); //위처리는 똑같고 블리트 옵션만 바꿈
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

//// Lock() 이미지
//// 주 의 : 목적표면(2차)이 Lock이 된 상태에서 함수 진입
//// 기 능 : 16, 32비트 모드 이미지 출력
//// lpDDS : 소스 표면
//// x, y  : 출력 좌표 
/*이미지(오프스크린)를 2차표면에 복사함, 클리핑 X, 컬러키 X*/
void PixelFx::__PutImageLock(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;

	memset(&ddsdSrc, 0, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);

	/*src 락*/
	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //오프스크린을 락함
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	///// BPP에 따라 연산 값을 취한다
	DWORD dwShift;
	switch (m_dwRGBBitCount)
	{
	case 16: dwShift = 1; break;
	case 32: dwShift = 2; break;
	}

	//// 이미지 표면에 대한 메모리 주소
	LPBYTE lpSrc = (LPBYTE)ddsdSrc.lpSurface;

	//// 2차 표면에 대한 메모리 주소 (전역)		
	LPBYTE lpDst = (LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << dwShift); //백버퍼에 대한 주소 값이며 백버퍼는 락한 상태

	DWORD dwWidth = ddsdSrc.dwWidth << dwShift; //픽셀당 바이트 수대로 곱해준 것(한 픽셀이 2바이트면 실제 가로 길이는 width*2기 때문
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// memcpy에 의한 메모리 전송
	for (DWORD iy = 0; iy < dwHeight; iy++)
	{
		memcpy(lpDst, lpSrc, dwWidth);
		lpSrc += ddsdSrc.lPitch;
		lpDst += m_ddsd.lPitch;
	}

	hRet = lpDDSSrc->Unlock(NULL); //언락
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

//// Lock() 스프라이트
//// 주 의 : 목적표면(2차)이 Lock이 된 상태에서 함수 진입
//// 기 능 : 16비트 모드 스프라이트 출력
//// lpDDS : 소스 표면
//// x, y  : 출력 좌표 

/*스프라이트 이미지를 2차표면에 넣음 16bit, 클리핑 X, 컬러키 O*/
void PixelFx::__PutSpriteLock16(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;

	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);

	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// 표면에 대한 메모리 주소
	WORD* lpSrc = (WORD*)ddsdSrc.lpSurface;

	//// 2차 표면에 대한 메모리 주소 (전역)
	WORD* lpDst = (WORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 1)); //출력할 위치로 이동

	//src크기
	DWORD dwWidth = ddsdSrc.dwWidth;
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// 컬러키
	DWORD dwSrcColorKey = ddsdSrc.ddckCKSrcBlt.dwColorSpaceLowValue; //컬러키를 받아옴
	DWORD dwSrc = 0;

	//// WORD(2바이트) 단위로 주소가 증가 되므로 RShift ( /2)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 1; //lpSrc가 Word단위라 Src + lPitch해버리면 사실상 src + (lPitch*2)랑 같아서 나눔
	DWORD dwDstPitch = m_ddsd.lPitch >> 1;

	//// 루프내에서는 최대한 연산을 최소화한다	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			dwSrc = *(lpSrc + ix);
			if (dwSrc != dwSrcColorKey) *(lpDst + ix) = (WORD)dwSrc; //컬러키가 아닌 값만 한픽셀씩 넣습니다.
		}
		lpSrc += dwSrcPitch; //라인 이동
		lpDst += dwDstPitch; //라인 이동
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

//// Lock() 스프라이트
//// 주 의 : 목적표면(2차)이 Lock이 된 상태에서 함수 진입
//// 기 능 : 32비트 모드 스프라이트 출력
//// lpDDS : 소스 표면
//// x, y  : 출력 좌표 

/*스프라이트 이미지를 2차표면에 넣음 32bit, 클리핑 X, 컬러키 O*/
void PixelFx::__PutSpriteLock32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;

	/*오프스크린(스프라이트이미지)에 대한 기술 */
	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);
	ddsdSrc.dwFlags = DDSD_CKSRCBLT;

	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //오프스크린을 락함
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// 표면에 대한 메모리 주소
	DWORD* lpSrc = (DWORD*)ddsdSrc.lpSurface; //오프스크린에 대한 주소

	//// 2차 표면에 대한 메모리 주소 ( 전역 ) , ( *4 )
	DWORD* lpDst = (DWORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 2)); //출력할 위치로 이동

	DWORD dwWidth = ddsdSrc.dwWidth; //오프스크린 가로세로
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// 컬러키
	DWORD dwSrcColorKey = ddsdSrc.ddckCKSrcBlt.dwColorSpaceLowValue; //컬러키 받아옴
	DWORD dwSrc = 0;

	//// DWORD 단위로 주소가 증가 되므로 RShift ( /4)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 2; //lpSrc가 DWord단위라 Src + lPitch해버리면 사실상 src + (lPitch*4)랑 같아서 나눔
	DWORD dwDstPitch = m_ddsd.lPitch >> 2;

	//// 루프내에서는 최대한 연산을 최소화한다	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			//// 최상위 8비트는 무시한다(XRGB, ARGB)
			dwSrc = *(lpSrc + ix) & 0x00FFFFFF;
			if (dwSrc != dwSrcColorKey) *(lpDst + ix) = dwSrc; //픽셀 하나씩 넣음
		}
		lpSrc += dwSrcPitch; //다음줄로(오프스크린)
		lpDst += dwDstPitch; // 다음 라인으로(2차 표면)
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

//// Lock() 이미지 & 스프라이트 출력 함수의 클립핑 처리 ( 소프트적인 구현 )
//// 기 능 : 16, 32비트 모드 스프라이트 출력
//// lpDDS : 소스 표면
//// x, y  : 출력 좌표

/*락하고 메모리적 접근으로 이미지 넣고 클리핑 처리*/
void PixelFx::__PutImageLockEx32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;
	DWORD dwSrc = 0;

	/*오프스크린(스프라이트이미지)에 대한 기술 */
	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);
	ddsdSrc.dwFlags = DDSD_CKSRCBLT;
	//오프스크린을 락함
	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //오프스크린은 읽기만함
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// 표면에 대한 메모리 주소
	DWORD* lpSrc = (DWORD*)ddsdSrc.lpSurface; //오프스크린에 대한 주소

	//// 2차 표면에 대한 메모리 주소 ( 전역 ) , ( *4 )
	DWORD* lpDst = (DWORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 2)); //출력할 위치로 이동

	DWORD dwWidth = ddsdSrc.dwWidth; //오프스크린 가로세로
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// DWORD 단위로 주소가 증가 되므로 RShift ( /4)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 2; //lpSrc가 DWord단위라 Src + lPitch해버리면 사실상 src + (lPitch*4)랑 같아서 나눔
	DWORD dwDstPitch = m_ddsd.lPitch >> 2;

	//// 스크린 밖으로 나가면 출력하지 않는다
	if (x >= HRES || y >= VRES)
	{
		hRet = lpDDSSrc->Unlock(NULL);
		if (hRet != DD_OK) { DDERRCHK(hRet); return; }
		return;
	}
	if (x + dwWidth <= 0 || y + dwHeight <= 0)
	{
		hRet = lpDDSSrc->Unlock(NULL);
		if (hRet != DD_OK) { DDERRCHK(hRet); return; }
		return;
	}

	//// 루프내에서는 최대한 연산을 최소화한다	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			//클리핑
			if (y + iy >= 0 && y + iy < VRES) //픽셀이 들어가는 위치를 파악해서 클리핑 될 부분만 넣는다.
			{
				if (x + ix >= 0 && x + ix < HRES)
				{
					//// 최상위 8비트는 무시한다(XRGB, ARGB)
					dwSrc = *(lpSrc + ix) & 0x00FFFFFF;
					*(lpDst + ix) = dwSrc; //픽셀 하나하나 넣음
				}
			}
		}

		lpSrc += dwSrcPitch; //다음줄로(오프스크린)
		lpDst += dwDstPitch; // 다음 라인으로(2차 표면)
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

/*락하고 메모리적 접근으로 이미지 넣고 컬러키 넣고 클리핑 처리하고..*/
void PixelFx::__PutSpriteLockEx32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;

	/*오프스크린(스프라이트이미지)에 대한 기술 */
	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);
	ddsdSrc.dwFlags = DDSD_CKSRCBLT;
	//오프스크린을 락함
	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //오프스크린은 읽기만함
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// 표면에 대한 메모리 주소
	DWORD* lpSrc = (DWORD*)ddsdSrc.lpSurface; //오프스크린에 대한 주소

	//// 2차 표면에 대한 메모리 주소 ( 전역 ) , ( *4 )
	DWORD* lpDst = (DWORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 2)); //출력할 위치로 이동

	DWORD dwWidth = ddsdSrc.dwWidth; //오프스크린 가로세로
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// DWORD 단위로 주소가 증가 되므로 RShift ( /4)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 2; //lpSrc가 DWord단위라 Src + lPitch해버리면 사실상 src + (lPitch*4)랑 같아서 나눔
	DWORD dwDstPitch = m_ddsd.lPitch >> 2;

	DWORD dwSrcColorKey = ddsdSrc.ddckCKSrcBlt.dwColorSpaceLowValue; //컬러키 받아옴
	DWORD dwSrc = 0;

	//// 스크린 밖으로 나가면 출력하지 않는다
	if (x >= HRES || y >= VRES)
	{
		hRet = lpDDSSrc->Unlock(NULL);
		if (hRet != DD_OK) { DDERRCHK(hRet); return; }
		return;
	}
	if (x + dwWidth <= 0 || y + dwHeight <= 0)
	{
		hRet = lpDDSSrc->Unlock(NULL);
		if (hRet != DD_OK) { DDERRCHK(hRet); return; }
		return;
	}

	//// 루프내에서는 최대한 연산을 최소화한다	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			//클리핑
			if (y + iy >= 0 && y + iy < VRES) //픽셀이 들어가는 위치를 파악해서 넣는다.
			{
				if (x + ix >= 0 && x + ix < HRES)
				{
					//// 최상위 8비트는 무시한다(XRGB, ARGB)
					dwSrc = *(lpSrc + ix) & 0x00FFFFFF;
					if (dwSrc != dwSrcColorKey)
						*(lpDst + ix) = dwSrc; //픽셀 하나하나 넣음
				}
			}
		}

		lpSrc += dwSrcPitch; //다음줄로(오프스크린)
		lpDst += dwDstPitch; // 다음 라인으로(2차 표면)
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

/*------------------------------------------------------------------------*/
// 좌표 공식 : y * 표면의 가로폭 + x
// y * 표면의 가로폭 + (x * 픽셀크기)
void PixelFx::__PutPixel16(int x, int y, WORD wColor)
{
	LPBYTE lpDDS = (LPBYTE)m_ddsd.lpSurface;
	WORD* lpDst = (WORD*)(lpDDS + y * m_ddsd.lPitch + (x * 2));

	*lpDst = wColor;
}

void PixelFx::__PutPixel32(int x, int y, DWORD dwColor)
{
	LPBYTE lpDDS = (LPBYTE)m_ddsd.lpSurface;

	DWORD* lpDst = (DWORD*)(lpDDS + y * m_ddsd.lPitch + (x * 4));

	*lpDst = dwColor;
}

WORD PixelFx::__GetPixel16(int x, int y)
{
	LPBYTE lpDDS = (LPBYTE)m_ddsd.lpSurface;
	WORD* pPixel = (WORD*)(lpDDS + y * m_ddsd.lPitch + (x * 2));

	return *pPixel;
}

/*32비트 픽셀값 받아오기 코드 추가*/
DWORD PixelFx::__GetPixel32(int x, int y)
{
	LPBYTE lpDDS = (LPBYTE)m_ddsd.lpSurface;
	DWORD* pPixel = (DWORD*)(lpDDS + y * m_ddsd.lPitch + (x * 4));

	return *pPixel;
}

//조합함수
DWORD PixelFx::__GetPixelRGB16(int R, int G, int B)
{
	DWORD wPixel = 0;

	if (m_dwPixelFormat == RGB565)
	{
		wPixel = (R << 11) | (G << 5) | B;   // 0000 0000 0001 1111		                                 
		return wPixel;					 // 0000 0111 1110 0000   5
	}                                    // 1111 1000 0000 0000  11
}

//조합함수
DWORD PixelFx::__GetPixelRGB32(int R, int G, int B)
{
	DWORD wPixel = 0x00000000;

	if (m_dwPixelFormat == RGB888)
	{
		wPixel = (R << 16) | (G << 8) | B;
		return wPixel;
	}
}

//// RGB 분해 함수 ( inline function )
void PixelFx::GetRGB565(DWORD RGB, DWORD* pR, DWORD* pG, DWORD* pB)
{
	*pR = (RGB & 0x0000F800) >> 11;
	*pG = (RGB & 0x000007E0) >> 5;
	*pB = (RGB & 0x0000001F);
}

void PixelFx::GetRGB888(DWORD RGB, DWORD* pR, DWORD* pG, DWORD* pB)
{
	*pR = (RGB & 0x00FF0000) >> 16;
	*pG = (RGB & 0x0000FF00) >> 8;
	*pB = (RGB & 0x000000FF);
}

/*-----------------------------------------------------------------------------------------------*/
void PixelFx::LUTInit()
{
	//// Fade Look Up Table 생성
	for (DWORD dwDepth = 0; dwDepth < 64; dwDepth++)
	{
		for (DWORD dwRate = 0; dwRate <= 100; dwRate++)
		{
			m_dwFADELUT16[dwDepth][dwRate] = dwDepth * dwRate / 100;
		}
	}

	for (DWORD dwDepth = 0; dwDepth < 256; dwDepth++)
	{
		for (DWORD dwRate = 0; dwRate <= 100; dwRate++)
		{
			m_dwFADELUT32[dwDepth][dwRate] = dwDepth * dwRate / 100;
		}
	}
}

/*페이드인*/
void PixelFx::__FxFadeIn(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRateStep)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy, dwRate;
	dwRate = 0;

	/*1.블리트*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	__Lock(lpDDSSrc); //표면 락
	while (dwRate < 99)
	{
		for (iy = 0; iy < VRES; iy++)
		{
			for (ix = 0; ix < HRES; ix++)
			{
				if (m_dwPixelFormat == RGB565)
				{
					wPixel = __GetPixel16(ix, iy);
					GetRGB565(wPixel, &R, &G, &B);

					R = R * dwRate / 100;
					G = G * dwRate / 100;
					B = B * dwRate / 100;
					RGB = __GetPixelRGB16(R, G, B);

					__PutPixel16(ix, iy, RGB);
				}
				else if (m_dwPixelFormat == RGB888)
				{
					//분해
					wPixel = __GetPixel32(ix, iy);
					GetRGB888(wPixel, &R, &G, &B);

					//조립
					R = R * dwRate / 100;
					G = G * dwRate / 100;
					B = B * dwRate / 100;
					RGB = __GetPixelRGB32(R, G, B);

					__PutPixel32(ix, iy, RGB);//재입력*/
				}
			}
		}

		dwRate += dwRateStep;
	}
	__Unlock(lpDDSSrc); //언락

	_BltSurface(lpDDSDst, lpDDSSrc);
}

/*페이드 아웃*/
void PixelFx::__FxFadeOut(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRateStep)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy, dwRate;
	dwRate = 100;

	/*1.블리트*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	//2. 분해 조립
	__Lock(lpDDSSrc); //표면 락
	while (dwRate > 1)
	{
		for (iy = 0; iy < VRES; iy++)
		{
			for (ix = 0; ix < HRES; ix++)
			{
				if (m_dwPixelFormat == RGB565)
				{
					wPixel = __GetPixel16(ix, iy);
					GetRGB565(wPixel, &R, &G, &B);

					R = R * dwRate / 100;
					G = G * dwRate / 100;
					B = B * dwRate / 100;
					RGB = __GetPixelRGB16(R, G, B);

					__PutPixel16(ix, iy, RGB);
				}
				else if (m_dwPixelFormat == RGB888)
				{
					//분해
					wPixel = __GetPixel32(ix, iy);
					GetRGB888(wPixel, &R, &G, &B);

					//조립
					R = R * dwRate / 100;
					G = G * dwRate / 100;
					B = B * dwRate / 100;
					RGB = __GetPixelRGB32(R, G, B);

					__PutPixel32(ix, iy, RGB);//재입력
				}
			}
		}

		dwRate -= dwRateStep;
	}
	__Unlock(lpDDSSrc); //언락

	_BltSurface(lpDDSDst, lpDDSSrc);
}

/*페이드 인아웃*/
void PixelFx::__FxFadeInOutEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRate)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy;

	/*1. 3차표면에 2차표면을 블리트*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	//2. 분해 조립
	__Lock(lpDDSSrc); //3차표면 락
	for (iy = 0; iy < VRES; iy++)
	{
		for (ix = 0; ix < HRES; ix++)
		{
			if (m_dwPixelFormat == RGB565)
			{
				wPixel = __GetPixel16(ix, iy);
				GetRGB565(wPixel, &R, &G, &B);

				//조립
				R = m_dwFADELUT16[R][dwRate];
				G = m_dwFADELUT16[G][dwRate];
				B = m_dwFADELUT16[B][dwRate];
				RGB = __GetPixelRGB16(R, G, B);

				__PutPixel16(ix, iy, RGB);
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//분해
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//조립
				R = m_dwFADELUT32[R][dwRate];
				G = m_dwFADELUT32[G][dwRate];
				B = m_dwFADELUT32[B][dwRate];
				RGB = __GetPixelRGB32(R, G, B);

				__PutPixel32(ix, iy, RGB);//재입력
			}
		}
	}
	__Unlock(lpDDSSrc); //언락

	//3. 3차 표면 그림을 2차표면(백버퍼)에 메모리 접근으로 그림
	__Lock(lpDDSDst);
	{
		__PutImageLock(lpDDSDst, lpDDSSrc, 0, 0);
	}
	__Unlock(lpDDSDst);
}

void PixelFx::__FxFadeTo(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD R2, DWORD G2, DWORD B2)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy;
	int i, nR, nG, nB;

	/*1. 3차표면에 2차표면을 블리트*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	//2. 분해 조립
	__Lock(lpDDSSrc); //3차표면 락
	for (iy = 0; iy < VRES; iy++)
	{
		for (ix = 0; ix < HRES; ix++)
		{
			if (m_dwPixelFormat == RGB565)
			{
				wPixel = __GetPixel16(ix, iy);
				GetRGB565(wPixel, &R, &G, &B);

				//증감
				while (1)
				{
					if (R > R2) R--;
					else if (R < R2) R++;
					if (G > G2) G--;
					else if (G < G2) G++;
					if (B > B2) B--;
					else if (B < B2) B++;

					if (R == R2 && G == G2 && B == B2) break;
				}

				RGB = __GetPixelRGB16(R, G, B);//조립

				__PutPixel16(ix, iy, RGB);
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//분해
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//증감
				while (1)
				{
					if (R > R2) R--;
					else if (R < R2) R++;
					if (G > G2) G--;
					else if (G < G2) G++;
					if (B > B2) B--;
					else if (B < B2) B++;

					if (R == R2 && G == G2 && B == B2) break;
				}

				RGB = __GetPixelRGB32(R, G, B);//조립

				__PutPixel32(ix, iy, RGB);//재입력
			}
		}
	}
	__Unlock(lpDDSSrc); //언락

	_BltSurface(lpDDSDst, lpDDSSrc);
}

/*그레이스케일 oooo*/
void PixelFx::__FxGrayScale(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc)
{
	DWORD R, G, B;
	DWORD RGB, Gray;
	DWORD wPixel;
	DWORD  ix, iy;

	/*1. 3차표면에 2차표면을 블리트*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	__Lock(lpDDSSrc); //3차표면 락
	/*2. 픽셀 값 얻어서 분해하고 조립한 후 재입력*/
	for (iy = 0; iy < VRES; iy++)
	{
		for (ix = 0; ix < HRES; ix++)
		{
			if (m_dwPixelFormat == RGB565)
			{
				wPixel = __GetPixel16(ix, iy);
				GetRGB565(wPixel, &R, &G, &B);

				Gray = (R + (G / 2) + B) / 3;
				RGB = __GetPixelRGB16(Gray, Gray * 2, Gray);

				__PutPixel16(ix, iy, RGB);//재입력
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//분해
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//조립
				Gray = (R + G + B) / 3;
				RGB = __GetPixelRGB32(Gray, Gray, Gray);

				__PutPixel32(ix, iy, RGB);//재입력
			}
		}
	}
	__Unlock(lpDDSSrc); //언락

	__Lock(lpDDSDst);
	{
		__PutImageLock(lpDDSDst,lpDDSSrc, 0, 0);
	}
	__Unlock(lpDDSDst);

}

void PixelFx::__FxColorize(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD R2, DWORD G2, DWORD B2)
{
	DWORD R, G, B;
	DWORD RGB;
	DWORD wPixel;
	DWORD  ix, iy;

	/*1. 3차표면에 2차표면을 블리트*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	__Lock(lpDDSSrc); //3차표면 락
	for (iy = 0; iy < VRES; iy++)
	{
		for (ix = 0; ix < HRES; ix++)
		{

			if (m_dwPixelFormat == RGB565)
			{
				wPixel = __GetPixel16(ix, iy);
				GetRGB565(wPixel, &R, &G, &B);

				R = R * R2 / 31;
				G = G * G2 / 63;
				B = B * B2 / 31;

				RGB = __GetPixelRGB16(R, G, B);

				__PutPixel16(ix, iy, RGB);//재입력*/
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//분해
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//조립
				R = R * R2 / 255;
				G = G * G2 / 255;
				B = B * B2 / 255;
				RGB = __GetPixelRGB32(R, G, B);

				__PutPixel32(ix, iy, RGB);//재입력*/
			}
		}
	}
	__Unlock(lpDDSSrc); //언락

	__Lock(lpDDSDst);
	{
		__PutImageLock(lpDDSDst, lpDDSSrc, 0, 0);
	}
	__Unlock(lpDDSDst);
}

/*RGB포맷 돌려줌*/
int PixelFx::getRGBFormat()
{
	return m_dwPixelFormat;
}
/*RGB 비트 카운트 리턴*/
int PixelFx::getBitCount()
{
	return m_dwRGBBitCount;
}

void PixelFx::SetClientSz(int hres, int vres)
{
	HRES = hres;
	VRES = vres;
}