#include "pch.h"
#include "ClientSocket.h"
#define BUF_SIZE 409600000

std::string GetErrInfo(int wsaErrorCode) {
	LPVOID lpMsgBuf = NULL;
	/*ͨ������ FormatMessage ������̬������һ���ڴ����洢������Ϣ�ַ�����
	������ͷ�����ڴ棬ÿ�ε��� GetErrInfo ����ʱ��������µ��ڴ���䣬���ɵ��ڴ���ò����ͷţ�����ڴ�й©��*/
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
	m_bAutoClose = ss.m_bAutoClose;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
}

CClientSocket::CClientSocket() :
	m_nIP(INADDR_ANY), m_nPort(0), m_sock(INVALID_SOCKET), m_bAutoClose(true), m_hThread(INVALID_HANDLE_VALUE)
	//��ЧIP�Ͷ˿ڣ���ֹ����
{
	m_sock = INVALID_SOCKET;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("��ʼ��WinSock��ʧ�ܣ�"), _T("����"), MB_OK | MB_ICONERROR);
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
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	_endthread();
}

void CClientSocket::threadFunc()
{
	//if (initSocket() == false)return;
	//send�����棬����recv
	char* pBuffer = m_buffer.data();;
	int index = 0;
	initSocket();
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			TRACE("lstSend size:%d\r\n", m_lstSend.size());
			m_lock.lock();
			CPacket& head = m_lstSend.front();//ȡ�������
			m_lock.unlock();
			if (sendCom(head) == false) {
				TRACE("����ʧ�ܣ�\r\n");
				continue;
			}
			auto it = m_mapAck.find(head.hEvent);
			if (it != m_mapAck.end()) {
				auto it0 = m_mapAutoClosed.find(head.hEvent);
				do {
					int length = recv(m_sock, pBuffer + index, BUF_SIZE - index, 0);
					TRACE("recv %d %d\r\n", length,index);
					if (length > 0 || index > 0) {
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);//���
						if (size > 0) {//TODO:�����ļ���/�ļ���Ϣ��ȡ��������
							pack.hEvent = head.hEvent;//�������ҵõ��Լ�
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							TRACE("SetEvent %d %d\r\n", pack.sCmd,it0->second);
							if (it0->second) {
								SetEvent(head.hEvent);
								break;
							}
						}
					}
					else if (length <= 0 && index <= 0) {
						CloseSocket();
						SetEvent(head.hEvent);//�ȵ��������ر�����֮����֪ͨ�������
						m_mapAutoClosed.erase(it0);
						TRACE("SetEvent %d %d\r\n", head.sCmd,it0->second);
						break;
					}
				} while (it0->second == false||index>0);
			}
			m_lock.lock();
			m_lstSend.pop_front();
			m_lock.unlock();
			if (initSocket() == false)initSocket();
		}
		Sleep(1);
	}
	CloseSocket();
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
		AfxMessageBox("�޿���IP��ַ!");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	int err = WSAGetLastError();
	if (ret == -1)
	{
		AfxMessageBox("����ʧ��!");
		TRACE("����ʧ�� %d %s\r\n", err, GetErrInfo(WSAGetLastError()));
		return false;
	}
	TRACE("socket init done!\r\n");
	return true;
}

int CClientSocket::dealCommand()
{
	if (m_sock == -1)return false;
	char* buffer = m_buffer.data();//TODO:���̷߳�������ʱ���ܻ���ֳ�ͻ
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
		if (i > 0 && (i % 16 == 0))strOut += "\n";
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
		m_instance = new CClientSocket();
	return m_instance;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClose)
{
	if (m_sock == INVALID_SOCKET&&m_hThread==INVALID_HANDLE_VALUE) {
		//if(initSocket()==false)return false;
		m_hThread=(HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
		TRACE("start thread\r\n");
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClose));
	m_lstSend.push_back(pack);//Ͷ�����
	m_lock.unlock();
	TRACE("cmd:%d event %08X thread id=%d\r\n", pack.sCmd, pack.hEvent,GetCurrentThreadId());
	WaitForSingleObject(pack.hEvent, INFINITE);
	TRACE("cmd:%d event %08X thread id=%d\r\n", pack.sCmd, pack.hEvent,GetCurrentThreadId());
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end()) {
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.unlock();
		return true;
	}
	return false;
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
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData����װ�κ�����
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
	if ((m_nIP != nIP) || (m_nPort != nPort)) {
		m_nIP = nIP;
		m_nPort = nPort;
	}
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
//������ʹ�����˽�о�ָ̬�����ָ�����Ψһʵ��������һ�����о�̬������ȡ��ʵ��
//static��Ա�����������౾��Ͷ��󣬵����ж������ӵ��һ���ľ�̬��Ա���Ӷ��ڶ�������ǲ���ͨ�����캯��������г�ʼ����
//���岢��ʼ����̬���ݳ�Ա,��̬��Ա�������ඨ����߳�ʼ����ֻ����class body���ʼ����
CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket* gpClient = CClientSocket::getInstance();
CClientSocket::CNewAndDel CClientSocket::m_newdel;


CPacket::CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0), hEvent(INVALID_HANDLE_VALUE) {}

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

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize, HANDLE hEvent)
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

//���
CPacket::CPacket(const BYTE* pData, size_t& nSize) :
	sHead(0), nLength(0), sCmd(0), sSum(0), hEvent(INVALID_HANDLE_VALUE)
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
	sSum = *(WORD*)(pData + i); i += 2;
	WORD sum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sum += BYTE(strData[j]) & 0xFF;//���ֶ����Ʋ����һ���ԣ���������
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


