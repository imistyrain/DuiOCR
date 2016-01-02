#include "mropencv.h"
#include<windows.h>
#include <fstream>
using namespace std;

string strTitle = "ImageLabeler 1.0";
string processingcategory = "../data";
string strext = "*.jpg";
string resultcategory = "result";
Point ptBegin=Point(0,0);
Point ptEnd=Point(0,0);
bool bDrawing = false;
int order= 0;
Mat img;
Mat img2Show;

string int2sring(int n)
{
	stringstream ss;
	ss << n;
	return ss.str();
}
void cvMouseCallback(int mouseEvent, int x, int y, int flags, void* param)
{
	switch (mouseEvent)
	{
	case CV_EVENT_LBUTTONDOWN:
		ptBegin =Point(x, y);
		ptEnd = Point(x, y);
		bDrawing = true;
		break;
	case CV_EVENT_MOUSEMOVE:
		ptEnd = Point(x, y);
		if (bDrawing)
		{
			img2Show = img.clone();
			rectangle(img2Show, Rect(ptBegin, ptEnd), Scalar(0,255,0));
			imshow(strTitle, img2Show);
		}
		break;
	case CV_EVENT_LBUTTONUP:
		bDrawing = false;
		ptEnd = Point(x, y);
		break;
	}
	return;
}


void ProcessFile(string strfile)
{
	img = imread(strfile);
	Rect rect;
	while (true)
	{
		rect.x = min(ptBegin.x, ptEnd.x);
		rect.y = min(ptBegin.y, ptEnd.y);
		rect.height = abs(ptEnd.y - ptBegin.y);
		rect.width = abs(ptEnd.x - ptBegin.x);
		img2Show = img.clone();
		imshow(strTitle, img2Show);
		cvSetMouseCallback(strTitle.c_str(), cvMouseCallback, 0);
		if (waitKey() == 'n')
		{
			break;
		};
	}
	rect.x = max(min(ptBegin.x, ptEnd.x),0);
	rect.y = max(min(ptBegin.y, ptEnd.y),0);
	rect.height = min(abs(ptEnd.y - ptBegin.y),img.rows-rect.y);
	rect.width = min(abs(ptEnd.x - ptBegin.x),img.cols-rect.x);
	Mat img2Save = img(rect);
	imwrite(resultcategory + "/" + int2sring(order++)+ ".bmp", img2Save);
}

bool findallimagesindir(string strDir,vector<string> &files)
{
	WIN32_FIND_DATA FindData;
	HANDLE hError;
	string file2find = strDir + "/" + strext;
	hError = FindFirstFile((LPCTSTR)file2find.c_str(), &FindData);
	if (hError == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	do
	{
		string file = strDir + "/"+FindData.cFileName;
		files.push_back(file);
	} while (::FindNextFile(hError, &FindData));
}

int main()
{
	vector<string> files;
	findallimagesindir(processingcategory, files);	
	for (auto it : files)
	{
		ProcessFile(it);
	}
	return 0;
}