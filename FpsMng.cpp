#include <windows.h>
#include <mmsystem.h>
#include "FpsMng.h"
#include "DxMng.h"

#define dTIMEGETTIME timeGetTime() * 0.001 //1000분의 1단위까지 계산 하기 때문에 1초는 1000이고 이를 1로 맞추기 위함?

/*생성자*/
CFpsMng::CFpsMng()
{
	m_dwFps = 0; //FPS값

	SetFPS(0);
}

/*소멸자*/
CFpsMng::~CFpsMng()
{
}

/*초당 프레임 설정*/
void CFpsMng::SetFPS(DWORD dwFPS)
{
	___Trace("FPS : %u\n", dwFPS);

	///// 지연시간 없음
	///// 지연시간 없음
	if (dwFPS == 0)
	{
		m_dMPF = 0.0;
	}
	else ///// 일반
	{
		m_dMPF = 1.000 / dwFPS;
		m_dLastTime = dTIMEGETTIME;
	}
}

/*FPS의 값을 가져옴*/
DWORD CFpsMng::GetFPS()
{
	m_dOneSecDeltaTime = dTIMEGETTIME - m_dOneSecLastTime;	//현재 시간 - 마지막으로 프레임 값을 초기화한 시간

	m_dwFramesRendered++; //프레임 증가

	if (m_dOneSecDeltaTime > 1.000)
	{
		m_dwFps = m_dwFramesRendered;
		m_dwFramesRendered = 0;
		m_dOneSecLastTime = dTIMEGETTIME;
	}

	return m_dwFps;
}

/*블리트 후 원한 FPS에 맞추어 블리트하기 위한 대기*/
void CFpsMng::FrameWaiting()
{
	//1 frame 처리시간	
	m_dProcTime = dTIMEGETTIME - m_dLastTime;

	/*한 프레임당 걸리는 시간보다 현재 시간 - 마지막으로 수행한 시간이 클때까지 루프*/
	//즉, 프레임을 바꾸기 전까지의 시간동안 루프를 도는건데...
	while (dTIMEGETTIME - m_dLastTime < m_dMPF);

	/*때가 되면 루프를 탈출하고 현재 시간을 저장*/
	m_dLastTime = dTIMEGETTIME;
}