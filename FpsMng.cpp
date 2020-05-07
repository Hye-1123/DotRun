#include <windows.h>
#include <mmsystem.h>
#include "FpsMng.h"
#include "DxMng.h"

#define dTIMEGETTIME timeGetTime() * 0.001 //1000���� 1�������� ��� �ϱ� ������ 1�ʴ� 1000�̰� �̸� 1�� ���߱� ����?

/*������*/
CFpsMng::CFpsMng()
{
	m_dwFps = 0; //FPS��

	SetFPS(0);
}

/*�Ҹ���*/
CFpsMng::~CFpsMng()
{
}

/*�ʴ� ������ ����*/
void CFpsMng::SetFPS(DWORD dwFPS)
{
	___Trace("FPS : %u\n", dwFPS);

	///// �����ð� ����
	///// �����ð� ����
	if (dwFPS == 0)
	{
		m_dMPF = 0.0;
	}
	else ///// �Ϲ�
	{
		m_dMPF = 1.000 / dwFPS;
		m_dLastTime = dTIMEGETTIME;
	}
}

/*FPS�� ���� ������*/
DWORD CFpsMng::GetFPS()
{
	m_dOneSecDeltaTime = dTIMEGETTIME - m_dOneSecLastTime;	//���� �ð� - ���������� ������ ���� �ʱ�ȭ�� �ð�

	m_dwFramesRendered++; //������ ����

	if (m_dOneSecDeltaTime > 1.000)
	{
		m_dwFps = m_dwFramesRendered;
		m_dwFramesRendered = 0;
		m_dOneSecLastTime = dTIMEGETTIME;
	}

	return m_dwFps;
}

/*��Ʈ �� ���� FPS�� ���߾� ��Ʈ�ϱ� ���� ���*/
void CFpsMng::FrameWaiting()
{
	//1 frame ó���ð�	
	m_dProcTime = dTIMEGETTIME - m_dLastTime;

	/*�� �����Ӵ� �ɸ��� �ð����� ���� �ð� - ���������� ������ �ð��� Ŭ������ ����*/
	//��, �������� �ٲٱ� �������� �ð����� ������ ���°ǵ�...
	while (dTIMEGETTIME - m_dLastTime < m_dMPF);

	/*���� �Ǹ� ������ Ż���ϰ� ���� �ð��� ����*/
	m_dLastTime = dTIMEGETTIME;
}