#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "StatusDlg.h"
#include "resource.h"
#include "RemoteClientDlg.h"
#include "CHuxlTool.h"
#include <map>
					    
//#define WM_SEND_DATA    (WM_USER + 2)//发送数据
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

	//初始化操作,创建线程
	int InitController();

	//启动 用于显示MFC界面，本次显示远控控制台m_remoteDlg的主界面窗口
	int Invoke(CWnd*&pMainWnd);

	//发送消息
	//LRESULT SendMessage(MSG msg);
	
	//更新网络服务器的地址
	//通知ClientSocket类对象调用UpdateAddress()，用于更新服务端IP和端口
	void UpdateAddress(int nIP, int nPort);
	
	//由套接字转换为controller过一个桥：斩断界面和数据的耦合
	//通知ClientSocket类对象调用DealCommand()函数处理得到命令  返回值：命令
	int DealCommand();
	
	//通知ClientSocket类对象关闭套接字
	void CloseSocket();
	//返回成员m_packet
	CPacket& GetPacket();
	
	/*----------发送数据包-------------------
	1、查看磁盘分区     ||   2、查看指定目录下的文件
	3、打开文件	      ||   4、下载文件
	5、鼠标操作	      ||   6、发送屏幕的内容
	7、锁机			  ||   8、解锁
	9、删除文件		  ||   2001、测试连接
	函数说明：通知ClientSocket类对象调用SendPacket()发送数据包命令
	参数1：传达给处理的窗口界面，数据包收到后，需要应答的窗口
	参数2：命令
	参数3：是否自动关闭，默认为true
	参数4：需要发送的数据包
	参数5：数据长度
	参数6：该参数用于传递文件流指针，用于文件下载，其余情况为0
	返回值：成功返回true,失败返回fasle*/
	bool SendCommandPacket(HWND hWnd, int nCmd,bool bAutoClose = true,
		 BYTE* pData = NULL,size_t nLength = 0,WPARAM wParam = 0
		/*std::list<CPacket>* plstPacks=NULL不关心应答应答通过发消息，不再需要队列*/);
		
	
	//获取图片，图传处理
	//获取数据，并且使用工具库CTool将字节转化为图片格式返回
	//参数1：图片CImage容器
	int GetImage(CImage& image);
	
	//下载完成，隐藏下载窗口，恢复光标
	void DownloadEnd();

	//下载文件
	int DownFile(CString& strPath);

	//开启远程图传线程，然后显示图传界面
	void StartWatchScreen();
protected:
	//默认构造函数，初始化成员变量，指明图传、下载窗口的父窗口为主界面窗口
	CClientController();

	//析构函数，等待线程结束再析构
	~CClientController();

	/*图传线程函数，控制发送频率，循环调用SendCommandPack()发送图传命令
	内部主要使用的系统函数: GetTickCount64()*/
	void threadWatchScreen();

	/*该操作需要另起线程来完成下载操作，由于启动线程的特殊要求，首先，启动的线程不能带有this指针，
	也就说该线程函数为静态函数，但是在线程函数中需要用到this指针来操作（成员函数、成员变量等)，
	因此需要中转机制完成，把this指针当成参数传给线程，再通过this指针调用成员函数*/
	static void threadWatchScreenEntry(void* arg);

	void threadFunc();

	//unsigned为了获取线程ID
	static unsigned __stdcall threadEntry(void* arg);

	static void releaseInstance();
	//LRESULT OnSendPack(UINT nMSG, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMSG, WPARAM wParam, LPARAM lParam);
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
	//回调函数
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMSG,WPARAM wParam, LPARAM lParam);
	//消息函数映射
	static std::map<UINT, MSGFUNC>m_mapFunc;
	CWatchDialog m_watchDlg;//远程桌面对话框
	CRemoteClientDlg m_remoteDlg;//主界面控制对话框
	CStatusDlg m_statusDlg;//正在下载对话框
	HANDLE m_hThread;//线程
	HANDLE m_hThreadWatch;//远程桌面线程
	bool m_isClosed;//监视是否关闭
	CString m_strRemote;//下载文件的远程路径
	CString m_strLocal;//下载文件的本地保存路径
	unsigned m_nThreadID;//线程ID
	static CClientController* m_instance;//全局唯一对象
	class CNewAndDel;
	static CNewAndDel m_newdel;
};

