#include "GameTimer.h"


GameTimer::GameTimer()
{
	 m_SecondsPerCount = 0;
	 m_DeltaTime = -1;
	 m_BaseTime = 0;
	 m_PauseTime = 0;
	 m_StopTime = 0;
	 m_PrevTime = 0;
	 m_CurrentTime = 0;
	 m_Stopped = false;

	 _int64 CountPerSecond;
	 QueryPerformanceFrequency((LARGE_INTEGER*)&CountPerSecond);
	 m_SecondsPerCount = 1.0 / (double)CountPerSecond;
}


float GameTimer::TotalTime() const
{
	//���������ͣ״̬����ô�ܹ����ĵ�ʱ��Ϊ��ʼ��ͣ��ʱ���ȥ��ͣʱ�����ټ�ȥ��Ϸ��ʼʱ�䡣
	if (m_Stopped)
	{
		return (float)(((m_StopTime - m_PauseTime) - m_BaseTime)*m_SecondsPerCount);
	}
	//�����������״̬���õ�ǰʱ���ȥ��ͣʱ������Ϸ��ʼʱ��
	else
	{
		return (float)(((m_CurrentTime - m_PauseTime) - m_BaseTime)*m_SecondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return (float)m_DeltaTime;
}

void GameTimer::Reset()
{
	//��ʼ����ʱ��
	_int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	m_PrevTime = currentTime;
	m_BaseTime = currentTime;
	m_StopTime = 0;
	m_Stopped = false;
}

void GameTimer::Start()
{
	_int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
	//����ͣ״̬�¿�ʼ��Ϸ
	if (m_Stopped)
	{
		//��ͣʱ�����ڿ�ʼʱ���ȥ��ͣʱ��
		m_PauseTime += (startTime - m_StopTime);
		m_PrevTime = startTime;
		m_StopTime = 0;
		m_Stopped = false;
	}

}

void GameTimer::Stop()
{
	if (!m_Stopped)
	{
		_int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		m_StopTime = currentTime;
		m_Stopped = true;
	}
}

void GameTimer::Tick()
{
	_int64 currentTime;
	//��ȡ��ǰ���õ�ʱ��
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	m_CurrentTime = currentTime;
	//�����ӳ�ʱ��
	m_DeltaTime = (m_CurrentTime - m_PrevTime)*m_SecondsPerCount;
	//��¼��ǰʱ��Ϊ��һ�ε��õ�ǰһʱ��
	m_PrevTime = m_CurrentTime;

	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0;
	}



}
