#include "LockPixelFx.h"

#define RGB565 565
#define RGB888 888

PixelFx::PixelFx()
{
	
}

PixelFx::~PixelFx()
{
}

/*RGB �����ϰ� ���Ʈ���� �˾Ƴ�*/
void PixelFx::__CheckRGBBit(LPDIRECTDRAWSURFACE7 lpDDS)
{
	DDSURFACEDESC2 ddsd;
	
	::ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_PIXELFORMAT;
	HRESULT hRet = lpDDS->GetSurfaceDesc(&ddsd);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	m_dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	switch (ddsd.ddpfPixelFormat.dwRBitMask) //R����ũ�� ������ �˾Ƴ�
	{
	case 0x0000F800: m_dwPixelFormat = RGB565; break;    // 11111000 00000000
	case 0x00FF0000: m_dwPixelFormat = RGB888; break;    // 11111111 00000000 00000000
	}

}

/*----------------------------------------------------------------------------*/
/*��ó��*/
inline BOOL PixelFx::__Lock(LPDIRECTDRAWSURFACE7 pDDSDest)
{
	::ZeroMemory(&m_ddsd, sizeof(DDSURFACEDESC2));
	m_ddsd.dwSize = sizeof(DDSURFACEDESC2);
	HRESULT hRet = pDDSDest->Lock(NULL, &m_ddsd, DDLOCK_WAIT, NULL);
	if (hRet != DD_OK) { return DDERRCHK(hRet); }

	return TRUE;
}

/*���*/
inline BOOL PixelFx::__Unlock(LPDIRECTDRAWSURFACE7 pDDSDest)
{
	HRESULT hRet = pDDSDest->Unlock(NULL);
	if (hRet != DD_OK) { return DDERRCHK(hRet); }

	return TRUE;
}

