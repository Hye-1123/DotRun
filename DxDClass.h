#ifndef H_DxD_Class
#define H_DxD_Class

#include "DxMng.h"

class DxDClass
{
public:
	DxDClass();
	~DxDClass();

	BOOL DD_Init(DWORD Wid, DWORD Hei, DWORD BPP, DWORD BufCount); //DD��ü ���� �� ����

	LPDIRECTDRAWSURFACE7 DxD_CreateSurface(DWORD dwWidth, DWORD dwHeight, BOOL bIsVram); //������ũ�� ����
	BOOL DxD_CSetClipper(LPDIRECTDRAWSURFACE7 lpDDS, RECT rc); //Ŭ���� ����
	BOOL DxD_SetClipperList(LPDIRECTDRAWSURFACE7 lpDDS, LPRECT rcClipList, int nCount); //Ŭ������Ʈ ����
	BOOL DxD_SetWindowClipper(LPDIRECTDRAWSURFACE7 lpDDs, HWND hwnd); //������ Ŭ�� ����

	void DxD_BltFlip(); //����ۿ� �׸� ȭ�� ��ȯ

	/*��ü ���*/
	LPDIRECTDRAWSURFACE7 DxD_getPrimarySur();
	LPDIRECTDRAWSURFACE7 DxD_getBackSur();
	LPDIRECTDRAW7 DxD_getDDobj();

	void DxD_ObjRelease();//��ü ����

protected:
	BOOL DxD_InitFull(DWORD Hres, DWORD Vres, DWORD BPP, DWORD bBufCount); //Ǯ��ũ�� ���� DD��ü ����
	BOOL DxD_InitWin(DWORD Hres, DWORD Vres); //�븻���� DD��ü ����

	BOOL DxD_DBLT(); //���� ���۸�

private:
	/*DD��ü*/
	LPDIRECTDRAW7 DD_Obj;
	LPDIRECTDRAWSURFACE7 Primary_Sur, Back_Sur;
	LPDIRECTDRAWSURFACE7 Off_Sur;
	DDSURFACEDESC2 ddsd;
	DDSCAPS2 ddcaps;

	/*Ŭ����*/
	LPDIRECTDRAWCLIPPER DD_Clipper;
	LPRGNDATA rgd;

	HRESULT hResult;

	BOOL b_full;
};




#endif // !H_DxD_Class
