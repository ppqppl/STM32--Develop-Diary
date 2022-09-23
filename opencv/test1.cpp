#include <opencv2/opencv.hpp>
using namespace cv;

int main()
{
	// 从摄像头读取视频
	VideoCapture capture(0);
	// 循环播放每一帧
	while(waitKey(30) != 27)
	{
		Mat frame;	// 定义一个 Mat 变量，用于存储每一帧的图像
		capture >> frame;	// 读取当前帧
		imshow("读取视频帧",frame);	// 显示当前帧
		//waitKey(30);	// 延时30ms
	}

	system("pause");
	return 0;
}