//// Blt() �̹���
/*����� ��ü�� ���� ǥ���� ��ü�� ��Ʈ��*/
void PixelFx::_BltSurface(LPDIRECTDRAWSURFACE7 lpDestSur, LPDIRECTDRAWSURFACE7 lpSrcSur) 
{
	DDBLTFX ddbltfx;

	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(DDBLTFX);

	HRESULT ddrval = lpDestSur->Blt(NULL, lpSrcSur, NULL, DDBLT_WAIT, &ddbltfx);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*ǥ��ä�� ����ۿ� ��Ʈ��, Ŭ����X, �÷�ŰX*/
void PixelFx::__PutImage(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	//// ǥ���� ũ�� ������ ��´� ( �ӽ��ڵ� )
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	/////
	RECT rcDest = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	HRESULT ddrval = lpDDSDst->Blt(&rcDest, lpDDSSrc, NULL, DDBLT_WAIT, NULL);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*�����ϰ� ǥ��ä�� ��Ʈ��, Ŭ����x, �÷�Ű O*/
void PixelFx::__PutSprite(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	// ǥ���� ũ�� ������ ��´�
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	RECT rcDest = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	HRESULT ddrval = lpDDSDst->Blt(&rcDest, lpDDSSrc, NULL, DDBLT_KEYSRC | DDBLT_WAIT, NULL); //�÷�Ű ó��
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*�̹����� ��Ʈ��, Ŭ���� O, �÷�Ű X*/
void PixelFx::__PutImageEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	//// ǥ���� ũ�� ������ ��´� ( �ӽ��ڵ� )
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	//// ��ũ����ǥ�� RECT�� ���Ѵ�
	RECT rcDst = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	RECT rcSrc = { 0, 0, ddsd.dwWidth, ddsd.dwHeight };

	//// ��ũ�� ������ ������ ������� �ʴ´�
	if (x >= HRES || y >= VRES) return;
	if (rcDst.right <= 0 || rcDst.bottom <= 0) return;

	//// ��������Ʈ�� ��ũ�� ������ ���� ��ŭ ������� �ʴ´�
	//// ���� ��ũ���� �簢������ ���� ���� Ŭ������ �����Ѵ�	

	//// ���� ����
	if (x < 0)
	{
		// �º��� ���δ�. rcSrc.left += �Ѿ ũ��, abs(x)
		rcSrc.left -= x;
		rcDst.left = x = 0;
	}
	else if (rcDst.right >= HRES)
	{
		// �캯�� ���δ�. rcSrc.right -= �Ѿ ũ��
		int nOverSize = (rcDst.right - HRES);
		rcSrc.right -= nOverSize;
		rcDst.right -= nOverSize;
	}
	//// ���� ����
	if (y < 0)
	{
		// ���� ���δ�. rcSrc.top += �Ѿ ũ��, abs(y)
		rcSrc.top -= y;
		rcDst.top = y = 0;
	}
	else if (rcDst.bottom >= VRES)
	{
		// �Ϻ��� ���δ�. rcSrc.bottom -= �Ѿ ũ�� 
		int nOverSize = (rcDst.bottom - VRES);
		rcSrc.bottom -= nOverSize;
		rcDst.bottom -= nOverSize;
	}

	//�װ���� 2��ǥ�鿡 ��Ʈ
	HRESULT ddrval = lpDDSDst->Blt(&rcDst, lpDDSSrc, &rcSrc, DDBLT_WAIT, NULL);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

/*�̹����� ��Ʈ��, Ŭ���� O, �÷�Ű O*/
void PixelFx::__PutSpriteEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	//// ǥ���� ũ�� ������ ��´� ( �ӽ��ڵ� )
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	lpDDSSrc->GetSurfaceDesc(&ddsd);

	//// ��ũ����ǥ�� RECT�� ���Ѵ�
	RECT rcDst = { x, y, x + ddsd.dwWidth, y + ddsd.dwHeight };
	RECT rcSrc = { 0, 0, ddsd.dwWidth, ddsd.dwHeight };

	//// ��ũ�� ������ ������ ������� �ʴ´�
	if (x >= HRES || y >= VRES) return;
	if (rcDst.right <= 0 || rcDst.bottom <= 0) return;

	//// ��������Ʈ�� ��ũ�� ������ ���� ��ŭ ������� �ʴ´�
	//// ���� ��ũ���� �簢������ ���� ���� Ŭ������ �����Ѵ�	

	//// ���� ����
	if (x < 0)
	{
		// �º��� ���δ�. rcSrc.left += �Ѿ ũ��, abs(x)
		rcSrc.left -= x;
		rcDst.left = x = 0;
	}
	else if (rcDst.right >= HRES)
	{
		// �캯�� ���δ�. rcSrc.right -= �Ѿ ũ��
		int nOverSize = (rcDst.right - HRES);
		rcSrc.right -= nOverSize;
		rcDst.right -= nOverSize;
	}
	//// ���� ����
	if (y < 0)
	{
		// ���� ���δ�. rcSrc.top += �Ѿ ũ��, abs(y)
		rcSrc.top -= y;
		rcDst.top = y = 0;
	}
	else if (rcDst.bottom >= VRES)
	{
		// �Ϻ��� ���δ�. rcSrc.bottom -= �Ѿ ũ�� 
		int nOverSize = (rcDst.bottom - VRES);
		rcSrc.bottom -= nOverSize;
		rcDst.bottom -= nOverSize;
	}

	//��Ʈ
	HRESULT ddrval = lpDDSDst->Blt(&rcDst, lpDDSSrc, &rcSrc, DDBLT_KEYSRC | DDBLT_WAIT, NULL); //��ó���� �Ȱ��� ��Ʈ �ɼǸ� �ٲ�
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

//// Lock() �̹���
//// �� �� : ����ǥ��(2��)�� Lock�� �� ���¿��� �Լ� ����
//// �� �� : 16, 32��Ʈ ��� �̹��� ���
//// lpDDS : �ҽ� ǥ��
//// x, y  : ��� ��ǥ 
/*�̹���(������ũ��)�� 2��ǥ�鿡 ������, Ŭ���� X, �÷�Ű X*/
void PixelFx::__PutImageLock(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;

	memset(&ddsdSrc, 0, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);

	/*src ��*/
	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //������ũ���� ����
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	///// BPP�� ���� ���� ���� ���Ѵ�
	DWORD dwShift;
	switch (m_dwRGBBitCount)
	{
	case 16: dwShift = 1; break;
	case 32: dwShift = 2; break;
	}

	//// �̹��� ǥ�鿡 ���� �޸� �ּ�
	LPBYTE lpSrc = (LPBYTE)ddsdSrc.lpSurface;

	//// 2�� ǥ�鿡 ���� �޸� �ּ� (����)		
	LPBYTE lpDst = (LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << dwShift); //����ۿ� ���� �ּ� ���̸� ����۴� ���� ����

	DWORD dwWidth = ddsdSrc.dwWidth << dwShift; //�ȼ��� ����Ʈ ����� ������ ��(�� �ȼ��� 2����Ʈ�� ���� ���� ���̴� width*2�� ����
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// memcpy�� ���� �޸� ����
	for (DWORD iy = 0; iy < dwHeight; iy++)
	{
		memcpy(lpDst, lpSrc, dwWidth);
		lpSrc += ddsdSrc.lPitch;
		lpDst += m_ddsd.lPitch;
	}

	hRet = lpDDSSrc->Unlock(NULL); //���
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

//// Lock() ��������Ʈ
//// �� �� : ����ǥ��(2��)�� Lock�� �� ���¿��� �Լ� ����
//// �� �� : 16��Ʈ ��� ��������Ʈ ���
//// lpDDS : �ҽ� ǥ��
//// x, y  : ��� ��ǥ 

/*��������Ʈ �̹����� 2��ǥ�鿡 ���� 16bit, Ŭ���� X, �÷�Ű O*/
void PixelFx::__PutSpriteLock16(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;

	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);

	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// ǥ�鿡 ���� �޸� �ּ�
	WORD* lpSrc = (WORD*)ddsdSrc.lpSurface;

	//// 2�� ǥ�鿡 ���� �޸� �ּ� (����)
	WORD* lpDst = (WORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 1)); //����� ��ġ�� �̵�

	//srcũ��
	DWORD dwWidth = ddsdSrc.dwWidth;
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// �÷�Ű
	DWORD dwSrcColorKey = ddsdSrc.ddckCKSrcBlt.dwColorSpaceLowValue; //�÷�Ű�� �޾ƿ�
	DWORD dwSrc = 0;

	//// WORD(2����Ʈ) ������ �ּҰ� ���� �ǹǷ� RShift ( /2)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 1; //lpSrc�� Word������ Src + lPitch�ع����� ��ǻ� src + (lPitch*2)�� ���Ƽ� ����
	DWORD dwDstPitch = m_ddsd.lPitch >> 1;

	//// ������������ �ִ��� ������ �ּ�ȭ�Ѵ�	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			dwSrc = *(lpSrc + ix);
			if (dwSrc != dwSrcColorKey) *(lpDst + ix) = (WORD)dwSrc; //�÷�Ű�� �ƴ� ���� ���ȼ��� �ֽ��ϴ�.
		}
		lpSrc += dwSrcPitch; //���� �̵�
		lpDst += dwDstPitch; //���� �̵�
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

//// Lock() ��������Ʈ
//// �� �� : ����ǥ��(2��)�� Lock�� �� ���¿��� �Լ� ����
//// �� �� : 32��Ʈ ��� ��������Ʈ ���
//// lpDDS : �ҽ� ǥ��
//// x, y  : ��� ��ǥ 

/*��������Ʈ �̹����� 2��ǥ�鿡 ���� 32bit, Ŭ���� X, �÷�Ű O*/
void PixelFx::__PutSpriteLock32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;

	/*������ũ��(��������Ʈ�̹���)�� ���� ��� */
	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);
	ddsdSrc.dwFlags = DDSD_CKSRCBLT;

	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //������ũ���� ����
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// ǥ�鿡 ���� �޸� �ּ�
	DWORD* lpSrc = (DWORD*)ddsdSrc.lpSurface; //������ũ���� ���� �ּ�

	//// 2�� ǥ�鿡 ���� �޸� �ּ� ( ���� ) , ( *4 )
	DWORD* lpDst = (DWORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 2)); //����� ��ġ�� �̵�

	DWORD dwWidth = ddsdSrc.dwWidth; //������ũ�� ���μ���
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// �÷�Ű
	DWORD dwSrcColorKey = ddsdSrc.ddckCKSrcBlt.dwColorSpaceLowValue; //�÷�Ű �޾ƿ�
	DWORD dwSrc = 0;

	//// DWORD ������ �ּҰ� ���� �ǹǷ� RShift ( /4)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 2; //lpSrc�� DWord������ Src + lPitch�ع����� ��ǻ� src + (lPitch*4)�� ���Ƽ� ����
	DWORD dwDstPitch = m_ddsd.lPitch >> 2;

	//// ������������ �ִ��� ������ �ּ�ȭ�Ѵ�	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			//// �ֻ��� 8��Ʈ�� �����Ѵ�(XRGB, ARGB)
			dwSrc = *(lpSrc + ix) & 0x00FFFFFF;
			if (dwSrc != dwSrcColorKey) *(lpDst + ix) = dwSrc; //�ȼ� �ϳ��� ����
		}
		lpSrc += dwSrcPitch; //�����ٷ�(������ũ��)
		lpDst += dwDstPitch; // ���� ��������(2�� ǥ��)
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

