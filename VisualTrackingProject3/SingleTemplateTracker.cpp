#include "SingleTemplateTracker.h"
//子类
namespace mycv {

	SingleTemplateTracker::SingleTemplateTracker(Params _params)
	{
		cout << "运行SingleTemplateTracker::SingleTemplateTracker()" << endl;
		this->params= _params;//传入参数
		
	}


	SingleTemplateTracker::~SingleTemplateTracker()
	{
	}
	
	//初始化跟踪器
	bool SingleTemplateTracker::init(const Mat&initFrame, const Rect&initBoundingBox)
	{
		cout << "运行SingleTemplateTracker::init()" << endl;
		//初始化视频帧的矩形区域
		this->FrameArea = Rect(0, 0, initFrame.cols, initFrame.rows);
		//提取初始帧上的目标模板
		this->TargetTemplate = initFrame(initBoundingBox).clone();//初始帧上的目标模板
		TargetTemplate0 = initFrame(initBoundingBox).clone();//保持初始帧上的目标模板
		//估计下一帧的搜索范围
		this->EstimateSearchArea(initBoundingBox, this->NextSearchArea, params.expandWidth, params.expandWidth );
		//初始化标准区间1*1随机采样点集
		//int num_points = 1000;
		//Point2d sigma = Point2d(0.3, 0.3);
		this->GeneraeRandomSamplePoints(this->SamplePoints, this->params.numPoints, this->params.sigma);
		
		return false;
	}
	//跟踪目标
	bool SingleTemplateTracker::track(const Mat&currentFrame, Rect &currentBondingbox)
	{
		cout << "运行SingleTemplateTracker::track()" << endl;
		Rect2i match_location(-1, -1,0,0); //int match_method = 1; Vec2i xy_step(2, 2); Vec2i xy_stride(1, 1);
		//在currentFrame上匹配模板，并用红色的框画出来
		//匹配策略
		if(this->params.matchStrategy == MatchStrategy::UNIFORM)
		this->MatchTemplate(currentFrame(this->NextSearchArea),
			this->TargetTemplate, match_location, this->params.matchMethod, this->params.xyStep, this->params.xyStride);
		/*提取当前帧上NextSearchArea对应的区域*/
		if (this->params.matchStrategy == MatchStrategy::NORMAL)//this表示是类的一个成员函数或成员变量，不是内部变量
		this->MatchTemplate(currentFrame(this->NextSearchArea)/*提取当前帧上NextSearchArea对应的区域*/,
			this->TargetTemplate, match_location, this->params.matchMethod, this->SamplePoints);

		//匹配完之后返回的match_location坐标是相对于NextSearchArea区域原点的坐标
		//重新调整匹配点坐标
		match_location.x += this->NextSearchArea.x;
		match_location.y += this->NextSearchArea.y;

		this->CurrentBoundingBox = match_location;
		//抓取当前帧的目标图像块
		this->CurrentTargetPatch = currentFrame(this->CurrentBoundingBox).clone();
		//输出跟踪结果
		currentBondingbox = this->CurrentBoundingBox;
		return false;
	}
	//跟新目标模型
	bool SingleTemplateTracker::update(Rect&searchBox)
	{
		cout << "运行SingleTemplateTracker::update(111)" << endl;
		//更新目标表面特征模型，一阶滤波
		//double alpha = 0.7;//权重
		cv::addWeighted(this->TargetTemplate, this->params.alpha,  this->CurrentTargetPatch,
			1.0 - this->params.alpha, 0.0, this->TargetTemplate);
		
		//更新下一帧上的局部搜索范围
		this->EstimateSearchArea(this->CurrentBoundingBox, this->NextSearchArea, this->params.expandWidth, this->params.expandWidth);
		searchBox=this->NextSearchArea;//防止搜索区域溢出，区域相减
		return false;
	}

	

	float SingleTemplateTracker::MatchTemplate(const Mat&src, const Mat&templ, Rect2i&match_location, MatchMethod match_method, Vec2i& xy_step, Vec2i& xy_stride)
	{
		//确认图像格式
		CV_Assert(src.type() == CV_8UC1 && templ.type() == CV_8UC1);
		//原图像与模板尺寸
		int src_width = src.cols;
		int src_height = src.rows;
		int templ_cols = templ.cols;
		int templ_rows = templ.rows;
		int y_end = src_height - templ_rows + 1;
		int x_end = src_width - templ_cols + 1;
		//在匹配过程中，记录最匹配的位置和匹配度
		float match_degree = FLT_MAX;
		int y_match = -1;
		int x_match = -1;
		//从上到下扫描原图像
		for (int y = 0; y < y_end; y += xy_stride[1])
		{
			//从左到右扫描原图像
			for (int x = 0; x < x_end; x += xy_stride[0])
			{
				//src(y,x)位置与模板的匹配度
				float match_yx = 0.0f;
				//将模板左上角的templ(0,0)对齐到src(y,x)位置，在模板内累加每个采样点上的像素值差异
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


				//与历史最好的差异度进行比较，找出差异最小的点
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

	// 在指定的采样范围内产生阶段正态分布点
		void SingleTemplateTracker::GeneraeRandomSamplePoints(vector<Point2d>&sample_points,
			int num_points /*= 1000*/, Point2d sigma /*= Point2d(0.3, 0.3)不能重定义*/)
	{
		RNG rng = theRNG();
		Rect2d sample_area(0.0, 0.0, 1.0, 1.0);//double型的点集，实际图片都是整型
		for (int k = 0; k < num_points;) {
			Point2d pt;
			pt.x = sample_area.width / 2.0/*均值0.5*/ + rng.gaussian(sigma.x)/*方差0.3*/;
			pt.y = sample_area.height / 2.0 + rng.gaussian(sigma.y);
			if (sample_area.contains(pt)) {
				sample_points.push_back(pt);
				k++;
			}
		}
	}
		//使用随机采样点集进行模板匹配
		float SingleTemplateTracker::MatchTemplate(const Mat&src, const Mat&templ, Rect2i&match_location,
			MatchMethod match_method, const vector<Point2d>&sample_points) {

			CV_Assert((src.type() == CV_8UC1) && (templ.type() == CV_8UC1));
			//原图像和模板尺寸
			int src_width = src.cols;
			int src_height = src.rows;
			int templ_cols = templ.cols;
			int templ_rows = templ.rows;
			int y_end = src_height - templ_rows + 1;
			int x_end = src_width - templ_cols + 1;
			//针对具体的模板大小，将采样点数组中的坐标点进行缩放
			vector<Point2i>SamplePoints(sample_points.size());
			for (size_t k = 0; k < sample_points.size(); k++)
			{
				const Point2d&ptd = sample_points[k];
				Point2i&pti = SamplePoints[k];//整型点集
				pti.x = cvRound(ptd.x*templ_cols);//浮点变整型
				pti.y = cvRound(ptd.y*templ_rows);
			}

			//在匹配过程中，记录最匹配的位置和匹配度
			float match_degree = FLT_MAX;
			int y_match = -1, x_match = -1;
			//从上到下扫描原图像
			for (int y = 0; y < y_end; y++)
			{
				//从左到右扫描原图像
				for (int x = 0; x < x_end; x++)
				{
					//src(y,x)位置上与模板的匹配度
					float match_yx = 0.0f;
					//按照采样点数组计算模板与原始图像的匹配度
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
					//与历史最好的差异度进行比较，找出差异最小的点
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