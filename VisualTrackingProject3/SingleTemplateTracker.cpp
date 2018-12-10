#include "SingleTemplateTracker.h"
//����
namespace mycv {

	SingleTemplateTracker::SingleTemplateTracker(Params _params)
	{
		cout << "����SingleTemplateTracker::SingleTemplateTracker()" << endl;
		this->params= _params;//�������
		
	}


	SingleTemplateTracker::~SingleTemplateTracker()
	{
	}
	
	//��ʼ��������
	bool SingleTemplateTracker::init(const Mat&initFrame, const Rect&initBoundingBox)
	{
		cout << "����SingleTemplateTracker::init()" << endl;
		//��ʼ����Ƶ֡�ľ�������
		this->FrameArea = Rect(0, 0, initFrame.cols, initFrame.rows);
		//��ȡ��ʼ֡�ϵ�Ŀ��ģ��
		this->TargetTemplate = initFrame(initBoundingBox).clone();//��ʼ֡�ϵ�Ŀ��ģ��
		TargetTemplate0 = initFrame(initBoundingBox).clone();//���ֳ�ʼ֡�ϵ�Ŀ��ģ��
		//������һ֡��������Χ
		this->EstimateSearchArea(initBoundingBox, this->NextSearchArea, params.expandWidth, params.expandWidth );
		//��ʼ����׼����1*1��������㼯
		//int num_points = 1000;
		//Point2d sigma = Point2d(0.3, 0.3);
		this->GeneraeRandomSamplePoints(this->SamplePoints, this->params.numPoints, this->params.sigma);
		
		return false;
	}
	//����Ŀ��
	bool SingleTemplateTracker::track(const Mat&currentFrame, Rect &currentBondingbox)
	{
		cout << "����SingleTemplateTracker::track()" << endl;
		Rect2i match_location(-1, -1,0,0); //int match_method = 1; Vec2i xy_step(2, 2); Vec2i xy_stride(1, 1);
		//��currentFrame��ƥ��ģ�壬���ú�ɫ�Ŀ򻭳���
		//ƥ�����
		if(this->params.matchStrategy == MatchStrategy::UNIFORM)
		this->MatchTemplate(currentFrame(this->NextSearchArea),
			this->TargetTemplate, match_location, this->params.matchMethod, this->params.xyStep, this->params.xyStride);
		/*��ȡ��ǰ֡��NextSearchArea��Ӧ������*/
		if (this->params.matchStrategy == MatchStrategy::NORMAL)//this��ʾ�����һ����Ա�������Ա�����������ڲ�����
		this->MatchTemplate(currentFrame(this->NextSearchArea)/*��ȡ��ǰ֡��NextSearchArea��Ӧ������*/,
			this->TargetTemplate, match_location, this->params.matchMethod, this->SamplePoints);

		//ƥ����֮�󷵻ص�match_location�����������NextSearchArea����ԭ�������
		//���µ���ƥ�������
		match_location.x += this->NextSearchArea.x;
		match_location.y += this->NextSearchArea.y;

		this->CurrentBoundingBox = match_location;
		//ץȡ��ǰ֡��Ŀ��ͼ���
		this->CurrentTargetPatch = currentFrame(this->CurrentBoundingBox).clone();
		//������ٽ��
		currentBondingbox = this->CurrentBoundingBox;
		return false;
	}
	//����Ŀ��ģ��
	bool SingleTemplateTracker::update(Rect&searchBox)
	{
		cout << "����SingleTemplateTracker::update(111)" << endl;
		//����Ŀ���������ģ�ͣ�һ���˲�
		//double alpha = 0.7;//Ȩ��
		cv::addWeighted(this->TargetTemplate, this->params.alpha,  this->CurrentTargetPatch,
			1.0 - this->params.alpha, 0.0, this->TargetTemplate);
		
		//������һ֡�ϵľֲ�������Χ
		this->EstimateSearchArea(this->CurrentBoundingBox, this->NextSearchArea, this->params.expandWidth, this->params.expandWidth);
		searchBox=this->NextSearchArea;//��ֹ��������������������
		return false;
	}

	

