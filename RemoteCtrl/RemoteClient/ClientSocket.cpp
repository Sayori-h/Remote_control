#include "pch.h"
#include "ClientSocket.h"
#define BUF_SIZE 4096

std::string GetErrorInfo(int wsaErrorCode) {
	LPVOID lpMsgBuf = NULL;
	/*ͨ������ FormatMessage ������̬������һ���ڴ����洢������Ϣ�ַ�����
	������ͷ�����ڴ棬ÿ�ε��� GetErrInfo ����ʱ��������µ��ڴ���䣬���ɵ��ڴ���ò����ͷţ�����ڴ�й©��*/
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		wsaErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,0,NULL);
	std::string ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}
CClientSocket::CClientSocket(const CClientSocket& ss) :\
m_sock{ ss.m_sock }, m_client{ ss.m_client }
{
}

CClientSocket::CClientSocket()
{
	m_client = INVALID_SOCKET;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("��ʼ��WinSock��ʧ�ܣ�"), _T("����"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
}

CClientSocket::~CClientSocket()
{
	closesocket(m_sock);
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
//_WINSOCK_DEPRECATED_NO_WARNINGS to disable deprecated API warnings

bool CClientSocket::initSocket(const std::string& strIPAddress)
{
	if (m_sock == -1)return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(strIPAddress.c_str());
	serv_adr.sin_port = htons(9527);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE);
	{
		AfxMessageBox("�޿���IP��ַ!");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret==-1)
	{
		AfxMessageBox("����ʧ��!");
		TRACE("����ʧ�� %d %s\r\n", WSAGetLastError(),GetErrorInfo(WSAGetLastError()));
		return false;
	}
	return true;
}
bool CClientSocket::acceptCli()
{
	sockaddr_in cli_adr;
	int cli_sz = sizeof(cli_adr);
	m_client = accept(m_sock, (sockaddr*)&cli_adr, &cli_sz);
	if (m_client == -1)
	{
		return false;
	}
	return true;
}
int CClientSocket::dealCommand()
{
	if (m_client == -1)return false;
	char* buffer = new char[BUF_SIZE];
	memset(buffer, 0, BUF_SIZE);
	size_t index = 0;
	while (true)
	{
		size_t len = recv(m_client, buffer, sizeof(buffer), 0);
		if (len <= 0)
		{
			return -1;
		}
		//TODO:��������
		index += len;
		m_packet = CPacket((BYTE*)buffer, len);
		if (len > 0)
		{
			memmove(buffer, buffer + len, BUF_SIZE - len);
			index -= len;
			return m_packet.sCmd;
		}
	}
	return -1;
}
bool CClientSocket::sendCom(const char* pData, int size)
{
	if (m_client == -1)return false;
	return send(m_client, pData, size, 0) > 0;
}
bool CClientSocket::sendCom(CPacket& pack)
{
	if (m_client == -1)return false;
	return send(m_client, pack.pacData(), pack.pacSize(), 0) > 0;
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
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData����װ�κ�����

	}
	return false;
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
//CClientSocket g_server;���뵥��֮ǰԭʼ����

//������ʹ�����˽�о�ָ̬�����ָ�����Ψһʵ��������һ�����о�̬������ȡ��ʵ��
//static��Ա�����������౾��Ͷ��󣬵����ж������ӵ��һ���ľ�̬��Ա���Ӷ��ڶ�������ǲ���ͨ�����캯��������г�ʼ����
//���岢��ʼ����̬���ݳ�Ա,��̬��Ա�������ඨ����߳�ʼ����ֻ����class body���ʼ����
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
	}
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
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
}

CPacket::CPacket(const BYTE* pData, size_t& nSize) :sHead(0), nLength(0), sCmd(0), sSum(0)
{
	size_t i = 0;
	for (; i < nSize; i++)
	{
		if (*(WORD*)(pData + i) == 0xFFEF) {
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
	nLength = *(DWORD*)(pData + i);//?  nLength����
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
		sum += BYTE(strData[i]) && 0xFF;//���ֶ����Ʋ����һ���ԣ���������
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
