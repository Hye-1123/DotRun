#ifndef H_DxD_Class
#define H_DxD_Class

#include "DxMng.h"

class DxDClass
{
public:
	DxDClass();
	~DxDClass();

	BOOL DD_Init(DWORD Wid, DWORD Hei, DWORD BPP, DWORD BufCount); //DD객체 생성 및 설정

	LPDIRECTDRAWSURFACE7 DxD_CreateSurface(DWORD dwWidth, DWORD dwHeight, BOOL bIsVram); //오프스크린 생성
	BOOL DxD_CSetClipper(LPDIRECTDRAWSURFACE7 lpDDS, RECT rc); //클리퍼 설정
	BOOL DxD_SetClipperList(LPDIRECTDRAWSURFACE7 lpDDS, LPRECT rcClipList, int nCount); //클립리스트 설정
	BOOL DxD_SetWindowClipper(LPDIRECTDRAWSURFACE7 lpDDs, HWND hwnd); //윈도우 클립 설정

	void DxD_BltFlip(); //백버퍼에 그린 화면 전환

	/*객체 얻기*/
	LPDIRECTDRAWSURFACE7 DxD_getPrimarySur();
	LPDIRECTDRAWSURFACE7 DxD_getBackSur();
	LPDIRECTDRAW7 DxD_getDDobj();

	void DxD_ObjRelease();//객체 해제

protected:
	BOOL DxD_InitFull(DWORD Hres, DWORD Vres, DWORD BPP, DWORD bBufCount); //풀스크린 모드로 DD객체 생성
	BOOL DxD_InitWin(DWORD Hres, DWORD Vres); //노말모드로 DD객체 생성

	BOOL DxD_DBLT(); //더블 버퍼링

private:
	/*DD객체*/
	LPDIRECTDRAW7 DD_Obj;
	LPDIRECTDRAWSURFACE7 Primary_Sur, Back_Sur;
	LPDIRECTDRAWSURFACE7 Off_Sur;
	DDSURFACEDESC2 ddsd;
	DDSCAPS2 ddcaps;

	/*클리핑*/
	LPDIRECTDRAWCLIPPER DD_Clipper;
	LPRGNDATA rgd;

	HRESULT hResult;

	BOOL b_full;
};




#endif // !H_DxD_Class
