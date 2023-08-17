#include "pch.h"
#include "CServerSocket.h"
#include <list>
#define BUF_SIZE 40960000

CServerSocket::CServerSocket(const CServerSocket& ss) :\
m_serv_sock{ ss.m_serv_sock }, m_client{ ss.m_client }
{
}

CServerSocket::CServerSocket()
{
	m_client = INVALID_SOCKET;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("初始化WinSock库失败！"), _T("错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_serv_sock = socket(PF_INET, SOCK_STREAM, 0);
}

CServerSocket::~CServerSocket()
{
	closesocket(m_serv_sock);
	WSACleanup();
}

BOOL CServerSocket::InitSockEnv()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data) == SOCKET_ERROR)
	{
		return FALSE;
	}
	return TRUE;

}
void CServerSocket::releaseInstance()
{
	if (m_instance != NULL)
	{
		CServerSocket* tmp = m_instance;
		m_instance = NULL;
		delete tmp;
	}
}

bool CServerSocket::initSocket(short port)
{
	if (m_serv_sock == -1)return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(9527);
	if (bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
		TRACE("bind ERRORnum = %d\n", GetLastError());
		return false;
	}
	if (listen(m_serv_sock, 1))return false;
	
	return true;
}

int CServerSocket::run(SOCKET_CALLBACK callback, void* arg,short port)
{
	//1进度的可控性2对接的方便性3可行性评估，提早暴露风险
	// TODO: socket、bind、 listen、accept、 read、write、 close
	// 函数功能：该函数是服务端核心入口函数，用于对外使用的接口
	//参数1：Command的静态处理函数
	//参数2：执行命令的对象，即Command对象
	//参数3：端口号，默认9527
	//返回值：失败返回-1，
	//	其做出以下几个响应动作：
	//		①初始化网络库，即调用InitSocket()
	//		②连接客户端，即调用AcceptClient()
	//		③处理客户端发来的数据，返回命令，即调用DealCommand()
	//		④将命令交给Command类对象，让其完成命令操作
	//		⑤从成员lstPackets表中取得从Command处理后的数据(功能函数push_back的数据，参考Command类的功能函数)，然后调用Send发送回客户端
	//套接字初始化
	bool ret = initSocket(port);
	if (ret == false)return -1;
	std::list<CPacket> lstPackets;
	m_callback = callback;
	m_arg = arg;
	int count = 0;
	while (true)
	{
		if (acceptCli() == false) {
			if (count >= 3) {
				return -2;
			}
			count++;
		}
		int ret = dealCommand();
		if (ret > 0)m_callback(m_arg, ret,lstPackets,m_packet);
		while (lstPackets.size() > 0) {
			sendCom(lstPackets.front());
			lstPackets.pop_front();
		}
		CloseClient();
	}
	return 0;
}

bool CServerSocket::acceptCli()
{
	sockaddr_in cli_adr;
	int cli_sz = sizeof(cli_adr);
	m_client = accept(m_serv_sock, (sockaddr*)&cli_adr, &cli_sz);
	TRACE("m_client=%d\r\n", m_client);
	if (m_client == -1)
	{
		return false;
	}
	return true;
}
int CServerSocket::dealCommand()
{
	/*函数说明：接受发数据，解包，取得包里的命令并返回
返回值： 成功得到命令 ，  失败得到 -1
*/
	if (m_client == -1)return false;
	char* buffer = new char[BUF_SIZE];
	if (buffer == NULL)
	{
		TRACE("内存不足!");
		return -2;
	}
	memset(buffer, 0, BUF_SIZE);
	static size_t index = 0;
	while (true)
	{
		size_t len = recv(m_client, buffer + index, BUF_SIZE - index, 0);//实际接收到的长度
		if ((int)len <= 0)
		{
			delete[]buffer;
			return -1;
		}
		//TRACE("recv %d\r\n", len);
		index += len;
		len = index;
		m_packet = CPacket((BYTE*)buffer, len);//解包
		if (len > 0)
		{
			memmove(buffer, buffer + len, BUF_SIZE - len);
			index -= len;
			delete[]buffer;
			return m_packet.sCmd;//接收成功，返回命令
		}
	}
	delete[]buffer;
	return -1;
}
bool CServerSocket::sendCom(const char* pData, int size)
{
	if (m_client == -1)return false;
	return send(m_client, pData, size, 0) > 0;
}
bool CServerSocket::sendCom(CPacket& pack)
{
	if (m_client == -1)return false;
	TRACE("Png Size %d\r\n", pack.pacSize());
	int ret = send(m_client, pack.pacData(), pack.pacSize(), 0);
	return ret > 0;
}
CServerSocket* CServerSocket::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CServerSocket();
	}
	return m_instance;
}
bool CServerSocket::getFilePath(std::string& strPath)
{
	if (((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) || (m_packet.sCmd == 9))
	{
		strPath = m_packet.strData;
		return true;
	}
	return false;
}
bool CServerSocket::getMouseEvent(MOUSEEV& mouse) {
	if (m_packet.sCmd == 5)
	{
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData可以装任何数据
		return true;
	}
	return false;
}

CPacket& CServerSocket::GetPacket()
{
	return m_packet;
}

void CServerSocket::CloseClient()
{
	if (m_client != INVALID_SOCKET) {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
}


class CServerSocket::CNewAndDel {
public:
	CNewAndDel() {
		CServerSocket::getInstance();
	}
	~CNewAndDel() {
		CServerSocket::releaseInstance();
	}
};
//CServerSocket g_server;引入单例之前原始做法

//单例：使用类的私有静态指针变量指向类的唯一实例，并用一个公有静态方法获取该实例
//static成员的所有者是类本身和对象，但是有多个对象拥有一样的静态成员。从而在定义对象是不能通过构造函数对其进行初始化。
//定义并初始化静态数据成员,静态成员不能在类定义里边初始化，只能在class body外初始化。
CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket* gpServer = CServerSocket::getInstance();
CServerSocket::CNewAndDel CServerSocket::m_newdel;

