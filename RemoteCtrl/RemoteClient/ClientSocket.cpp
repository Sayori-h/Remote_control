#include "pch.h"
#include "ClientSocket.h"
#define BUF_SIZE 409600000

std::string GetErrInfo(int wsaErrorCode) {
	LPVOID lpMsgBuf = NULL;
	/*通过调用 FormatMessage 函数动态分配了一块内存来存储错误信息字符串。
	如果不释放这块内存，每次调用 GetErrInfo 函数时都会产生新的内存分配，而旧的内存则得不到释放，造成内存泄漏。*/
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		wsaErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::string ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}
CClientSocket::CClientSocket(const CClientSocket& ss)
{
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
}

CClientSocket::CClientSocket() :m_nIP(INADDR_ANY), m_nPort(0)
//无效IP和端口，防止连上
{
	m_sock = INVALID_SOCKET;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("初始化WinSock库失败！"), _T("错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_buffer.resize(BUF_SIZE);
	memset(m_buffer.data(), 0, BUF_SIZE);
}

CClientSocket::~CClientSocket()
{
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
	WSACleanup();
}

BOOL CClientSocket::InitSockEnv()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data) == SOCKET_ERROR)
	{
		return FALSE;
	}
	return TRUE;

}
void CClientSocket::releaseInstance()
{
	if (m_instance != NULL)
	{
		CClientSocket* tmp = m_instance;
		m_instance = NULL;
		delete tmp;
	}
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket*thiz = (CClientSocket*)arg;
	thiz->threadFunc();
}

void CClientSocket::threadFunc()
{
	if (initSocket() == false)return;
	//send在外面，这里recv
	std::string strBuffer;
	strBuffer.resize(BUF_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			CPacket& head = m_lstSend.front();//取到这个包
			if (sendCom(head) == false) {
				TRACE("发送失败！\r\n");
				continue;
			}
			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));

			std::list<CPacket> lstRecv;
			int length = recv(m_sock, pBuffer + index, BUF_SIZE - index, 0);
			if (length > 0 || index > 0) {
				index += length;
				size_t size = (size_t)index;
				CPacket pack((BYTE*)pBuffer, size);//解包
				if (size>0) {//TODO:对于文件夹/文件信息获取会有问题
					pack.hEvent = head.hEvent;//让他能找得到自己
					pr.first->second.push_back(pack);
					SetEvent(head.hEvent);
				}
			}
			else if (length <= 0 && index <= 0) {
				CloseSocket();
			}
					m_lstSend.pop_front();
		}
	}
}

bool CClientSocket::initSocket()
{
	if (m_sock != INVALID_SOCKET)CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1)return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("无可用IP地址!");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	int err = WSAGetLastError();
	if (ret == -1)
	{
		AfxMessageBox("连接失败!");
		TRACE("连接失败 %d %s\r\n", err, GetErrInfo(WSAGetLastError()));
		return false;
	}
	return true;
}

int CClientSocket::dealCommand()
{
	if (m_sock == -1)return false;
	char* buffer = m_buffer.data();//TODO:多线程发送命令时可能会出现冲突
	static size_t index = 0;
	while (true)
	{
		size_t len = recv(m_sock, buffer + index, BUF_SIZE - index, 0);
		if (((int)len <= 0) && ((int)index <= 0))return -1;
		TRACE("recv len =%d(0x%08X) index =%d(0x%08X)\r\n", len, len, index, index);
		//dump((BYTE*)buffer, len);
		index += len;
		len = index;
		TRACE("recv len =%d(0x%08X) index =%d(0x%08X)\r\n", len, len, index, index);
		m_packet = CPacket((BYTE*)buffer, len);
		TRACE("command %d\r\n", m_packet.sCmd);
		if (len > 0)
		{
			memmove(buffer, buffer + len, index - len);
			index -= len;
			return m_packet.sCmd;
		}
	}
	return -1;
}

void CClientSocket::dump(BYTE* pData, size_t nSize) {
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

bool CClientSocket::sendCom(const CPacket& pack)
{
	TRACE("m_sock=%d\r\n", m_sock);
	//dump((BYTE*)pack.pacData(), pack.pacSize());
	if (m_sock == -1)return false;
	std::string strOut;
	pack.pacData(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}
bool CClientSocket::sendCom(const char* pData, int nSize)
{
	if (m_sock == -1)return false;
	return send(m_sock, pData, nSize, 0) > 0;
}
CClientSocket* CClientSocket::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CClientSocket();
	}
	return m_instance;
}
bool CClientSocket::getFilePath(std::string& strPath)
{
	if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4))
	{
		strPath = m_packet.strData;
		return true;
	}
	return false;
}
bool CClientSocket::getMouseEvent(MOUSEEV& mouse) {
	if (m_packet.sCmd == 5)
	{
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData可以装任何数据

	}
	return false;
}

CPacket& CClientSocket::GetPacket()
{
	return m_packet;
}

void CClientSocket::CloseSocket()
{
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}

void CClientSocket::UpdateAddress(int nIP, int nPort)
{
	m_nIP = nIP;
	m_nPort = nPort;
}

class CClientSocket::CNewAndDel {
public:
	CNewAndDel() {
		CClientSocket::getInstance();
	}
	~CNewAndDel() {
		CClientSocket::releaseInstance();
	}
};
//单例：使用类的私有静态指针变量指向类的唯一实例，并用一个公有静态方法获取该实例
//static成员的所有者是类本身和对象，但是有多个对象拥有一样的静态成员。从而在定义对象是不能通过构造函数对其进行初始化。
//定义并初始化静态数据成员,静态成员不能在类定义里边初始化，只能在class body外初始化。
CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket* gpClient = CClientSocket::getInstance();
CClientSocket::CNewAndDel CClientSocket::m_newdel;


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
		hEvent = pack.hEvent;
	}
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize,HANDLE hEvent)
{
	sHead = 0xFEFF;
	nLength = nSize + 4;
	sCmd = nCmd;
	if (nSize > 0)
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
	this->hEvent = hEvent;
}

//解包
CPacket::CPacket(const BYTE* pData, size_t& nSize) :
	sHead(0), nLength(0), sCmd(0), sSum(0),hEvent(INVALID_HANDLE_VALUE)
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
	sSum = *(WORD*)(pData + i); i += 2;
	WORD sum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sum += BYTE(strData[j]) & 0xFF;//保持二进制补码的一致性，消除负数
	}
	if (sum == sSum) {
		nSize = i;
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
	hEvent = pack.hEvent;
}

int CPacket::pacSize()
{
	return nLength + 6;
}

const char* CPacket::pacData(std::string& strOut) const
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


