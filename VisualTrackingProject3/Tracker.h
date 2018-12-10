#pragma once
#include <string>
#include<vector>
#include<opencv2\opencv.hpp>
using namespace std;
using namespace cv;

namespace mycv {  //����һ������tracker�Ŀռ�,������openCV�ռ��ڵĶ����ظ�
	class Tracker
	{
	public:
		Tracker();
		virtual ~Tracker();
		//��ʼ��������
		virtual bool init(const Mat&initFrame,const Rect&initBoundingBox);//����Ϊ�麯����ʹ�õ��õ�ʱ��ֻ�ܵ�������ĺ���
		//����Ŀ��
		virtual bool track(const Mat&cutrrentFrame,Rect &currentBondingbox);
		//����Ŀ��ģ��
		virtual bool update(Rect&searchBox);//������������
	};

}


