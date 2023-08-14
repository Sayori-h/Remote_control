#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>

#define WM_SEND_PACK     (WM_USER + 1)//发送包数据
#define WM_SEND_PACK_ACK (WM_USER + 2)//发送包数据应答
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

enum {//模式
	CSM_AUTOCLOSE = 1,//CSM Client Socket Mode 自动关闭模式
};

typedef struct PacketData {
	std::string strData;//数据
	UINT nMode;//模式
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT mode,WPARAM nParam=0) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;
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
	//HANDLE hEvent;//方便发送数据时弄得
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(WORD sCmd, const BYTE* pData, size_t nSize/*HANDLE hEvent*/);
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

	/*默认构造函数，初始化客户端网络数据，初始化成员变量为空，初始化函数映射表，创建事件体，开启线程threadFunc()
	内部主要使用的系统函数：WSAStartup ()、CreateEvent()、_beginthreadex()*/
	CClientSocket();

	~CClientSocket();

	//初始化网络环境，采用1.1版本网络库
	BOOL InitSockEnv();

	static void releaseInstance();

	static unsigned __stdcall threadEntry(void* arg);
	//void threadFunc();
	
	//线程函数无限循环获取消息，其目的是得到SendPacket()里post过来的参数，并通过映射表调用本类的SendPack()函数
	void threadFunc2();

	//发送消息
	bool sendCom(const CPacket& pData);

	//发送消息
	bool sendCom(const char* pData, int nSize);

	/*函数功能：主要用于发送数据到服务端，然后接收从服务端返回的消息，并且SendMessage()到对应的界面回调函数中
	//参数1：消息类型
	//参数2：消息w。这里为PacketData的结构指针(请参照PacketData)
	//参数3：消息l。这里为对应处理的窗口句柄*/
	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//嵌套类
	class CNewAndDel;
	//成员属性
	CPacket m_packet;//用于接收数据包
	SOCKET m_sock;//客户端套接字
	static CClientSocket* m_instance;
	static CNewAndDel m_newdel;
	std::vector<char>m_buffer;//动态分配的数据缓冲区
	int m_nIP, m_nPort;//地址和端口
	//list：要获取对方应答的包，对方可能应答多个包（包的数量抖动很剧烈，用vector不合适）
	std::map<HANDLE/*哪个命令*/, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	std::list<CPacket> m_lstSend;//要发送的数据
	bool m_bAutoClose;//自动关闭sock标志
	std::mutex m_lock;//互斥锁
	HANDLE m_hThread;//线程句柄
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC>m_mapFunc;//回调处理函数
	UINT m_nThreadID;//调用回调函数的线程ID
	HANDLE m_eventInvoke;//用于等待线程启动
public:
	//初始化客户端socket，填好服务器地址和端口后，连接到服务器
	bool initSocket();
	int dealCommand();//获取客户端命令
	static CClientSocket* getInstance();
	//bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClose = true);
	
	//该函数内部调用PostThreadMessage()函数来通知线程函数threadFunc()的GetMessage()
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed=true,WPARAM wParam=0);
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
	CPacket& GetPacket();//获取数据包
	void CloseSocket();//关闭socket
	void UpdateAddress(int nIP, int nPort);//更新数据
	void dump(BYTE* pData, size_t nSize);
};

extern CClientSocket* gpClient;


