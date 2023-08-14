#pragma once
#include "Resource.h"
#include <map>
#include <atlimage.h>
#include <direct.h>
#include <corecrt_io.h>
#include "CServerSocket.h"
#include "LockInfoDialog.h"
#include "CHuxlTool.h"

class CCommand
{
public:
	typedef int(CCommand::* CMDFUNC)();//函数指针
	std::map<int, CMDFUNC>m_mapFunction;// 从命令号到功能的映射
	CLockInfoDialog dlg;
	unsigned threadid;
	//默认构造函数，使用映射表方式对所有远程操作进行映射
	CCommand();
	~CCommand() {};
	int ExcuteCommand(int nCmd);
	//创建磁盘分区的信息，用于查看文件
	int makeDriverInfo();
	//查看指定目录下的文件
	int makeDirectoryInfo();
	//运行文件
	int runFile();
	//下载文件
	int downLoadFile();
	//鼠标操作
	int mouseEvent();
	//发送屏幕图片
	int sendScreen();
	/*******************************锁机***************************************/
	//启动子线程中介跳转，用于锁机
	static unsigned __stdcall threadLockDlg(void* arg);
	//锁机实现
	void threadLockDlgMain();
	//锁机命令
	int LockMachine();
	/***************************************************************************/
	//解锁
	int UnLockMachine();
	//连接测试
	int TestConnect();
	//删除文件
	int DeleteLocalFile();
};

