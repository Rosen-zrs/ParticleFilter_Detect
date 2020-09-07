#ifndef __Hist_Calculation_
#define __Hist_Calculation_

#include "headerfile.h"

/* HSVֱ��ͼ��H��S��V���������ֵ */
#define H_MAX 180
#define S_MAX 255
#define V_MAX 255

/* HSVֱ��ͼ��H��S��V������bin��Ŀ */
#define NH 10
#define NS 10
#define NV 10


//����hsvֱ��ͼ�ṹ��
typedef struct histogram
{
	float hist_value_list[NH*NS + NV];			//hsvֱ��ͼ��������
	int list_length;							//hsvֱ��ͼ�������鳤��
}histogram;


histogram* hist_calc(Mat* hsv_frame);
int value_calc(float h, float s, float v);
void normalize_hist(histogram* histo);
float histo_dist_sq(histogram* h1, histogram* h2);

#endif // !__Hist_Calculation_

