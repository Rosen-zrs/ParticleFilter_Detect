#include <opencv2\opencv.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

//创建记录轮廓数量和坐标的结构体
struct num_contours
{
	double x, y;             //轮廓的x、y坐标
	int index;               //轮廓的顺序

	bool operator <(num_contours &m)
	{
		if (y > m.y+50) return false;

		else if (y > m.y-50)
		{
			if (x < m.x) return true;

			else return false;
		}

		else return true;
	}
}num_contours[100];

//创建记录结果的结构体
struct result
{
	double data;
	int num;

	bool operator<(result &m)
	{
		if (data < m.data)return true;
		else return false;
	}
}result[100];

void select_result(Mat & src, int num);
double compare(Mat &src, Mat &model);
vector<Mat> imread_model();
void deal(Mat &src, Mat &model, int m);
void select_result(Mat & src, int num);

int main()
{
	Mat src, gray_src, dest, dst;
	src = imread("D:/Project/Opencv-Project/num_text/num_text_03.png");           //加载原图

	cvtColor(src, gray_src, COLOR_BGR2GRAY);                     //对图像进行预处理
	threshold(gray_src, dest, 150, 255, THRESH_BINARY_INV);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(dest, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));    //提取轮廓

	vector<Rect> rect;
	Rect min_bounding_rect,re_rect;
	float tl_x, br_y, width, height;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contourArea(contours[i]) > 50)
		{
			min_bounding_rect = boundingRect(contours[i]);
			rect.push_back(boundingRect(contours[i]));

			tl_x = (float)min_bounding_rect.tl().x;
			br_y = (float)min_bounding_rect.br().y;
			width = (float)min_bounding_rect.width;
			height = (float)min_bounding_rect.height;

			num_contours[i].x = (tl_x * 2 + width) / 2.0;          //计算轮廓中心点
			num_contours[i].y = (br_y * 2 + height) / 2.0;
			num_contours[i].index = i;
		}
	}
	sort(num_contours, num_contours + contours.size());         //根据轮廓中心点坐标进行排序

	vector<Mat> roi_rect(contours.size());
	for (int j = 0; j < contours.size(); j++)           //ROI处理分割对象
	{
		int k = num_contours[j].index;                  //对分割对象进行预处理
		src(rect[k]).copyTo(roi_rect[j]);
		cvtColor(roi_rect[j], roi_rect[j], COLOR_BGR2GRAY);
		threshold(roi_rect[j], roi_rect[j], 150, 255, THRESH_BINARY_INV);
		select_result(roi_rect[j], j + 1);
	}

	for (int i = 0; i < contours.size(); i++)
	{
		rectangle(src, rect[i], Scalar(0, 0, 255), 1, 8, 0);
	}

	imshow("二值图", dest);
	imshow("原图", src);

	waitKey(0);
	return 0;
}

double compare(Mat &src, Mat &model)               //模板与图像对比计算相似度
{
	Mat re_model;
	resize(model, re_model, src.size());
	int rows, cols;
	uchar *src_data, *model_data;
	rows = re_model.rows;
	cols = re_model.cols*src.channels();
	double percentage,same=0.0,different=0.0;

	for (int i = 0; i < rows; i++)
	{
		src_data = src.ptr<uchar>(i);
		model_data = re_model.ptr<uchar>(i);
		for (int j = 0; j < cols; j++)
		{
			if (src_data[j] == model_data[j])
			{
				same++;
			}
			else
			{
				different++;
			}
		}
	}
	percentage = same / (same + different);
	return percentage;                     //返回相似度
}

vector<Mat> imread_model()
{
	//加载模板
	vector<Mat> myTemplate;

	for (int i = 0; i < 10; i++)
	{
		char name[64];
		sprintf_s(name, "D:/Project/Opencv-Project/num_text/%d.png", i);
		Mat temp = imread(name, 0);
		myTemplate.push_back(temp);
	}

	return myTemplate;
}

void deal(Mat &src, Mat &dst, int m)
{

	threshold(dst, dst, 100, 255, THRESH_BINARY_INV);
	Mat dest;
	resize(src, dest, Size(src.cols *1, src.rows *1), 0, 0, INTER_LINEAR);

	result[m].data = compare(dest, dst);
	result[m].num = m;
}

void select_result(Mat & src, int num)
{
	for (int i = 0; i < 10; i++)
	{
		deal(src, imread_model()[i], i);
	}
	sort(result, result + 10);

	if (result[9].data>0.7)
	{
		cout << "第" << num << "个数字为 " << result[9].num << endl;
		cout << "识别精度为 " << result[9].data << endl << endl;
	}

}