//// Lock() �̹��� & ��������Ʈ ��� �Լ��� Ŭ���� ó�� ( ����Ʈ���� ���� )
//// �� �� : 16, 32��Ʈ ��� ��������Ʈ ���
//// lpDDS : �ҽ� ǥ��
//// x, y  : ��� ��ǥ

/*���ϰ� �޸��� �������� �̹��� �ְ� Ŭ���� ó��*/
void PixelFx::__PutImageLockEx32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;
	DWORD dwSrc = 0;

	/*������ũ��(��������Ʈ�̹���)�� ���� ��� */
	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);
	ddsdSrc.dwFlags = DDSD_CKSRCBLT;
	//������ũ���� ����
	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //������ũ���� �б⸸��
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// ǥ�鿡 ���� �޸� �ּ�
	DWORD* lpSrc = (DWORD*)ddsdSrc.lpSurface; //������ũ���� ���� �ּ�

	//// 2�� ǥ�鿡 ���� �޸� �ּ� ( ���� ) , ( *4 )
	DWORD* lpDst = (DWORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 2)); //����� ��ġ�� �̵�

	DWORD dwWidth = ddsdSrc.dwWidth; //������ũ�� ���μ���
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// DWORD ������ �ּҰ� ���� �ǹǷ� RShift ( /4)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 2; //lpSrc�� DWord������ Src + lPitch�ع����� ��ǻ� src + (lPitch*4)�� ���Ƽ� ����
	DWORD dwDstPitch = m_ddsd.lPitch >> 2;

	//// ��ũ�� ������ ������ ������� �ʴ´�
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

	//// ������������ �ִ��� ������ �ּ�ȭ�Ѵ�	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			//Ŭ����
			if (y + iy >= 0 && y + iy < VRES) //�ȼ��� ���� ��ġ�� �ľ��ؼ� Ŭ���� �� �κи� �ִ´�.
			{
				if (x + ix >= 0 && x + ix < HRES)
				{
					//// �ֻ��� 8��Ʈ�� �����Ѵ�(XRGB, ARGB)
					dwSrc = *(lpSrc + ix) & 0x00FFFFFF;
					*(lpDst + ix) = dwSrc; //�ȼ� �ϳ��ϳ� ����
				}
			}
		}

		lpSrc += dwSrcPitch; //�����ٷ�(������ũ��)
		lpDst += dwDstPitch; // ���� ��������(2�� ǥ��)
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

