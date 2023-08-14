#pragma once
#include <Windows.h>
#include <atlimage.h>
#include <string>


class CHuxlTool
{
public:
	static void dump(BYTE* pData, size_t nSize);
	//数据转化为图片
	static int BytesToImage(CImage& image, const std::string& strBuffer);
};

