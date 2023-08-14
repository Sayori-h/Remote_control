#include "pch.h"
#include "CHuxlTool.h"

void CHuxlTool::dump(BYTE* pData, size_t nSize) {
	std::string strOut;
	for (size_t i = 0; i < nSize; i++)
	{
		char buf[16] = "";
		if (i > 0 && (i % 16 == 0))
		{
			strOut += "\n";
		}
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}

int CHuxlTool::BytesToImage(CImage& image, const std::string& strBuffer)
{
	//存入CImage
	BYTE* pData = (BYTE*)strBuffer.c_str();
	//创建一块内存句柄，用于目标流
	//能调整大小的  在内存中分配一个全局的堆
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (!hMem)
	{
		TRACE("内存不足了!");
		Sleep(1);
		return -1;
	}
	IStream* pStream = NULL;//建立内存流
	HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//创建流到全局内存区
	if (hRet == S_OK) {
		ULONG length = 0;//用于记录_OUT_实际写入的字节数
		//利用pData流写入pStream中
		pStream->Write(pData, strBuffer.size(), &length);
		LARGE_INTEGER bg{ 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);//跳转到流的头部
		if ((HBITMAP)image != NULL)image.Destroy();//在加载前先释放之前的
		image.Load(pStream);//加载进缓存
		return hRet;
	}
	return -1;
}

