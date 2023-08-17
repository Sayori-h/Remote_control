#pragma once
#include "Resource.h"
#include <map>
#include <atlimage.h>
#include <direct.h>
#include <corecrt_io.h>
#include "Packet.h"
#include "LockInfoDialog.h"
#include "CHuxlTool.h"
#include <list>

class CCommand
{
public:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>& lstPacket, CPacket& inPacket);//函数指针
	std::map<int, CMDFUNC>m_mapFunction;// 从命令号到功能的映射
	CLockInfoDialog dlg;
	unsigned threadid;
	//默认构造函数，使用映射表方式对所有远程操作进行映射
	CCommand();
	~CCommand() {};
	static void runCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket);
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket);
	//创建磁盘分区的信息，用于查看文件
	int makeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//查看指定目录下的文件
	int makeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//运行文件
	int runFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//下载文件
	int downLoadFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//鼠标操作
	int mouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//发送屏幕图片
	int sendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket);
	/*******************************锁机***************************************/
	//启动子线程中介跳转，用于锁机
	static unsigned __stdcall threadLockDlg(void* arg);
	//锁机实现
	void threadLockDlgMain();
	//锁机命令
	int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket);
	/***************************************************************************/
	//解锁
	int UnLockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//连接测试
	int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//删除文件
	int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
};

