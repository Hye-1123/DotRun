#include "RankH.h"

int g_nSize = 0; //exturn �Լ�

/*���� ȭ�� ����Ʈ ���ھ� ���� �Լ�*/
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

/*��ŷ �Լ� ��������� ��, ��ŷ�� ������ ������ ��*/
BOOL RankingCompare(int nScore)
{
	FILE* rankFile;
	int nRankingLast = 0;
	char name[255] = { '\0' };

	rankFile = fopen("rank.bin", "rb");
	if (rankFile == NULL) //��ŷ ������ ����
	{
		return TRUE;
	}
	else
	{
		while (!feof(rankFile)) //�������ٱ��� ������ ����
		{
			fscanf(rankFile, "%d %s\n", &nRankingLast, &name); //�ڵ����� �����ٷ� �Ѿ��?
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

/*��ŷ ���� �д� �Լ�, ������ �о ���δ� ������ ������*/
BOOL RankingReading(Ranking* ranking)
{
	FILE* rankFile;

	rankFile = fopen("rank.bin", "rb");
	if (rankFile != NULL)
	{
		while (!feof(rankFile)) //������ ���� �ƴҶ� TRUE
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

/*��ŷ ����ϴ� �Լ�*/
int RankUp(int score, char* name)
{
	FILE* rankFile;
	int i, j;
	Ranking ranking[10]; //����ü�� int score�� char name[]���� �Ǿ�����
	int nTemp;
	char cTemp[256];

	RankingReading(ranking); //��ŷ�� ����� ������ ���� �о���� �Լ���, �ѱ�� �Ű������� ��� �����

	if (g_nSize == 10)
	{
		ranking[g_nSize - 1].score = score; //������ ������ �ϴ� �־���� ���ؼ� ���� ������
		strcpy(ranking[g_nSize - 1].name, name);
	}
	else
	{
		ranking[g_nSize].score = score; //������ ������ �ϴ� �־���� ���ؼ� ���� ������
		strcpy(ranking[g_nSize].name, name);
		g_nSize++; //�߰��ϴ°Ŵϱ� ������ �������Ѿ� ��
	}


	//�����ڵ�
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

	/*��ŷ ����*/
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

	return ranking[0].score; /*1� ����*/
}
