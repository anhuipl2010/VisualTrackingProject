#include <iostream>
#include <string>
#include<vector>
#include<opencv2\opencv.hpp>
#include "Tracker.h"
//#include"SingleTemplateTracker.h"
#include"MultiTemplateTracker.h"
#include"datasets.h"




using namespace std;
using namespace cv;
using namespace mycv;


namespace global {
	bool paused = true;//��������Ҽ�����ͣ��־
	Mat displayImg;//����ѡ��Ŀ��ʱ�������϶��켣
	bool selectObject = false;//selectObject��ʼֵΪfalse
	bool isRoiReady = 0;//ROI�����Ƿ��Ѿ�ѡ���
	Point origin;//ROI�������Ͻ���ʼλ��
	Rect selectedRoi;//����ͨ�����ѡ���ROI����

	static void onMouse(int event, int x, int y, int, void*)
	{
		if (selectObject)//�����������󣬸ö���俪ʼִ��
		{//��ס����϶����ʱ�򣬸������Ӧ����
		 //�ᱻ���ϴ��������ϼ���Ŀ����εĴ���
			selectedRoi.x = MIN(x, origin.x);
			selectedRoi.y = MIN(y, origin.y);
			selectedRoi.width = std::abs(x - origin.x);
			selectedRoi.height = std::abs(y - origin.y);
			selectedRoi &= Rect(0, 0, displayImg.cols, displayImg.rows);//����Խ��
			rectangle(displayImg, selectedRoi, Scalar(0, 0, 255), 1);//��������ѡ��ۼ�
		}
		switch (event)
		{
			//���ڵ�һ֡������������selectObject������Ϊtrue,�϶���꿪ʼѡ��Ŀ��ľ�������
		case CV_EVENT_LBUTTONDOWN:
			origin = Point(x, y);
			selectedRoi = Rect(x, y, 0, 0);
			selectObject = true;
			isRoiReady = false;
			break;
			//ֱ�����̧�𣬱�־Ŀ������ѡ����ϣ�selectObject������Ϊfalse
		case CV_EVENT_LBUTTONUP:
			selectObject = false;
			if (selectedRoi.width > 0 && selectedRoi.height > 0)
				isRoiReady = true;
			cout << "ѡ�еľ�������Ϊ��" << selectedRoi << endl;
			break;
			//�����Ҽ�����ͣ��ʼ
		case CV_EVENT_RBUTTONDOWN:
			paused = !paused;
		}
	}
}

/*namespace datsets {
	//const string datasets_dir = "\\";
	const string video1 = "123.mp4";//datasetsir + "123.mp4";
	int video1_strat_frame = 5;

	const string video = video1;
	int strat_frame = video1_strat_frame;

}*/

