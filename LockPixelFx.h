#ifndef LockPFx_H
#define LockPFx_H

#include "DxMng.h"

class PixelFx
{
public:
	PixelFx();
	~PixelFx();

	//RGB포맷 확인
	void __CheckRGBBit(LPDIRECTDRAWSURFACE7 lpdds);

	//락언락
	BOOL __Lock(LPDIRECTDRAWSURFACE7 pDDSDest);
	BOOL __Unlock(LPDIRECTDRAWSURFACE7 pDDSDest);

	/*blt*/
	void _BltSurface(LPDIRECTDRAWSURFACE7 lpDestSur, LPDIRECTDRAWSURFACE7 lpSrcSur);

	void __PutImage(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutSprite(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutImageEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutSpriteEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);

	/*Lock*/
	void __PutImageLock(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutSpriteLock16(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutSpriteLock32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutImageLockEx32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);
	void __PutSpriteLockEx32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y);

	/*넣고 얻고*/
	inline void __PutPixel32(int x, int y, DWORD dwColor);
	inline void __PutPixel16(int x, int y, WORD wColor);

	inline WORD __GetPixel16(int x, int y);
	inline DWORD __GetPixel32(int x, int y); //32비트 픽셀 받아오기 추가

	//RGB조합
	inline DWORD __GetPixelRGB16(int R, int G, int B);
	inline DWORD __GetPixelRGB32(int R, int G, int B);

	//RGB분해
	inline void GetRGB565(DWORD RGB, DWORD* pR, DWORD* pG, DWORD* pB);
	inline void GetRGB888(DWORD RGB, DWORD* pR, DWORD* pG, DWORD* pB);

	/*화면 효과*/
	void LUTInit(void); //lockuptable
	void __FxFadeIn(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRate);
	void __FxFadeOut(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRate);
	void __FxFadeInOutEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRate);
	void __FxFadeTo(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD R2, DWORD G2, DWORD B2);
	void __FxGrayScale(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc);
	void __FxColorize(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD R2, DWORD G2, DWORD B2);

	/*기타 멤버 변수 얻기설정*/
	int getRGBFormat();
	int getBitCount();
	
	void SetClientSz(int hres, int vres);

private:
	DWORD m_dwFADELUT16[64][101]; //lookuptable용
	DWORD m_dwFADELUT32[256][101];

	DDSURFACEDESC2 m_ddsd;
	DWORD m_dwPixelFormat;
	DWORD m_dwRGBBitCount;

	int VRES, HRES;
};



#endif // !LockPFx_H

