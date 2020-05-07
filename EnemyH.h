#ifndef ENEMY_H
#define ENEMY_H

#include <vector> //c++���
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
	//// ������ �ڵ�
	void Init();
	void Generation(int num); //�� ����
	void Clear();
	void Draw(int num, LPDIRECTDRAWSURFACE7 lpDDSDest); //�� �׸���
	void SzUp(); //�� ������ Ű��� �ڵ�
	void Move(int num, POINT HeroPos); //�� �����̴� �ڵ�

	int GetCount();
	RECT GetPos(int num);

protected:
	inline void __Chase(int x1, int y1, int speed, RECT* Rect); //���� �ڵ�
private:
	int count;
	int size;
	
	vector <EnemyInfo> enemyInfo;
};


#endif // !ENEMY_H

