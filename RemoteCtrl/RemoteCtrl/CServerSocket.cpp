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
		MessageBox(NULL, _T("��ʼ��WinSock��ʧ�ܣ�"), _T("����"), MB_OK | MB_ICONERROR);
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
	//1���ȵĿɿ���2�Խӵķ�����3���������������籩¶����
	// TODO: socket��bind�� listen��accept�� read��write�� close
	// �������ܣ��ú����Ƿ���˺�����ں��������ڶ���ʹ�õĽӿ�
	//����1��Command�ľ�̬������
	//����2��ִ������Ķ��󣬼�Command����
	//����3���˿ںţ�Ĭ��9527
	//����ֵ��ʧ�ܷ���-1��
	//	���������¼�����Ӧ������
	//		�ٳ�ʼ������⣬������InitSocket()
	//		�����ӿͻ��ˣ�������AcceptClient()
	//		�۴���ͻ��˷��������ݣ��������������DealCommand()
	//		�ܽ������Command�������������������
	//		�ݴӳ�ԱlstPackets����ȡ�ô�Command����������(���ܺ���push_back�����ݣ��ο�Command��Ĺ��ܺ���)��Ȼ�����Send���ͻؿͻ���
	//�׽��ֳ�ʼ��
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
	/*����˵�������ܷ����ݣ������ȡ�ð�����������
����ֵ�� �ɹ��õ����� ��  ʧ�ܵõ� -1
*/
	if (m_client == -1)return false;
	char* buffer = new char[BUF_SIZE];
	if (buffer == NULL)
	{
		TRACE("�ڴ治��!");
		return -2;
	}
	memset(buffer, 0, BUF_SIZE);
	static size_t index = 0;
	while (true)
	{
		size_t len = recv(m_client, buffer + index, BUF_SIZE - index, 0);//ʵ�ʽ��յ��ĳ���
		if ((int)len <= 0)
		{
			delete[]buffer;
			return -1;
		}
		//TRACE("recv %d\r\n", len);
		index += len;
		len = index;
		m_packet = CPacket((BYTE*)buffer, len);//���
		if (len > 0)
		{
			memmove(buffer, buffer + len, BUF_SIZE - len);
			index -= len;
			delete[]buffer;
			return m_packet.sCmd;//���ճɹ�����������
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
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));//strData����װ�κ�����
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
//CServerSocket g_server;���뵥��֮ǰԭʼ����

//������ʹ�����˽�о�ָ̬�����ָ�����Ψһʵ��������һ�����о�̬������ȡ��ʵ��
//static��Ա�����������౾��Ͷ��󣬵����ж������ӵ��һ���ľ�̬��Ա���Ӷ��ڶ�������ǲ���ͨ�����캯��������г�ʼ����
//���岢��ʼ����̬���ݳ�Ա,��̬��Ա�������ඨ����߳�ʼ����ֻ����class body���ʼ����
CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket* gpServer = CServerSocket::getInstance();
CServerSocket::CNewAndDel CServerSocket::m_newdel;

