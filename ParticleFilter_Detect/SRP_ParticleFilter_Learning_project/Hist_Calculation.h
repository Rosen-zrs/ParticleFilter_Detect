#ifndef __Hist_Calculation_
#define __Hist_Calculation_

#include "headerfile.h"

/* HSV直方图中H、S、V分量的最大值 */
#define H_MAX 180
#define S_MAX 255
#define V_MAX 255

/* HSV直方图中H、S、V分量的bin数目 */
#define NH 10
#define NS 10
#define NV 10


//定义hsv直方图结构体
typedef struct histogram
{
	float hist_value_list[NH*NS + NV];			//hsv直方图分量数组
	int list_length;							//hsv直方图分量数组长度
}histogram;


histogram* hist_calc(Mat* hsv_frame);
int value_calc(float h, float s, float v);
void normalize_hist(histogram* histo);
float histo_dist_sq(histogram* h1, histogram* h2);

#endif // !__Hist_Calculation_

