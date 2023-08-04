#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>

#define WM_SEND_PACK    (WM_USER + 1)//发送包数据
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
	//std::string strOut;//整个包的数据
	HANDLE hEvent;//方便发送数据时弄得
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(WORD sCmd, const BYTE* pData, size_t nSize, HANDLE hEvent);
	CPacket(const BYTE* pData, size_t& nSize);
	CPacket(const CPacket& pack);
	int pacSize();//包数据的大小
	const char* pacData(std::string& strOut) const;//包的数据的内容
};
#pragma pack(pop)
#include <mutex>
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
	static void threadEntry(void* arg);
	void threadFunc();
	void threadFunc2();
	bool sendCom(const CPacket& pData);
	bool sendCom(const char* pData, int nSize);
	void SendPack(UINT nMsg, WPARAM wParam/*缓冲区的值*/, LPARAM lParam/*缓冲区的长度*/);
	//嵌套类
	class CNewAndDel;
	//成员属性
	CPacket m_packet;
	SOCKET m_sock;
	static CClientSocket* m_instance;
	static CNewAndDel m_newdel;
	std::vector<char>m_buffer;
	int m_nIP, m_nPort;//地址和端口
	//list：要获取对方应答的包，对方可能应答多个包（包的数量抖动很剧烈，用vector不合适）
	std::map<HANDLE/*哪个命令*/, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	std::list<CPacket> m_lstSend;//要发送的数据
	bool m_bAutoClose;
	std::mutex m_lock;
	HANDLE m_hThread;
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC>m_mapFunc;
	
public:
	bool initSocket();
	int dealCommand();
	static CClientSocket* getInstance();
	bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClose = true);
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
	CPacket& GetPacket();
	void CloseSocket();
	void UpdateAddress(int nIP, int nPort);
	void dump(BYTE* pData, size_t nSize);
};

extern CClientSocket* gpClient;


