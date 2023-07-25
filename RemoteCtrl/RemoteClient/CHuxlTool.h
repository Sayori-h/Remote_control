#pragma once
#include <Windows.h>
#include <atlimage.h>
#include <string>


class CHuxlTool
{
public:
	static void dump(BYTE* pData, size_t nSize);
	static int BytesToImage(CImage& image, const std::string& strBuffer);
};

