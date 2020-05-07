#ifndef __FPSMNG_H__
#define __FPSMNG_H__

class CFpsMng
{
public:
    CFpsMng();						// ����Ʈ�� �����ð� ����
    virtual ~CFpsMng();

    void  SetFPS(DWORD dwFPS=0);	// dwFPS == 0, �����ð� ����
	DWORD GetFPS(void);				// ���� FPS�� ��´�
    
    void  FrameWaiting(void);		// ������ �����Ӽ��� ���� ( Polling  )


private:
	DWORD m_dwFramesRendered;
	DWORD m_dwFps;
		
	double m_dOneSecLastTime;		// 1 second
	double m_dOneSecDeltaTime;		// 1 second
    double m_dMPF;					// Millisecond Per Frame
    double m_dLastTime;				// Timer

	double m_dProcTime;				// 1 Frame ó���ð�

	LARGE_INTEGER Frequency;
	LARGE_INTEGER Start;
	LARGE_INTEGER End;
};

#endif