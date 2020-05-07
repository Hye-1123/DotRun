#include "RankH.h"

int g_nSize = 0; //exturn 함수

/*메인 화면 베스트 스코어 띄우는 함수*/
int RankBS()
{
	int nbuf;
	FILE* rankFile;

	rankFile = fopen("rank.bin", "rb");
	if (rankFile == NULL)
	{
		return 0;
	}
	else
	{
		fscanf(rankFile, "%d", &nbuf);
		fclose(rankFile);
		return nbuf;
	}
}

/*랭킹 입성 대상자인지 비교, 랭킹의 마지막 순위와 비교*/
BOOL RankingCompare(int nScore)
{
	FILE* rankFile;
	int nRankingLast = 0;
	char name[255] = { '\0' };

	rankFile = fopen("rank.bin", "rb");
	if (rankFile == NULL) //랭킹 파일이 없음
	{
		return TRUE;
	}
	else
	{
		while (!feof(rankFile)) //마지막줄까지 열심히 읽음
		{
			fscanf(rankFile, "%d %s\n", &nRankingLast, &name); //자동으로 다음줄로 넘어가나?
			g_nSize++;
		}

		fclose(rankFile);

		if (nScore > nRankingLast || g_nSize < 10)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*랭킹 파일 읽는 함수, 끝까지 읽어서 전부다 변수에 저장함*/
BOOL RankingReading(Ranking* ranking)
{
	FILE* rankFile;

	rankFile = fopen("rank.bin", "rb");
	if (rankFile != NULL)
	{
		while (!feof(rankFile)) //파일의 끝이 아닐때 TRUE
		{
			fscanf(rankFile, "%d %s\n", &ranking[g_nSize].score, &ranking[g_nSize].name);
			g_nSize++;
		}

		fclose(rankFile);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*랭킹 등록하는 함수*/
int RankUp(int score, char* name)
{
	FILE* rankFile;
	int i, j;
	Ranking ranking[10]; //구조체로 int score와 char name[]으로 되어있음
	int nTemp;
	char cTemp[256];

	RankingReading(ranking); //랭킹이 저장된 파일을 전부 읽어오는 함수임, 넘기는 매개변수에 모두 저장됨

	if (g_nSize == 10)
	{
		ranking[g_nSize - 1].score = score; //마지막 순위에 일단 넣어놓고 비교해서 순위 정렬함
		strcpy(ranking[g_nSize - 1].name, name);
	}
	else
	{
		ranking[g_nSize].score = score; //마지막 순위에 일단 넣어놓고 비교해서 순위 정렬함
		strcpy(ranking[g_nSize].name, name);
		g_nSize++; //추가하는거니까 사이즈 증가시켜야 함
	}


	//정렬코드
	for (i = 0; i < g_nSize; i++)
	{
		for (j = i + 1; j < g_nSize; j++)
		{
			if (ranking[i].score < ranking[j].score)
			{
				nTemp = ranking[j].score;
				strcpy(cTemp, ranking[j].name);

				ranking[j].score = ranking[i].score;
				strcpy(ranking[j].name, ranking[i].name);

				ranking[i].score = nTemp;
				strcpy(ranking[i].name, cTemp);
			}
		}
	}

	/*랭킹 저장*/
	rankFile = fopen("rank.bin", "wb");
	if (g_nSize == 1)
	{
		fprintf(rankFile, "%d %s\n", ranking[0].score, ranking[0].name);
	}
	else
	{
		for (i = 0; i < g_nSize; i++)
		{
			fprintf(rankFile, "%d %s\n", ranking[i].score, ranking[i].name);
		}
	}
	fclose(rankFile);

	return ranking[0].score; /*1등값 리턴*/
}
