#ifndef RANK_H_
#define RANK_H_

#include "DxMng.h"

extern int g_nSize; //랭킹 개수확인

//파일 관련 코드(랭킹)
typedef struct ranking
{
	int score;
	char name[256];

}Ranking;

int RankBS(); //베스트 스코어(1등) 값 받아오기
BOOL RankingCompare(int nScore);
BOOL RankingReading(Ranking* ranking);
int RankUp(int score, char* name);


#endif // !RANK_H_