/*���ϰ� �޸��� �������� �̹��� �ְ� �÷�Ű �ְ� Ŭ���� ó���ϰ�..*/
void PixelFx::__PutSpriteLockEx32(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, int x, int y)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsdSrc;
	DWORD ix, iy;

	/*������ũ��(��������Ʈ�̹���)�� ���� ��� */
	ZeroMemory(&ddsdSrc, sizeof(ddsdSrc));
	ddsdSrc.dwSize = sizeof(ddsdSrc);
	ddsdSrc.dwFlags = DDSD_CKSRCBLT;
	//������ũ���� ����
	hRet = lpDDSSrc->Lock(NULL, &ddsdSrc, DDLOCK_READONLY | DDLOCK_WAIT, NULL); //������ũ���� �б⸸��
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }

	//// ǥ�鿡 ���� �޸� �ּ�
	DWORD* lpSrc = (DWORD*)ddsdSrc.lpSurface; //������ũ���� ���� �ּ�

	//// 2�� ǥ�鿡 ���� �޸� �ּ� ( ���� ) , ( *4 )
	DWORD* lpDst = (DWORD*)((LPBYTE)m_ddsd.lpSurface + y * m_ddsd.lPitch + (x << 2)); //����� ��ġ�� �̵�

	DWORD dwWidth = ddsdSrc.dwWidth; //������ũ�� ���μ���
	DWORD dwHeight = ddsdSrc.dwHeight;

	//// DWORD ������ �ּҰ� ���� �ǹǷ� RShift ( /4)
	DWORD dwSrcPitch = ddsdSrc.lPitch >> 2; //lpSrc�� DWord������ Src + lPitch�ع����� ��ǻ� src + (lPitch*4)�� ���Ƽ� ����
	DWORD dwDstPitch = m_ddsd.lPitch >> 2;

	DWORD dwSrcColorKey = ddsdSrc.ddckCKSrcBlt.dwColorSpaceLowValue; //�÷�Ű �޾ƿ�
	DWORD dwSrc = 0;

	//// ��ũ�� ������ ������ ������� �ʴ´�
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

	//// ������������ �ִ��� ������ �ּ�ȭ�Ѵ�	
	for (iy = 0; iy < dwHeight; iy++)
	{
		for (ix = 0; ix < dwWidth; ix++)
		{
			//Ŭ����
			if (y + iy >= 0 && y + iy < VRES) //�ȼ��� ���� ��ġ�� �ľ��ؼ� �ִ´�.
			{
				if (x + ix >= 0 && x + ix < HRES)
				{
					//// �ֻ��� 8��Ʈ�� �����Ѵ�(XRGB, ARGB)
					dwSrc = *(lpSrc + ix) & 0x00FFFFFF;
					if (dwSrc != dwSrcColorKey)
						*(lpDst + ix) = dwSrc; //�ȼ� �ϳ��ϳ� ����
				}
			}
		}

		lpSrc += dwSrcPitch; //�����ٷ�(������ũ��)
		lpDst += dwDstPitch; // ���� ��������(2�� ǥ��)
	}

	hRet = lpDDSSrc->Unlock(NULL);
	if (hRet != DD_OK) { DDERRCHK(hRet); return; }
}

