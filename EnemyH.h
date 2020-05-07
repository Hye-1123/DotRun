#ifndef ENEMY_H
#define ENEMY_H

#include <vector> //c++기능
#include "DxMng.h"

using namespace std;

typedef struct EnemyInfo
{
	RECT rPos;
	DWORD dwColor;
	int nSpeed;
	int MoveType;
}EnemyInfo;

class EnemyClass
{
public:
	EnemyClass();
	~EnemyClass();
	//// 적관련 코드
	void Init();
	void Generation(int num); //적 생성
	void Clear();
	void Draw(int num, LPDIRECTDRAWSURFACE7 lpDDSDest); //적 그리기
	void SzUp(); //적 사이즈 키우는 코드
	void Move(int num, POINT HeroPos); //적 움직이는 코드

	int GetCount();
	RECT GetPos(int num);

protected:
	inline void __Chase(int x1, int y1, int speed, RECT* Rect); //유도 코드
private:
	int count;
	int size;
	
	vector <EnemyInfo> enemyInfo;
};


#endif // !ENEMY_H

