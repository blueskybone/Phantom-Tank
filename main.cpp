#include <iostream>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#define MAX_LENTH 50
using namespace cv;
using namespace std;

// 按照R:G:B = 3:6:1转灰度图
Mat RGB_to_grey(Mat &img, int w, int h)
{
	//遍历图像
	for (int row = 0; row < h; row++) 
	{
		uchar* uc_pixel = img.data + row*img.step;
		for (int col = 0; col < w; col++) 
		{
			int grey = uc_pixel[0] * 1 / 10 + uc_pixel[2] * 6 / 10 + uc_pixel[2] * 3 / 10;
			uc_pixel[0] = grey;
			uc_pixel[1] = grey;
			uc_pixel[2] = grey;
			uc_pixel += 3;
		}
	}
	// imshow("grey", img);waitKey(3000);
	return img;
}

// 表图添加白色蒙版
Mat above_add_white(Mat &img, int w, int h)
{
	for (int row = 0; row < h; row++)
	{
		uchar* uc_pixel = img.data + row*img.step;
		for (int col = 0; col < w; col++)
		{
			uc_pixel[0] = uc_pixel[0] / 2 + 128;
			uc_pixel[1] = uc_pixel[1] / 2 + 128;
			uc_pixel[2] = uc_pixel[2] / 2 + 128;
			uc_pixel += 3;
		}
	}
	// imshow("white", img);imwrite("white.png", img);waitKey(3000);
	return img;
}

// 里图添加黑色蒙版
Mat under_add_black(Mat &img, int w, int h)
{
	for (int row = 0; row < h; row++)
	{
		uchar* uc_pixel = img.data + row*img.step;
		for (int col = 0; col < w; col++)
		{
			uc_pixel[0] = uc_pixel[0] / 2;
			uc_pixel[1] = uc_pixel[1] / 2;
			uc_pixel[2] = uc_pixel[2] / 2;
			uc_pixel += 3;
		}
	}
	// imshow("black", img); imwrite("black.png", img); waitKey(3000);
	return img;
}

// 合成幻影坦克
Mat create_tank(Mat & img_a, Mat & img_u, Mat & img_r, int w, int h)
{
	
	for (int row = 0; row < h; row++)
	{
		uchar* a_pixel = img_a.data + row * img_a.step;
		uchar* u_pixel = img_u.data + row * img_u.step;
		uchar* r_pixel = img_r.data + row * img_r.step;
		for (int col = 0; col < w; col++)
		{
			int alpha = u_pixel[0] - a_pixel[0] + 255;
			int r_new = u_pixel[0] / (alpha / 255.0);
			r_pixel[0] = r_new;
			r_pixel[1] = r_new;
			r_pixel[2] = r_new;
			r_pixel[3] = alpha;
			a_pixel += 3;
			u_pixel += 3;
			r_pixel += 4;
		}
	}
	//imshow("result", img_r); waitKey(3000);
	return img_r;
}

// 拼接字符串
void strcat_m(char * a, char * b, char * r)
{
	int i = 0, j = 0;
	memset(r, 0, MAX_LENTH);
	while (a[i] != '\0'){ r[i] = a[i]; i++; }
	while (b[j] != '\0'){ r[i++] = b[j++];}
}

// 错误信息
void wrong_msg(int state)
{
	switch (state)
	{
	case 1:
		printf("too few parameters! format like Tank.exe file_1 file_2 outfile\n");
		break;
	case 2:
		printf("too much parameters! format like tank.exe file_1 file_2 outfile\n");
		break;
	default:
		printf("unknwon error!\n");
		exit(1);
		break;
	}
	
}

int main(int argc, char *argv[])
{
	char* above_file;
	char* under_file;
	char re_file[MAX_LENTH] = "result.png";	//默认输出文件名

	// 本想单独写个命令行解析器模仿python，发现太麻烦，凑合下
	if (argc < 2)
	{
		wrong_msg(1);
		return 1;
	}
	else if (argc > 4)
	{
		wrong_msg(2);
		return 2;
	}
	 above_file = argv[1];
	 under_file = argv[2];
	 char * png = ".png";	//输出后缀
	if (argc == 4)
	{
		strcat_m(argv[3], png , re_file);
	}
	
	Mat img_above;
	Mat img_under;
	try
	{
		img_above = imread(above_file);
		img_under = imread(under_file);
	}
	catch (cv::Exception& e)
	{
		const char* err_msg = e.what();
		std::cout << "exception caught:" << err_msg << std::endl;
		return 0;
	}
	int a_cols = img_above.cols;
	int u_cols = img_under.cols;
	int a_rows = img_above.rows;
	int u_rows = img_under.rows;

	// 取长宽
	int width = max(a_cols,u_cols);
	int height = max(a_rows,u_rows);

	//创建白底和黑底图
	Mat white_back = Mat::zeros(Size(width,height),CV_8UC3);
	Mat black_back = Mat::zeros(Size(width,height),CV_8UC3);
	white_back.setTo(Scalar(255,255,255));
	black_back.setTo(Scalar(0,0,0));

	//拼接图像
	Rect rect_a = Rect((width-a_cols)/2,(height-a_rows)/2,a_cols,a_rows);
	img_above.copyTo(white_back(rect_a));
	
	Rect rect_u = Rect((width - u_cols) / 2, (height - u_rows) / 2, u_cols, u_rows);
	img_under.copyTo(black_back(rect_u));

	Mat rgb_a = white_back;
	Mat rgb_u = black_back;

	// 转为灰度图
	Mat grey_a = RGB_to_grey(rgb_a, width, height);
	Mat grey_u = RGB_to_grey(rgb_u, width, height);

	//添加黑白蒙版
	Mat white_a = above_add_white(grey_a, width, height);
	Mat black_u = under_add_black(grey_u, width, height);

	// 创建坦克
	Mat result = Mat::zeros(Size(width, height), CV_8UC4);
	Mat result_r = create_tank(white_a, black_u, result, width, height);

	// 输出
	imwrite(re_file, result_r);

	return 0;
}
