#include "pch.h"
#include "ClientSocket.h"
#define BUF_SIZE 100*1024*1024

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
	m_hThread = INVALID_HANDLE_VALUE;
	m_bAutoClose = ss.m_bAutoClose;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it != ss.m_mapFunc.end(); it++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}

CClientSocket::CClientSocket() :
	m_nIP(INADDR_ANY), m_nPort(0), m_sock(INVALID_SOCKET), m_bAutoClose(true), m_hThread(INVALID_HANDLE_VALUE){
	//无效IP和端口，防止连上
	if (InitSockEnv() == FALSE){
		MessageBox(NULL, _T("初始化WinSock库失败！"), _T("错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_eventInvoke = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT) {
		TRACE("网络消息处理线程启动失败！\r\n");
	}
	CloseHandle(m_eventInvoke);
	m_buffer.resize(BUF_SIZE);
	memset(m_buffer.data(), 0, BUF_SIZE);
	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEND_PACK,&CClientSocket::SendPack},
		{0,NULL}
	};
	for (int i = 0; funcs[i].message; i++) {
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("插入失败，消息值：%d 函数值：%08x 序号：%d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}
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

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}

//void CClientSocket::threadFunc()
//{
//	//if (initSocket() == false)return;
//	//send在外面，这里recv
//	char* pBuffer = m_buffer.data();
//	int index = 0;
//	initSocket();
//	while (m_sock != INVALID_SOCKET) {
//		if (m_lstSend.size() > 0) {
//			TRACE("lstSend size:%d\r\n", m_lstSend.size());
//			m_lock.lock();
//			CPacket& head = m_lstSend.front();//取到这个包
//			m_lock.unlock();
//			if (sendCom(head) == false) {
//				TRACE("发送失败！\r\n");
//				continue;
//			}
//			auto it = m_mapAck.find(head.hEvent);
//			if (it != m_mapAck.end()) {
//				auto it0 = m_mapAutoClosed.find(head.hEvent);
//				do {
//					int length = recv(m_sock, pBuffer + index, BUF_SIZE - index, 0);
//					TRACE("recv %d %d\r\n", length,index);
//					if (length > 0 || index > 0) {
//						index += length;
//						size_t size = (size_t)index;
//						CPacket pack((BYTE*)pBuffer, size);//解包
//						if (size > 0) {//TODO:对于文件夹/文件信息获取会有问题
//							pack.hEvent = head.hEvent;//让他能找得到自己
//							it->second.push_back(pack);
//							memmove(pBuffer, pBuffer + size, index - size);
//							index -= size;
//							TRACE("SetEvent %d %d\r\n", pack.sCmd,it0->second);
//							if (it0->second) {
//								SetEvent(head.hEvent);
//								break;
//							}
//						}
//					}
//					else if (length <= 0 && index <= 0) {
//						CloseSocket();
//						SetEvent(head.hEvent);//等到服务器关闭命令之后，再通知事情完成
//						if (it0 != m_mapAutoClosed.end()) {
//							TRACE("SetEvent %d %d\r\n", head.sCmd, it0->second);
//						}
//						else {
//							TRACE("异常的情况，没有对应的pair对!\r\n");
//						}
//						break;
//					}
//				} while (it0->second == false||index>0);
//			}
//			m_lock.lock();
//			m_lstSend.pop_front();
//			m_mapAutoClosed.erase(head.hEvent);
//			m_lock.unlock();
//			if (initSocket() == false)initSocket();
//		}
//		Sleep(1);
//	}
//	CloseSocket();
//}

void CClientSocket::threadFunc2()
{
	SetEvent(m_eventInvoke);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		TRACE("Get Message:%08x\r\n", msg.message);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

bool CClientSocket::initSocket(){
	if (m_sock != INVALID_SOCKET)CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1)return false;

	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	TRACE("addr %08X nIP %08X\r\n", inet_addr("127.0.0.1"), m_nIP);
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
		TRACE("连接失败 %d %s\r\n", err, GetErrInfo(WSAGetLastError()).c_str());
		return false;
	}
	TRACE("socket init done!\r\n");
	return true;
}

int CClientSocket::dealCommand()
{
	if (m_sock == -1)return false;
	char* buffer = m_buffer.data();//new char[BUFFER_SIZE]
	static size_t index = 0;//*注意，此处用来静态，为了在下面循环中保留index的值
	while (true)
	{
		/*目的是将所有的包都放入缓冲区*/
		size_t len = recv(m_sock, buffer + index, BUF_SIZE - index, 0);//实际接收到的长度
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
			memmove(buffer, buffer + len, index - len);//追加
			index -= len;
			return m_packet.sCmd;//接收成功，返回命令
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

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{//定义一个消息的数据结构(数据和数据长度，模式：是接到一个应答包就结束还是多个应答包)
	//回调消息的数据结构（HWND:窗口句柄 MESSAGE(后来砍掉了):回调什么消息))
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	if (initSocket() == true) {
		int ret = send(m_sock, (char*)data.strData.c_str(), (int)data.strData.size(), 0);
		if (ret > 0) {
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUF_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sock != INVALID_SOCKET) {
				int length = recv(m_sock,pBuffer+index,BUF_SIZE-index, 0);
				if (length > 0||index>0) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						TRACE("ack pack %d to hWnd %08x\r\n", pack.sCmd, hWnd);
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CSM_AUTOCLOSE) {
							CloseSocket();
							return;
						}
					}
					index -= nLen;
					memmove(pBuffer, pBuffer + index, nLen);
				}
				else {//对方关闭了套接字或者网络设备异常
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, 1);
				}
			}
		}
		else {
			CloseSocket();
			//网络终止处理
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else {
		//错误处理
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
	}
}

