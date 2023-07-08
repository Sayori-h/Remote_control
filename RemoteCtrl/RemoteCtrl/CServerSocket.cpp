#include "pch.h"
#include "CServerSocket.h"
#define BUF_SIZE 4096

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
	if (bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
		TRACE("bind ERRORnum = %d\n", GetLastError());
		return false;
	}
	if (listen(m_serv_sock, 1))return false;
	return true;
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
	if (m_client == -1)return false;
	char* buffer = new char[BUF_SIZE];
	if (buffer==NULL)
	{
		TRACE("内存不足!");
		return -2;
	}
	memset(buffer, 0, BUF_SIZE);
	size_t index = 0;
	while (true)
	{
		size_t len = recv(m_client, buffer+index, BUF_SIZE-index, 0);
		if (len <= 0)
		{
			delete[]buffer;
			return -1;
		}
		TRACE("recv %d\r\n", len);
		index += len;
		m_packet = CPacket((BYTE*)buffer, len);
		if (len > 0)
		{
			memmove(buffer, buffer + len, BUF_SIZE - len);
			index -= len;
			delete[]buffer;
			return m_packet.sCmd;
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
	
	int ret= send(m_client, pack.pacData(), pack.pacSize(), 0);
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
	if ((m_packet.sCmd >= 2)&&(m_packet.sCmd <=4))
	{
		strPath = m_packet.strData;
		return true;
	}
	return false;
}
bool CServerSocket::getMouseEvent(MOUSEEV& mouse){
	if (m_packet.sCmd==5)
	{
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData可以装任何数据

	}
	return false;
}

CPacket& CServerSocket::GetPacket()
{
	return m_packet;
}

void CServerSocket::CloseClient()
{
	closesocket(m_client);
	m_client = INVALID_SOCKET;
}

void CServerSocket::dump(BYTE* pData, size_t nSize) {
	std::string strOut;
	for (size_t i = 0; i < nSize; i++)
	{
		char buf[16] = "";
		if (i > 0 && (i % 16 == 0))
		{
			strOut += "\n";
		}
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
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


CPacket::CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

CPacket& CPacket::operator=(const CPacket& pack)
{
	if (this != &pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
{
	sHead = 0xFEFF;
	nLength = nSize + 4;
	sCmd = nCmd;
	if (nSize>0)
	{
		strData.resize(nSize);
		memcpy((void*)strData.c_str(), pData, nSize);
	}
	else {
		strData.clear();
	}
	sSum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sSum += BYTE(strData[j]) & 0xFF;
	}
}

CPacket::CPacket(const BYTE* pData, size_t& nSize) :sHead(0), nLength(0), sCmd(0), sSum(0)
{
	size_t i = 0;
	for (; i < nSize; i++)
	{
		if (*(WORD*)(pData + i) == 0xFEFF) {
			sHead = *(WORD*)(pData + i);//?
			i += 2;
			break;
		}
	}
	if (i + 4 + 2 + 2 > nSize)
	{
		nSize = 0;
		return;
	}
	nLength = *(DWORD*)(pData + i);//?  nLength长度
	i += 4;
	if (nLength + i > nSize)
	{
		nSize = 0;
		return;
	}
	sCmd = *(WORD*)(pData + i);//?
	i += 2;
	if (nLength > 4)
	{
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.c_str(), pData + i, nLength - 4);
		i += nLength - 4;
	}
	sSum = *(WORD*)(pData + i);
	WORD sum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sum += BYTE(strData[j]) & 0xFF;//保持二进制补码的一致性，消除负数
	}
	if (sum == sSum) {
		nSize = nLength + 2 + 4;
		return;
	}
}

CPacket::CPacket(const CPacket& pack)
{
	sHead = pack.sHead;
	nLength = pack.nLength;
	sCmd = pack.sCmd;
	strData = pack.strData;
	sSum = pack.sSum;
}

int CPacket::pacSize()
{
	return nLength + 6;
}

const char* CPacket::pacData()
{
	strOut.resize(nLength + 6);
	BYTE* pData = (BYTE*)strOut.c_str();
	*(WORD*)pData = sHead; pData += 2;
	*(DWORD*)pData = nLength; pData += 4;
	*(WORD*)pData = sCmd; pData += 2;
	memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
	*(WORD*)pData = sSum; 
	return strOut.c_str();
}