	float SingleTemplateTracker::MatchTemplate(const Mat&src, const Mat&templ, Rect2i&match_location, MatchMethod match_method, Vec2i& xy_step, Vec2i& xy_stride)
	{
		//ȷ��ͼ���ʽ
		CV_Assert(src.type() == CV_8UC1 && templ.type() == CV_8UC1);
		//ԭͼ����ģ��ߴ�
		int src_width = src.cols;
		int src_height = src.rows;
		int templ_cols = templ.cols;
		int templ_rows = templ.rows;
		int y_end = src_height - templ_rows + 1;
		int x_end = src_width - templ_cols + 1;
		//��ƥ������У���¼��ƥ���λ�ú�ƥ���
		float match_degree = FLT_MAX;
		int y_match = -1;
		int x_match = -1;
		//���ϵ���ɨ��ԭͼ��
		for (int y = 0; y < y_end; y += xy_stride[1])
		{
			//������ɨ��ԭͼ��
			for (int x = 0; x < x_end; x += xy_stride[0])
			{
				//src(y,x)λ����ģ���ƥ���
				float match_yx = 0.0f;
				//��ģ�����Ͻǵ�templ(0,0)���뵽src(y,x)λ�ã���ģ�����ۼ�ÿ���������ϵ�����ֵ����
				for (int r = 0; r < templ_rows; r += xy_step[1])
				{
					for (int c = 0; c < templ_cols; c += xy_step[0])
					{
						uchar src_val = src.ptr<uchar>(y + r)[x + c];
						uchar templ_val = templ.ptr<uchar>(r)[c];
						if (match_method == MatchMethod::SQDIFF)//SQDIFF
							match_yx += float(std::abs(src_val - templ_val)*std::abs(src_val - templ_val));
						if (match_method == MatchMethod::SADIFF)//SADIFF
							match_yx += float(std::abs(src_val - templ_val));
					}
				}


				//����ʷ��õĲ���Ƚ��бȽϣ��ҳ�������С�ĵ�
				if (match_degree > match_yx) {
					match_degree = match_yx;
					x_match = x;
					y_match = y;

				}
			}

		}
		match_location = Rect2i(x_match, y_match, templ_cols, templ_rows);
		return match_degree;
	}

	void SingleTemplateTracker::EstimateSearchArea(const Rect&target_location, Rect& search_area,
		int expand_x, int expand_y)
	{
		float center_x = target_location.x + 0.5f*target_location.width;
		float center_y = target_location.y + 0.5f*target_location.height;
		search_area.width = target_location.width + expand_x;
		search_area.height = target_location.height + expand_y;
		search_area.x = int(center_x - 0.5f*search_area.width);
		search_area.y = int(center_y - 0.5f*search_area.height);
		search_area &= this->FrameArea;

	}

	// ��ָ���Ĳ�����Χ�ڲ����׶���̬�ֲ���
		void SingleTemplateTracker::GeneraeRandomSamplePoints(vector<Point2d>&sample_points,
			int num_points /*= 1000*/, Point2d sigma /*= Point2d(0.3, 0.3)�����ض���*/)
	{
		RNG rng = theRNG();
		Rect2d sample_area(0.0, 0.0, 1.0, 1.0);//double�͵ĵ㼯��ʵ��ͼƬ��������
		for (int k = 0; k < num_points;) {
			Point2d pt;
			pt.x = sample_area.width / 2.0/*��ֵ0.5*/ + rng.gaussian(sigma.x)/*����0.3*/;
			pt.y = sample_area.height / 2.0 + rng.gaussian(sigma.y);
			if (sample_area.contains(pt)) {
				sample_points.push_back(pt);
				k++;
			}
		}
	}
		//ʹ����������㼯����ģ��ƥ��
		float SingleTemplateTracker::MatchTemplate(const Mat&src, const Mat&templ, Rect2i&match_location,
			MatchMethod match_method, const vector<Point2d>&sample_points) {

			CV_Assert((src.type() == CV_8UC1) && (templ.type() == CV_8UC1));
			//ԭͼ���ģ��ߴ�
			int src_width = src.cols;
			int src_height = src.rows;
			int templ_cols = templ.cols;
			int templ_rows = templ.rows;
			int y_end = src_height - templ_rows + 1;
			int x_end = src_width - templ_cols + 1;
			//��Ծ����ģ���С���������������е�������������
			vector<Point2i>SamplePoints(sample_points.size());
			for (size_t k = 0; k < sample_points.size(); k++)
			{
				const Point2d&ptd = sample_points[k];
				Point2i&pti = SamplePoints[k];//���͵㼯
				pti.x = cvRound(ptd.x*templ_cols);//���������
				pti.y = cvRound(ptd.y*templ_rows);
			}

			//��ƥ������У���¼��ƥ���λ�ú�ƥ���
			float match_degree = FLT_MAX;
			int y_match = -1, x_match = -1;
			//���ϵ���ɨ��ԭͼ��
			for (int y = 0; y < y_end; y++)
			{
				//������ɨ��ԭͼ��
				for (int x = 0; x < x_end; x++)
				{
					//src(y,x)λ������ģ���ƥ���
					float match_yx = 0.0f;
					//���ղ������������ģ����ԭʼͼ���ƥ���
					for (size_t k = 0; k < SamplePoints.size(); k++)
					{
						Point2i& pt = SamplePoints[k];
						uchar src_val = src.ptr<uchar>(y + pt.y)[x + pt.x];
						uchar templ_val = templ.ptr<uchar>(pt.y)[pt.x];
						if (match_method == MatchMethod::SQDIFF)
							match_yx += float(std::abs(src_val - templ_val)*std::abs(src_val - templ_val));
						if (match_method == MatchMethod::SADIFF)
							match_yx += float(std::abs(src_val - templ_val));

					}
					//����ʷ��õĲ���Ƚ��бȽϣ��ҳ�������С�ĵ�
					if (match_degree > match_yx) {
						match_degree = match_yx;
						x_match = x;
						y_match = y;
					}

				}
			}
			match_location = Rect2i(x_match, y_match,templ_cols,templ_rows);
			return match_degree;
		}


}