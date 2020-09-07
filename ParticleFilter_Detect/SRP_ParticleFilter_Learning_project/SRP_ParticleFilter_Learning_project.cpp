#include "headerfile.h"
#include "Hist_Calculation.h"


#define particle_number 100				//每次撒下粒子的个数

//定义粒子结构体
typedef struct particles
{
	int x, y;							//t时刻粒子的坐标
	int init_x, init_y;					//粒子的中心点
	int previous_x, previous_y;			//t-1时刻粒子的坐标
	float scale;						//当前粒子窗口的尺寸
	float prescale;						//上一帧粒子窗口的尺寸
	float weight;						//粒子的权重
	Rect rect;							//粒子所在的矩形框
	Mat Hist;
	histogram* histo;					/* 粒子描述的区域直方图指针 */
}PARTICLE;

//创建粒子容器
PARTICLE particle[particle_number];


Point first_point;								//定义鼠标点击初始点
Rect select_rect, mobile_rect;					//定义鼠标选定矩形
bool istrack = false, isshow = false, mouse_flag = false;
int select_over = 0;
void onMouse(int event, int x, int y, int, void*);


/****hsv空间用到的变量****/
int hist_size[] = { 16,16,16 };
float hrange[] = { 0,180.0 };
float srange[] = { 0,256.0 };
float vrange[] = { 0,256.0 };

const float *ranges[] = { hrange,srange,vrange };

int channels[] = { 0,1,2 };

/****有关粒子窗口变化用到的相关变量****/
/* 转换模型（二阶自回归模型）参数 */
int A1 = 2;
int A2 = -1;
int B0 = 1;
double sigmax = 1.0;
double sigmay = 0.5;
double sigmas = 0.001;

//粒子排序函数声明
int particle_sort(const void *p1, const void *p2);
PARTICLE* resample(PARTICLE* particle, int n);

Mat frame;

int main(int argc, unsigned char* argv[])
{
	VideoCapture capture(1);
	capture >> frame;

	namedWindow("capture", WINDOW_AUTOSIZE);
	setMouseCallback("capture", onMouse, 0);

	histogram *ref_histo, *sample_histo;
	histogram standard_histo;
	Mat hsv_frame,first_sample,track_image;
	Mat sample_hist, image_hist;
	PARTICLE *pParticle, *new_particles;

	while (capture.read(frame))
	{
		if (!capture.isOpened())
		{
			return -1;
		}
		cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

		if (istrack)
		{
			//当摄像头接收到第一帧时
			if (select_over ==1)
			{
				//获得截取的目标样本
				first_sample = Mat(hsv_frame, select_rect);

				//计算初始样本的直方图
				calcHist(&first_sample, 1, channels, Mat(), sample_hist, 3, hist_size, ranges);
				normalize(sample_hist, sample_hist);

				//计算直方图
				ref_histo=hist_calc(&first_sample);
				standard_histo = *ref_histo;
				
				pParticle = particle;
				//初始化粒子数组
				for (int i = 0; i < particle_number; i++)
				{
					pParticle->x = cvRound(select_rect.x + 0.5*select_rect.width);
					pParticle->y = cvRound(select_rect.y + 0.5*select_rect.height);
					pParticle->rect = select_rect;
					pParticle->init_x = particle->x;
					pParticle->init_y = particle->y;
					pParticle->previous_x = pParticle->x;
					pParticle->previous_y = pParticle->y;
					pParticle->histo = ref_histo;
					pParticle->Hist = sample_hist;
					pParticle->weight = 0;
					pParticle->prescale = 1;
					pParticle->scale = 1;
					pParticle++;
				}
				
			}
			else if(select_over >=2)
			{
				float sum = 0.0;
				RNG rng;					//随机数产生器
				pParticle = particle;

				//放狗
				for (int i = 0; i < particle_number;i++)
				{
					int x, y;
					int xpre, ypre;
					double s, pres;

					xpre = pParticle->x;
					ypre = pParticle->y;
					pres = pParticle->scale;

					/****更新粒子的矩形区域即粒子中心****/
					x = cvRound(A1*(pParticle->x - pParticle->init_x) + A2 * (pParticle->previous_x - pParticle->init_x) +
						B0 * rng.gaussian(sigmax) + pParticle->init_x);
					pParticle->x = max(0, min(x, frame.cols - 1));

					y = cvRound(A1*(pParticle->y - pParticle->init_y) + A2 * (pParticle->previous_y- pParticle->init_y) +
						B0 * rng.gaussian(sigmay) + pParticle->init_y);
					pParticle->y = max(0, min(y, frame.rows - 1));

					s = A1 * (pParticle->scale - 1) + A2 * (pParticle->prescale - 1) + B0 * (rng.gaussian(sigmas)) + 1.0;
					pParticle->scale = max(1.0, min(s, 3.0));

					//先记录t-1时刻粒子坐标
					pParticle->previous_x = xpre;
					pParticle->previous_y = ypre;
					pParticle->prescale = pres;

					//更新t时刻粒子的矩形框
					pParticle->rect.x = max(0, min(cvRound(pParticle->x - 0.5*pParticle->scale*pParticle->rect.width), frame.cols));
					pParticle->rect.y = max(0, min(cvRound(pParticle->y - 0.5*pParticle->scale*pParticle->rect.height), frame.rows));
					pParticle->rect.width = min(cvRound(pParticle->rect.width), frame.cols - pParticle->rect.x);
					pParticle->rect.height = min(cvRound(pParticle->rect.height), frame.rows - pParticle->rect.y);

					//计算样本的直方图
					track_image = Mat(hsv_frame, pParticle->rect);
					calcHist(&track_image, 1, channels, Mat(), image_hist, 3, hist_size, ranges);
					normalize(image_hist, image_hist);

					///****更新粒子的权值****/
					pParticle->weight = 1.0 - compareHist(sample_hist, image_hist, HISTCMP_BHATTACHARYYA);

					pParticle->histo=hist_calc(&track_image);
					track_image.release();

					pParticle->weight = histo_dist_sq(pParticle->histo, &standard_histo);
					

					///****累加粒子权值****/
					sum += pParticle->weight;
					pParticle++;

				}

				pParticle = particle;
				//对粒子的权重进行归一化
				for (int i = 0; i < particle_number; i++)
				{
					pParticle->weight /= sum;
					cout << pParticle->weight << endl;
					pParticle++;
				}

				//根据粒子权重进行排序
				pParticle = particle;
				qsort(pParticle, particle_number, sizeof(PARTICLE), &particle_sort);

				//重采样
				new_particles=resample(particle, particle_number);
				free(pParticle);
				pParticle = new_particles;
			}

			pParticle = particle;

			//创建目标矩形
			Rect tracking_goal(0, 0, 0, 0);
			tracking_goal = pParticle->rect;
			
			//创建目标矩形区域
			Rect tracking_rect(tracking_goal);
			pParticle = particle;
			//绘制滑动矩形
			for (int i = 0; i < particle_number; i++)
			{
				rectangle(frame, pParticle->rect, Scalar(255,0,0), 1, 8, 0);
				pParticle++;
			}

			//绘制目标矩形
			rectangle(frame, tracking_rect, Scalar(0, 0, 255), 3, 8, 0);
			select_over++;
			if (select_over > 2)
			{
				select_over = 2;
			}
		}

		if (isshow)
		{
			rectangle(frame, select_rect, Scalar(0, 0, 255), 3, LINE_8, 0);
		}

		imshow("capture", frame);


		if (waitKey(30) > 0)
		{
			break;
		}
	}
	return 0;
}


