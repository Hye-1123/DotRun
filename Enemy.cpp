#include <math.h>
#include "EnemyH.h"

#define HRES 640
#define VRES 480
#define ENEMYSIZE 256
#define DEG2RAD  0.017453293f // PI/180, ���� ���� ������ ��ȯ

EnemyClass::EnemyClass()
{
	
}

EnemyClass::~EnemyClass()
{
}

void EnemyClass::Init()
{
	size = 10; //��������-default
	count = 0;
}

////*�� �����*/
void EnemyClass::Generation(int num)
{
	int x, y;
	int R, G, B;
	EnemyInfo temp;

	x = rand() % (HRES - size);
	y = rand() % (VRES - size);

	//��ä�� �Ȱɸ��� 
	while (1)
	{
		R = rand() % 256;
		G = rand() % 256;
		B = rand() % 256;

		if (R != G && R != B && G != B)
		{
			break;
		}
	}

	temp.dwColor = RGB(B, G, R); //Colorref�� ������ BGR������ ���� ����� �������� �̷��� �ؾ���.
	___Trace("%d %d %d \n", R, G, B);
	SetRect(&temp.rPos, x, y, x + size, y + size);
	temp.nSpeed = rand() % 2 + 2; //2~5���� �ӵ�
	temp.MoveType = rand() % 5;

	enemyInfo.push_back(temp);

	count++;
}

void EnemyClass::Clear()
{
	if (!enemyInfo.empty()) //����� �ִ��� Ȯ���ϰ�
	{
		enemyInfo.clear(); //��Ҹ� ��� ����
		count = 0; //ī��Ʈ�� 0���� ����
	}
	
}

//�� �׸���, ���η����� ���� �������� �������� �и���
void EnemyClass::Draw(int num, LPDIRECTDRAWSURFACE7 lpDDSDest)
{
	DDBLTFX ddbltfx;

	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = enemyInfo[num].dwColor;

	HRESULT ddrval = lpDDSDest->Blt(&enemyInfo[num].rPos, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

//�� ������ �ø��� �ڵ�
void EnemyClass::SzUp()
{
	size += 10;
}

//�� �����̴� �ڵ�
void EnemyClass::Move(int num, POINT HeroPos)
{
	POINT EnePos = { enemyInfo[num].rPos.left, enemyInfo[num].rPos.top };
	static int n = 0;

	n++;
	if (n > count * 5)
	{
		enemyInfo[num].MoveType = rand() % 5;
		n = 0;
	}

	switch (enemyInfo[num].MoveType)
	{
	case 0:
		break;
	case 1:
		enemyInfo[num].rPos.left += enemyInfo[num].nSpeed;
		break;
	case 2:
		enemyInfo[num].rPos.left -= enemyInfo[num].nSpeed;
		break;
	case 3:
		enemyInfo[num].rPos.top += enemyInfo[num].nSpeed;
		break;
	case 4:
		enemyInfo[num].rPos.top -= enemyInfo[num].nSpeed;
		break;;
	}

	__Chase(HeroPos.x, HeroPos.y, enemyInfo[num].nSpeed, &enemyInfo[num].rPos);

	/*�����̴� ȭ��ۿ� ������ �ʰ� �ϴ� �ڵ�*/
	if (enemyInfo[num].rPos.left < 0)
	{
		enemyInfo[num].rPos.left = 0;
		enemyInfo[num].MoveType = rand() % 5;
	}
	else if (enemyInfo[num].rPos.left + size > HRES)
	{
		enemyInfo[num].rPos.left = HRES - size;
		enemyInfo[num].MoveType = rand() % 5;
	}

	if (enemyInfo[num].rPos.top < 0)
	{
		enemyInfo[num].rPos.top = 0;
		enemyInfo[num].MoveType = rand() % 5;
	}
	else if (enemyInfo[num].rPos.top + size > VRES)
	{
		enemyInfo[num].rPos.top = VRES - size;
		enemyInfo[num].MoveType = rand() % 5;
	}

	enemyInfo[num].rPos.right = enemyInfo[num].rPos.left + size;
	enemyInfo[num].rPos.bottom = enemyInfo[num].rPos.top + size;

}

//���� �ڵ�, �ֽ� �ڵ忡�� ���ſ� �°� ��ħ
void EnemyClass::__Chase(int x1, int y1, int speed, RECT* Rect) //���� ��� ��ǥ, �Ѵ� ��� ��ǥ, �ӵ�;
{
	//// ������ �Ÿ��� ���Ѵ�
	float distX = (float)(x1 - Rect->left); //�÷��̾� x - �� x��ǥ
	float distY = (float)(y1 - Rect->top); //�÷��̾� y - �� y��ǥ
	float fDist = (float)sqrt(distX*distX + distY * distY); //������ ���ϴ� �Լ�

	//// �̵� ������ ���Ѵ�
	float fStepX = distX / fDist;
	float fStepY = distY / fDist;

	//// ���Ŀ� ����
	Rect->left += (int)(speed * fStepX);
	Rect->top += (int)(speed * fStepY);

}

int EnemyClass::GetCount()
{
	return count;
}

RECT EnemyClass::GetPos(int num)
{
	return enemyInfo[num].rPos;
}