int main(int argc, char*argv[]) {
    //ָ�����ݼ�,��ʼ֡����ʼĿ��
    mycv::DataSet dataset = mycv::dataset21;
	
	
	//����һ����ȡ��Ƶ������VideoCapture�Ķ���
	VideoCapture capture;
	//����Videocapture�����open��������Ƶ�ļ�
	capture.open(dataset.video_name);
	//�ж�����Ƿ񱻴�
	CV_Assert(capture.isOpened());
	//��ȡ��Ƶ��Ϣ
	const int FrameCount = (int)capture.get(VideoCaptureProperties::CAP_PROP_FRAME_COUNT);
	const int FrameWidth = (int)capture.get(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
	const int FrameHeight = (int)capture.get(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
	const Rect FrameArea(0, 0, FrameWidth, FrameHeight);
	//���ôӵڼ�֡��ʼ��ȡ
	int frameIndex = dataset.strat_frame;
	capture.set(VideoCaptureProperties::CAP_PROP_POS_FRAMES,double(frameIndex ));

	//������ʾ���ٹ��̵Ĵ���
	const string winName = "Tracking Window";
	namedWindow(winName, 1);
	setMouseCallback(winName, global::onMouse, 0);


	//��ȡָ������ʼ֡
	Mat CurrentFrame, WorkFrame;
	capture >> CurrentFrame;
	CV_Assert(!CurrentFrame.empty());
	cout << "��ǰ֡������" << frameIndex << endl;
	frameIndex++;

	//����ʼ֡��ѡ��Ŀ��
	while (!global::isRoiReady)
	{
		//����ʼ֡������displayImg��
		CurrentFrame.copyTo(global::displayImg);
		//�ڰ�����������̧�����֮�䣬selectObjectΪ��
		//selectROI�����������ƶ����ϱ仯��ֱ��̧����������
		//selectObjectΪ�٣�selectedROI����ѡ�е�Ŀ����ο�
		if (global::selectObject&&global::selectedRoi.width > 0 && global::selectedRoi.height>0)
		{
			Mat roi_img(global::displayImg, global::selectedRoi);
			cv::bitwise_not(roi_img, roi_img);//��ѡ�е�����ͼ��ת��ʾ
		}
		//��ʾ���ѡ�����
		imshow(winName, global::displayImg);
		waitKey(10);
	}

	//���lock_roi==true,�ͱ�ʾ���Ķ�ѡ��������Ч
	if (dataset.lock_roi)
		global::selectedRoi = dataset.start_roi;
	cout << "��ʼ֡�ϵ�Ŀ��λ��" <<global::selectedRoi<< endl;


	cout << "��������������ʵ������ʼ��Ŀ�������������" << endl;

	//mycv::STTracker::Params params = mycv::STTracker::Params();//�����ռ䣺���ࣺ���ṹ�壬��ֵĬ�ϲ���
	//params.alpha = 0.7;//�޸�Ĭ�ϲ���
	//params.numPoints = 800;
	//Ptr<mycv::Tracker> tracker=new mycv::SingleTemplateTracker(params);//����Ϊָ������mycv::Tracker tracker;�����ָ��ֱ�Ӹ�ֵ�������ָ�룬��������SingleTemplateTracker()
	
	
	//��������ﶼ���еĸ�������������Ը�Ϊ��߶�Ŀ��ģ���ʱ��ֻ��Ҫ������������ĵ�
	mycv::MTTracker::Params mtparams = mycv::MTTracker::Params();//�����ռ䣺���ࣺ���ṹ�壬��ֵĬ�ϲ���
	mtparams.alpha = 0.7;//�޸�Ĭ�ϲ���
	mtparams.numPoints = 800;
	mtparams.sigma = Point2d(0.4, 0.4);
	mtparams.expandWidth = 50;
	Ptr<mycv::Tracker> tracker=new mycv::MultiTemplateTracker(mtparams);//����Ϊָ������mycv::Tracker tracker;�����ָ��ֱ�Ӹ�ֵ�������ָ�룬��������SingleTemplateTracker()
	

	cvtColor(CurrentFrame, WorkFrame, CV_BGR2GRAY);
	tracker->init(WorkFrame,global::selectedRoi);//����Ϊָ������tracker.init();
	cout << "��������Ҽ��������١�����" << endl;

	//����ѭ����������Ƶͼ�����У�����Ŀ��
	for (; frameIndex < FrameCount;) {
		//���û���źţ������������һ֡ͼ��
		if (!global::paused)
		{
			capture >> CurrentFrame;
			CV_Assert(!CurrentFrame.empty());
			cout << "��ǰ����֡��" << endl;
			frameIndex++;

			//�������ͼ�񿽱���displayImg����
			CurrentFrame.copyTo(global::displayImg);
			//ת��Ϊ�Ҷ�ͼ��
			cvtColor(CurrentFrame, WorkFrame, CV_BGR2GRAY);

			//��ʼ����
			Rect CurrentBondingBox;
			tracker->track(WorkFrame,CurrentBondingBox);//mycv::Tracker tracker���ָ�룬����


			//����Ŀ��ģ��
			Rect NextSearchBox;
			tracker->update(NextSearchBox);
			
			//��ʾ��ǰ֡���ٽ��ͼ��
			rectangle(global::displayImg, CurrentBondingBox, Scalar(0, 0, 255), 2);
			rectangle(global::displayImg, NextSearchBox, Scalar(255, 0, 0), 2);
			imshow(winName, global::displayImg); 
			waitKey(30);
		}
		else
		{
			//��ʾ��ǰ֡���ٽ��ͼ��
			imshow(winName, global::displayImg);
			waitKey(300);
		}



	}




}

