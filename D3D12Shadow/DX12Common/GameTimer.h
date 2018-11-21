#pragma once

//--------------------��ʱ��ͷ�ļ�-----------------//
//ʱ�䣻2018-7-24  by;��һĨϦϼ(lastSunlight)
//˵�������ļ�������Ϸ��������ʱ�ļ�ʱ��
//------------------------------------------------//

#include <Windows.h>


class GameTimer 
{
public:
	GameTimer();
	float TotalTime() const;//������Ϸʱ��
	float DeltaTime() const;//����֡ʱ��
	void Reset();//��ʼ����ʱ��
	void Start();//���¿�ʼ��ʱ
	void Stop();//��ͣ��ʱ
	void Tick();//ÿ֡����
private:
	double m_SecondsPerCount;
	double m_DeltaTime;
	_int64 m_BaseTime;
	_int64 m_PauseTime;
	_int64 m_StopTime;
	_int64 m_PrevTime;
	_int64 m_CurrentTime;
	bool m_Stopped;
};