CClientSocket* CClientSocket::getInstance()
{
	if (m_instance == NULL)
		m_instance = new CClientSocket();
	return m_instance;
}

//bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClose)
//{
//	if (m_sock == INVALID_SOCKET&&m_hThread==INVALID_HANDLE_VALUE) {
//		//if(initSocket()==false)return false;
//		m_hThread=(HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
//		TRACE("start thread\r\n");
//	}
//	m_lock.lock();
//	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
//	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClose));
//	m_lstSend.push_back(pack);//投入队列
//	m_lock.unlock();
//	TRACE("cmd:%d event %08X thread id=%d\r\n", pack.sCmd, pack.hEvent,GetCurrentThreadId());
//	WaitForSingleObject(pack.hEvent, INFINITE);
//	TRACE("cmd:%d event %08X thread id=%d\r\n", pack.sCmd, pack.hEvent,GetCurrentThreadId());
//	std::map<HANDLE, std::list<CPacket>&>::iterator it;
//	it = m_mapAck.find(pack.hEvent);
//	if (it != m_mapAck.end()) {
//		m_lock.lock();
//		m_mapAck.erase(it);
//		m_lock.unlock();
//		return true;
//	}
//	return false;
//}

bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed,WPARAM wParam)
{
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;
	std::string strOut;
	pack.pacData(strOut);
	//post不关心是否投递成功，send关心是否投递成功
	bool ret=PostThreadMessage(m_nThreadID,WM_SEND_PACK,
		(WPARAM)new PACKET_DATA(strOut.c_str(),strOut.size(),nMode,wParam),(LPARAM)hWnd);
	return ret;
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
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData可以装任何数据
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
//单例：使用类的私有静态指针变量指向类的唯一实例，并用一个公有静态方法获取该实例
//static成员的所有者是类本身和对象，但是有多个对象拥有一样的静态成员。从而在定义对象是不能通过构造函数对其进行初始化。
//定义并初始化静态数据成员,静态成员不能在类定义里边初始化，只能在class body外初始化。
CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket* gpClient = CClientSocket::getInstance();
CClientSocket::CNewAndDel CClientSocket::m_newdel;


CPacket::CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0)/*, hEvent(INVALID_HANDLE_VALUE)*/ {}

CPacket& CPacket::operator=(const CPacket& pack)
{
	if (this != &pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		//hEvent = pack.hEvent;
	}
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize/*, HANDLE hEvent*/)
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
	//this->hEvent = hEvent;
}

//解包
CPacket::CPacket(const BYTE* pData, size_t& nSize) :
	sHead(0), nLength(0), sCmd(0), sSum(0)/*, hEvent(INVALID_HANDLE_VALUE)*/
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
	//hEvent = pack.hEvent;
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


