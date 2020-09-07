#include "Hist_Calculation.h"




//统计图像hsv的值纳入直方图数组
int value_calc(float h, float s, float v)
{
	int hd, sd, vd;

	/* 若S或V低于低阈值，判定为无色图像 */
	vd = MIN((int)(v * NV / V_MAX), NV - 1);
	if (s < 0.1 || v < 0.2)
		return NH * NS + vd;

	/* 有色图像 */
	hd = MIN((int)(h * NH / H_MAX), NH - 1);
	sd = MIN((int)(s * NS / S_MAX), NS - 1);

	return sd * NH + hd;

}


//计算HSV图像的直方图
histogram* hist_calc(Mat* hsv_frame)
{
	histogram* histo;
	float* bin;
	int value;

	histo = (histogram*)malloc(sizeof(histogram));
	histo->list_length = NH * NS + NV;
	bin = histo->hist_value_list;
	memset(bin, 0, histo->list_length * sizeof(float));

	for (int i = 0; i < hsv_frame->rows; i++)
	{
		for (int j = 0; j < hsv_frame->cols; j++)
		{
			value = value_calc((float)hsv_frame->at<Vec3b>(i, j)[0], (float)hsv_frame->at<Vec3b>(i, j)[1], (float)hsv_frame->at<Vec3b>(i, j)[2]);
			bin[value] += 1;
		}
	}
	normalize_hist(histo);
	return histo;
}

//直方图归一化
void normalize_hist(histogram* histo)
{
	float* bin;
	float sum = 0, inv_sum;
	int i, n;

	bin = histo->hist_value_list;
	n = histo->list_length;

	for (i = 0; i < n; i++)
		sum += bin[i];
	inv_sum = 1.0 / sum;
	for (i = 0; i < n; i++)
		bin[i] *= inv_sum;
}

//计算巴氏距离
float histo_dist_sq(histogram* h1, histogram* h2)
{
	float* hist1, *hist2;
	float sum = 0;
	int i, n;

	n = h1->list_length;
	hist1 = h1->hist_value_list;
	hist2 = h2->hist_value_list;

	/* D = \sqrt{ 1 - \sum_1^n{ \sqrt{ h_1(i) * h_2(i) } } } */
	for (i = 0; i < n; i++)
		sum += sqrt(hist1[i] * hist2[i]);
	sum = 1.0 - sum;
	return exp(-20 * sum);
}