//鼠标响应事件函数
void onMouse(int event, int x, int y, int, void*)
{
	//定义鼠标接收事件逻辑
	if (mouse_flag)
	{
		select_rect.x = MIN(first_point.x, x);
		select_rect.y = MIN(first_point.y, y);
		select_rect.width = abs(x - first_point.x);
		select_rect.height = abs(y - first_point.y);
		select_rect &= Rect(0, 0, frame.cols, frame.rows);//保证所选矩形框在视频显示区域之内
	}
	//鼠标左键响应
	if (event == EVENT_LBUTTONDOWN)
	{
		mouse_flag = true;
		istrack = false;
		isshow = true;
		select_over = 0;
		first_point = Point(x, y);
		select_rect = Rect(x, y, 0, 0);
	}
	//鼠标右键响应
	if (event == EVENT_RBUTTONDOWN)
	{
		mouse_flag = false;
		istrack = true;
		isshow = false;
		select_over = 1;
	}
}


//粒子排序函数
int particle_sort(const void *p1, const void *p2)
{
	PARTICLE* _p1 = (PARTICLE*)p1;
	PARTICLE* _p2 = (PARTICLE*)p2;
	if (_p1->weight > _p2->weight)
	{
		return -1;
	}
	else if (_p1->weight < _p2->weight)
	{
		return 1;
	}
	return 0;
}


//重采样函数
PARTICLE* resample(PARTICLE* particle, int n)
{
	PARTICLE* new_particles;
	int i, j, nw, k = 0;

	new_particles = (PARTICLE*)malloc(n * sizeof(PARTICLE));

	/* 将权值为w的粒子复制[n*w]份 */
	for (i = 0; i < n; i++)
	{
		nw = cvRound(particle[i].weight * n);
		for (j = 0; j < nw; j++)
		{
			new_particles[k++] = particle[i];
			if (k == n)
				goto exit;
		}
	}
	/* 未填满则都用原最大权值粒子补满 */
	while (k < n)
		new_particles[k++] = particle[0];
exit:
	return new_particles;
}
