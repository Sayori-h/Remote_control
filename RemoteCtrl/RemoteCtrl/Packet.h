#pragma once
#include "pch.h"
#include "framework.h"
#pragma pack(push)
#pragma pack(1)
class CPacket
{
private:
public:
	WORD sHead;//包头 固定位：0xFEFF
	DWORD nLength;//包的长度（从控制命令开始，到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//包的数据
	WORD sSum;//和校验
	std::string strOut;//整个包的数据
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(WORD sCmd, const BYTE* pData, size_t nSize);
	CPacket(const BYTE* pData, size_t& nSize);
	CPacket(const CPacket& pack);
	int pacSize();//包数据的大小
	const char* pacData();//包的数据的内容
};
#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;//默认没效果
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、移动、双击
	WORD nButton;//左键、右键、中键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info() {
		isInvalid = 0;
		isDirectory = 0;
		hasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL isInvalid;//是否有效 1无效
	BOOL isDirectory;//是否为目录 1是
	BOOL hasNext;//是否有后续  1有
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;