/*------------------------------------------------------------------------*/
// ��ǥ ���� : y * ǥ���� ������ + x
// y * ǥ���� ������ + (x * �ȼ�ũ��)
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

/*32��Ʈ �ȼ��� �޾ƿ��� �ڵ� �߰�*/
DWORD PixelFx::__GetPixel32(int x, int y)
{
	LPBYTE lpDDS = (LPBYTE)m_ddsd.lpSurface;
	DWORD* pPixel = (DWORD*)(lpDDS + y * m_ddsd.lPitch + (x * 4));

	return *pPixel;
}

//�����Լ�
DWORD PixelFx::__GetPixelRGB16(int R, int G, int B)
{
	DWORD wPixel = 0;

	if (m_dwPixelFormat == RGB565)
	{
		wPixel = (R << 11) | (G << 5) | B;   // 0000 0000 0001 1111		                                 
		return wPixel;					 // 0000 0111 1110 0000   5
	}                                    // 1111 1000 0000 0000  11
}

//�����Լ�
DWORD PixelFx::__GetPixelRGB32(int R, int G, int B)
{
	DWORD wPixel = 0x00000000;

	if (m_dwPixelFormat == RGB888)
	{
		wPixel = (R << 16) | (G << 8) | B;
		return wPixel;
	}
}

//// RGB ���� �Լ� ( inline function )
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
	//// Fade Look Up Table ����
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

/*���̵���*/
void PixelFx::__FxFadeIn(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRateStep)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy, dwRate;
	dwRate = 0;

	/*1.��Ʈ*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	__Lock(lpDDSSrc); //ǥ�� ��
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
					//����
					wPixel = __GetPixel32(ix, iy);
					GetRGB888(wPixel, &R, &G, &B);

					//����
					R = R * dwRate / 100;
					G = G * dwRate / 100;
					B = B * dwRate / 100;
					RGB = __GetPixelRGB32(R, G, B);

					__PutPixel32(ix, iy, RGB);//���Է�*/
				}
			}
		}

		dwRate += dwRateStep;
	}
	__Unlock(lpDDSSrc); //���

	_BltSurface(lpDDSDst, lpDDSSrc);
}

