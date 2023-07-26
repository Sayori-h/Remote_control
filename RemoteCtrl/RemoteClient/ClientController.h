#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "StatusDlg.h"
#include "resource.h"
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "CHuxlTool.h"
#include <map>
					    
#define WM_SEND_PACK    (WM_USER + 1)//发送包数据
#define WM_SEND_DATA    (WM_USER + 2)//发送数据
#define WM_SHOW_STATUS  (WM_USER + 3)//展示状态
#define WM_SHOW_WATCH   (WM_USER + 4)//远程监视
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息处理

//业务逻辑和流程，是随时可能发生改变的！！！！！
//业务逻辑和流程，是随时可能发生改变的！！！！！
//业务逻辑和流程，是随时可能发生改变的！！！！！

class CClientController
{
public:
	//获取全局唯一对象
	static CClientController* getInstance();
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*&pMainWnd);
	//发送消息
	LRESULT SendMessage(MSG msg);
	//更新网络服务器的地址
	void UpdateAddress(int nIP, int nPort);
	//由套接字转换为controller过一个桥：斩断界面和数据的耦合
	int DealCommand();
	void CloseSocket();
	CPacket& GetPacket();
	bool SendPacket(const CPacket& pack);
	/*---------------------------------------
	1、查看进盘分区    ||   2、查看指定目录下的文件
	3、打开文件	    ||   4、下载文件
	5、鼠标操作	    ||   6、发送屏幕的内容
	7、锁机			||   8、解锁
	9、删除文件		||   2001、测试连接
	返回值：是命令号，如果<0则错误*/
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0);
	int GetImage(CImage& image);
	int DownFile(CString& strPath);
	void StartWatchScreen();
protected:
	CClientController();
	~CClientController();
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);
	void threadFunc();
	//unsigned为了获取线程ID
	static unsigned __stdcall threadEntry(void* arg);
	void threadDownFile();
	static void threadEntryOfDownFile(void* arg);
	static void releaseInstance();
	LRESULT OnSendPack(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMSG, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMSG,
		WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC>m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadDown;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//监视是否关闭
	CString m_strRemote;//下载文件的远程路径
	CString m_strLocal;//下载文件的本地保存路径
	unsigned m_nThreadID;
	static CClientController* m_instance;
	class CNewAndDel;
	static CNewAndDel m_newdel;
};

