#pragma once


#include <string>
#include<vector>
#include<opencv2\opencv.hpp>
#include "Tracker.h"
using namespace std;
using namespace cv;

namespace mycv {


	class MultiTemplateTracker :public mycv::Tracker
	{
	public:
		
		virtual ~MultiTemplateTracker();
	public:
		enum MatchMethod { SQDIFF = 0, SADIFF = 1 };
		enum MatchStrategy { UNIFORM = 0, NORMAL = 1 };
		struct Params {
			//������Χ����չ
			int expandWidth;
			//ƥ�䷽��
			MatchMethod matchMethod;
			//ģ��ƥ�����
			MatchStrategy matchStrategy;
			//ģ������ٶ�
			double alpha;
			//ģ��ƥ��ʱ��������������
			int numPoints;
			//��������㣨�ض���̬�ֲ����ı�׼��
			Point2d sigma;
			//ģ���ڵĲ�������
			Vec2i xyStep;
			//ģ����ͼ���ڵĻ�������
			Vec2i xyStride;
			Params() {//����Ĭ��ֵ
				expandWidth = 20;
				matchMethod = MatchMethod::SADIFF;
				matchStrategy = MatchStrategy::NORMAL;
				alpha = 0.7;
				numPoints = 500;
				sigma = Point2d(0.5, 0.5);
				xyStep = Vec2i(2, 2);
				xyStride = Vec2i(1, 1);
			}
		};

		//��ʼ��Params
		MultiTemplateTracker(Params _params);
		//public:
		//SingleTemplateTracker(int expandWidth=50);
		
	public:

		//��ʼ��������
		bool init(const Mat&initFrame, const Rect&initBoundingBox);
		//����Ŀ��
		bool track(const Mat&cutrrentFrame, Rect &currentBondingbox);
		//����Ŀ��ģ��
		bool update(Rect&searchBox);

		float MatchTemplate(const Mat&src, const Mat&templ, Rect2i&match_location,
			MatchMethod match_method, Vec2i& xy_step, Vec2i& xy_stride);

		//������һ֡������Χ
		void EstimateSearchArea(const Rect&target_location, Rect& search_area,
			int expand_x, int expand_y);
		// ��ָ���Ĳ�����Χ�ڲ����׶���̬�ֲ���
		void GeneraeRandomSamplePoints(vector<Point2d>&sample_points,
			int num_points = 1000, Point2d sigma = Point2d(0.3, 0.3));
		float MatchTemplate(const Mat&src, const Mat&templ, Rect2i&match_location,
			MatchMethod match_method, const vector<Point2d>&sample_points);
		// ������߶�Ŀ��ģ��
		void GenerateMultiScaleTargetTempletes(const Mat&origin_target, vector<Mat>&multiscale_targets);
		//��ʾ��߶�Ŀ��ģ��
		void ShowMultiScaleTemplates(const vector<Mat>& multiscale_targets);
		float MatchMultiScaleTemplates(const Mat& src, const vector<Mat>&multiscale_templs, Rect2i&best_match_location,
			MatchMethod match_method, MatchStrategy match_strategy, const vector<Point2d>&sample_points, Vec2i& xy_step, Vec2i& xy_stride);
		void UpdateMultiScaleTargetTemplates(const Mat& currentTargetPatch);
	public:

		//Ŀ��ģ��
		Mat TargetTemplate0;
		Mat TargetTemplate;
		//��߶�Ŀ��ģ��
		vector<Mat>MultiScaleTargetTemplates;
		//��ǰ֡���ҵ���Ŀ���
		Rect CurrentBoundingBox;
		//��ǰ֡���ҵ���Ŀ��ͼ���
		Mat CurrentTargetPatch;
		//��һ֡��������Χ
		Rect NextSearchArea;
		//������Χ����չ
		//int expandWidth;

		//��Ƶ֡�ľ�������
		Rect FrameArea;
		//����һ����׼�ض���̬�ֲ������㼯
		vector<Point2d>SamplePoints;
		//ģ��ƥ�����
		//int matchStrategy;
		//�������Ĳ����ṹ��
		Params params;
	};
	typedef MultiTemplateTracker MTTracker;
}