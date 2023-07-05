#pragma once

#include "pch.h"
#include "framework.h"
#include <string>

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
class CClientSocket
{
private:
	//成员方法
	CClientSocket& operator=(const CClientSocket& ss) {};
	CClientSocket(const CClientSocket& ss);
	CClientSocket();
	~CClientSocket();
	BOOL InitSockEnv();
	static void releaseInstance();
	//嵌套类
	class CNewAndDel;
	//成员属性
	CPacket m_packet;
	SOCKET m_sock;
	SOCKET m_client;
	static CClientSocket* m_instance;
	static CNewAndDel m_newdel;
public:
	bool initSocket(const std::string& strIPAddress);
	bool acceptCli();
	int dealCommand();
	bool sendCom(const char* pData, int size);
	bool sendCom(CPacket& pData);
	static CClientSocket* getInstance();
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
};

extern CClientSocket* gpClient;


