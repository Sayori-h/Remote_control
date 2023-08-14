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
	//����CImage
	BYTE* pData = (BYTE*)strBuffer.c_str();
	//����һ���ڴ���������Ŀ����
	//�ܵ�����С��  ���ڴ��з���һ��ȫ�ֵĶ�
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (!hMem)
	{
		TRACE("�ڴ治����!");
		Sleep(1);
		return -1;
	}
	IStream* pStream = NULL;//�����ڴ���
	HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//��������ȫ���ڴ���
	if (hRet == S_OK) {
		ULONG length = 0;//���ڼ�¼_OUT_ʵ��д����ֽ���
		//����pData��д��pStream��
		pStream->Write(pData, strBuffer.size(), &length);
		LARGE_INTEGER bg{ 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);//��ת������ͷ��
		if ((HBITMAP)image != NULL)image.Destroy();//�ڼ���ǰ���ͷ�֮ǰ��
		image.Load(pStream);//���ؽ�����
		return hRet;
	}
	return -1;
}

