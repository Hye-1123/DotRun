#ifndef RANK_H_
#define RANK_H_

#include "DxMng.h"

extern int g_nSize; //��ŷ ����Ȯ��

//���� ���� �ڵ�(��ŷ)
typedef struct ranking
{
	int score;
	char name[256];

}Ranking;

int RankBS(); //����Ʈ ���ھ�(1��) �� �޾ƿ���
BOOL RankingCompare(int nScore);
BOOL RankingReading(Ranking* ranking);
int RankUp(int score, char* name);


#endif // !RANK_H_

