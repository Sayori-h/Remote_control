#include "pch.h"
#include "CServerSocket.h"

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
bool CServerSocket::initSocket()
{
	if (m_serv_sock == -1)return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(9527);
	if (bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		return false;
	if (listen(m_serv_sock, 1))return false;
	return true;
}
bool CServerSocket::acceptCli()
{
	sockaddr_in cli_adr;
	int cli_sz = sizeof(cli_adr);
	m_client = accept(m_serv_sock, (sockaddr*)&cli_adr, &cli_sz);
	if (m_client == -1)
	{
		return false;
	}
	return true;
}
int CServerSocket::dealCommand()
{
	if (m_client == -1)return false;
	char buffer[1024] = "";
	while (true)
	{
		int len = recv(m_client, buffer, sizeof(buffer), 0);
		if (len <= 0)
		{
			return -1;
		}
		//TODO:处理命令
	}
	return 0;
}
bool CServerSocket::sendCom(const char* pData, int size)
{
	if (m_client == -1)return false;
	return send(m_client, pData, size, 0) > 0;
}
CServerSocket* CServerSocket::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CServerSocket();
	}
	return m_instance;
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



