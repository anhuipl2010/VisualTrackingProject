#include "Tracker.h"
//����

namespace mycv {
	//���캯��
	Tracker::Tracker()
	{
		cout << "����Tracker::Tracker()" << endl;
	}
	//��������
	Tracker::~Tracker()
	{
	}
	//��ʼ��������
	bool Tracker::init(const Mat&initFrame, const Rect&initBoundingBox)
	{
		cout << "����Tracker::init()" << endl;
		return false;
	}
	//����Ŀ��
	bool Tracker::track(const Mat&cutrrentFrame, Rect &currentBondingbox)
	{
		cout << "����Tracker::track()" << endl;
		return false;
	}
	//����Ŀ��ģ��
	bool Tracker::update(Rect&searchBox)
	{
		cout << "����Tracker::update()" << endl;
		return false;
	}

}