/*���̵� �ƿ�*/
void PixelFx::__FxFadeOut(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRateStep)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy, dwRate;
	dwRate = 100;

	/*1.��Ʈ*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	//2. ���� ����
	__Lock(lpDDSSrc); //ǥ�� ��
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
					//����
					wPixel = __GetPixel32(ix, iy);
					GetRGB888(wPixel, &R, &G, &B);

					//����
					R = R * dwRate / 100;
					G = G * dwRate / 100;
					B = B * dwRate / 100;
					RGB = __GetPixelRGB32(R, G, B);

					__PutPixel32(ix, iy, RGB);//���Է�
				}
			}
		}

		dwRate -= dwRateStep;
	}
	__Unlock(lpDDSSrc); //���

	_BltSurface(lpDDSDst, lpDDSSrc);
}

/*���̵� �ξƿ�*/
void PixelFx::__FxFadeInOutEx(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc, DWORD dwRate)
{
	DWORD RGB;
	DWORD wPixel;
	DWORD R, G, B;
	DWORD  ix, iy;

	/*1. 3��ǥ�鿡 2��ǥ���� ��Ʈ*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	//2. ���� ����
	__Lock(lpDDSSrc); //3��ǥ�� ��
	for (iy = 0; iy < VRES; iy++)
	{
		for (ix = 0; ix < HRES; ix++)
		{
			if (m_dwPixelFormat == RGB565)
			{
				wPixel = __GetPixel16(ix, iy);
				GetRGB565(wPixel, &R, &G, &B);

				//����
				R = m_dwFADELUT16[R][dwRate];
				G = m_dwFADELUT16[G][dwRate];
				B = m_dwFADELUT16[B][dwRate];
				RGB = __GetPixelRGB16(R, G, B);

				__PutPixel16(ix, iy, RGB);
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//����
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//����
				R = m_dwFADELUT32[R][dwRate];
				G = m_dwFADELUT32[G][dwRate];
				B = m_dwFADELUT32[B][dwRate];
				RGB = __GetPixelRGB32(R, G, B);

				__PutPixel32(ix, iy, RGB);//���Է�
			}
		}
	}
	__Unlock(lpDDSSrc); //���

	//3. 3�� ǥ�� �׸��� 2��ǥ��(�����)�� �޸� �������� �׸�
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

	/*1. 3��ǥ�鿡 2��ǥ���� ��Ʈ*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	//2. ���� ����
	__Lock(lpDDSSrc); //3��ǥ�� ��
	for (iy = 0; iy < VRES; iy++)
	{
		for (ix = 0; ix < HRES; ix++)
		{
			if (m_dwPixelFormat == RGB565)
			{
				wPixel = __GetPixel16(ix, iy);
				GetRGB565(wPixel, &R, &G, &B);

				//����
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

				RGB = __GetPixelRGB16(R, G, B);//����

				__PutPixel16(ix, iy, RGB);
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//����
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//����
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

				RGB = __GetPixelRGB32(R, G, B);//����

				__PutPixel32(ix, iy, RGB);//���Է�
			}
		}
	}
	__Unlock(lpDDSSrc); //���

	_BltSurface(lpDDSDst, lpDDSSrc);
}

/*�׷��̽����� oooo*/
void PixelFx::__FxGrayScale(LPDIRECTDRAWSURFACE7 lpDDSDst, LPDIRECTDRAWSURFACE7 lpDDSSrc)
{
	DWORD R, G, B;
	DWORD RGB, Gray;
	DWORD wPixel;
	DWORD  ix, iy;

	/*1. 3��ǥ�鿡 2��ǥ���� ��Ʈ*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	__Lock(lpDDSSrc); //3��ǥ�� ��
	/*2. �ȼ� �� �� �����ϰ� ������ �� ���Է�*/
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

				__PutPixel16(ix, iy, RGB);//���Է�
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//����
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//����
				Gray = (R + G + B) / 3;
				RGB = __GetPixelRGB32(Gray, Gray, Gray);

				__PutPixel32(ix, iy, RGB);//���Է�
			}
		}
	}
	__Unlock(lpDDSSrc); //���

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

	/*1. 3��ǥ�鿡 2��ǥ���� ��Ʈ*/
	_BltSurface(lpDDSSrc, lpDDSDst);

	__Lock(lpDDSSrc); //3��ǥ�� ��
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

				__PutPixel16(ix, iy, RGB);//���Է�*/
			}
			else if (m_dwPixelFormat == RGB888)
			{
				//����
				wPixel = __GetPixel32(ix, iy);
				GetRGB888(wPixel, &R, &G, &B);

				//����
				R = R * R2 / 255;
				G = G * G2 / 255;
				B = B * B2 / 255;
				RGB = __GetPixelRGB32(R, G, B);

				__PutPixel32(ix, iy, RGB);//���Է�*/
			}
		}
	}
	__Unlock(lpDDSSrc); //���

	__Lock(lpDDSDst);
	{
		__PutImageLock(lpDDSDst, lpDDSSrc, 0, 0);
	}
	__Unlock(lpDDSDst);
}

/*RGB���� ������*/
int PixelFx::getRGBFormat()
{
	return m_dwPixelFormat;
}
/*RGB ��Ʈ ī��Ʈ ����*/
int PixelFx::getBitCount()
{
	return m_dwRGBBitCount;
}

void PixelFx::SetClientSz(int hres, int vres)
{
	HRES = hres;
	VRES = vres;
}