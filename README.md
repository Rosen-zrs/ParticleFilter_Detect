# ParticleFilter_Detect
/****************************************************************/
本项目是基于SRP老师给的一个任务，运用粒子滤波检测选定的矩形
目标区域。
首先，随机撒出一百个窗口提取样本信息(简称"放狗")，然后根据狗
反馈的数据生成直方图，再与目标矩形区域的直方图进行比较后，归
一化成权重，然后再根据权重在下一次重新分配放狗的区域和密度(重
取样)，以此实现对运动目标的检测。
附带整个VS工程、效果视频以及当时对项目的汇报ppt(内含对于粒子
滤波以及整个算法思路的详细解释)
/****************************************************************/
