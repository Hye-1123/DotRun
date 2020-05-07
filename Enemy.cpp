#include <math.h>
#include "EnemyH.h"

#define HRES 640
#define VRES 480
#define ENEMYSIZE 256
#define DEG2RAD  0.017453293f // PI/180, 도를 라디안 단위로 변환

EnemyClass::EnemyClass()
{
	
}

EnemyClass::~EnemyClass()
{
}

void EnemyClass::Init()
{
	size = 10; //적사이즈-default
	count = 0;
}

////*적 만들기*/
void EnemyClass::Generation(int num)
{
	int x, y;
	int R, G, B;
	EnemyInfo temp;

	x = rand() % (HRES - size);
	y = rand() % (VRES - size);

	//무채색 안걸리게 
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

	temp.dwColor = RGB(B, G, R); //Colorref이 변수는 BGR순으로 들어가서 제대로 뽑으려면 이렇게 해야함.
	___Trace("%d %d %d \n", R, G, B);
	SetRect(&temp.rPos, x, y, x + size, y + size);
	temp.nSpeed = rand() % 2 + 2; //2~5까지 속도
	temp.MoveType = rand() % 5;

	enemyInfo.push_back(temp);

	count++;
}

void EnemyClass::Clear()
{
	if (!enemyInfo.empty()) //비워져 있는지 확인하고
	{
		enemyInfo.clear(); //요소를 모두 지움
		count = 0; //카운트도 0부터 시작
	}
	
}

//적 그리기, 메인루프에 들어가면 프레임이 떨어져서 분리함
void EnemyClass::Draw(int num, LPDIRECTDRAWSURFACE7 lpDDSDest)
{
	DDBLTFX ddbltfx;

	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = enemyInfo[num].dwColor;

	HRESULT ddrval = lpDDSDest->Blt(&enemyInfo[num].rPos, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	if (ddrval != DD_OK) DDERRCHK(ddrval);
}

//적 사이즈 늘리는 코드
void EnemyClass::SzUp()
{
	size += 10;
}

//적 움직이는 코드
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

	/*움직이다 화면밖에 나가지 않게 하는 코드*/
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

//유도 코드, 주신 코드에서 내거에 맞게 고침
void EnemyClass::__Chase(int x1, int y1, int speed, RECT* Rect) //쫓을 대상 좌표, 쫓는 대상 대표, 속도;
{
	//// 각각의 거리를 구한다
	float distX = (float)(x1 - Rect->left); //플레이어 x - 적 x좌표
	float distY = (float)(y1 - Rect->top); //플레이어 y - 적 y좌표
	float fDist = (float)sqrt(distX*distX + distY * distY); //제곱근 구하는 함수

	//// 이동 간격을 구한다
	float fStepX = distX / fDist;
	float fStepY = distY / fDist;

	//// 수식에 대입
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