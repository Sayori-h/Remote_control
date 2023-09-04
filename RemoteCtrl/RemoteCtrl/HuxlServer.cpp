#include "pch.h"
#include "HuxlServer.h"
#include "CHuxlTool.h"
#pragma warning(disable:4407)

template<HuxlOperator op>
AcceptOverlapped<op>::AcceptOverlapped() 
{
	m_worker = ThreadWorker(this, (FUNCTYPE)&AcceptOverlapped<op>::AcceptWorker);
	m_operator = EAccept;
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_server = NULL;
}

template<HuxlOperator op>
int AcceptOverlapped<op>::AcceptWorker() {
    //WARN
    //解析连接过来的客户端
	INT local_length = 0, remote_length = 0;
    //WARN
    if (m_client->GetBufferSize() > 0) {
    
        //m_client = m_server->m_client.at(0).get();
        sockaddr* plocal = NULL, * premote = NULL;
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16,
			sizeof(sockaddr_in) + 16,
			(sockaddr**)&plocal, &local_length,//本地地址
			(sockaddr**)&premote, &remote_length//远程地址
		);
		//PCLIENT pClient(new HuxlClient());//所有的人拿到的都是引用，一个shared_ptr
		//m_server->m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
        memcpy(m_client->GetLocalAddr(), plocal, sizeof(sockaddr_in));
        memcpy(m_client->GetRemoteAddr(), premote, sizeof(sockaddr_in));
        m_server->BindNewSocket(*m_client);
		int ret = WSARecv((SOCKET)*m_client, m_client->RecvWSABuffer(),
			1, *m_client, &m_client->flags(), m_client->RecvOverlapped(), NULL);
		if (ret == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING)) {
			//TODO:报错
            TRACE("ret=%d error=%d\r\n", ret, WSAGetLastError());
		}
		if (!m_server->NewAccept()) return -2;
	}
	return -1;
}

template<HuxlOperator op>SendOverlapped<op>::SendOverlapped()
{
    m_operator = op;
    m_worker = ThreadWorker(this, (FUNCTYPE)&SendOverlapped<op>::SendWorker);
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}

template<HuxlOperator op>RecvOverlapped<op>::RecvOverlapped()
{
    m_operator = op;
    m_worker = ThreadWorker(this, (FUNCTYPE)&RecvOverlapped<op>::RecvWorker);
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}

HuxlClient::HuxlClient()
    :m_isbusy(false), m_flags(0),
    m_overlapped(new ACCEPTOVERLAPPED()),
    m_recv(new RECVOVERLAPPED()),
    m_send(new SENDOVERLAPPED()),
    m_vecSend(this, (SENDCALLBACK)(&HuxlClient::SendData))
{
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
	memset(&m_laddr, 0, sizeof(m_laddr));
	memset(&m_raddr, 0, sizeof(m_raddr));
}

//HuxlClient::HuxlClient(const HuxlClient& a)
//{
//}

void HuxlClient::SetOverlapped(PCLIENT& ptr) {
	m_overlapped->m_client = ptr.get();
    m_recv->m_client = ptr.get();
    m_send->m_client = ptr.get();
}

HuxlClient::operator LPOVERLAPPED() {
	return &m_overlapped->m_overlapped;
}

LPWSABUF HuxlClient::RecvWSABuffer()
{
    return &m_recv->m_wsabuffer;
}

LPWSAOVERLAPPED HuxlClient::RecvOverlapped()
{
    return &m_recv->m_overlapped;
}

LPWSABUF HuxlClient::SendWSABuffer()
{
    return &m_send->m_wsabuffer;
}

LPWSAOVERLAPPED HuxlClient::SendOverlapped()
{
    return &m_send->m_overlapped;
}

int HuxlClient::Recv()
{
    int ret = recv(m_sock, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
    if (ret <= 0)return -1;
    m_used += (size_t)ret;
    //TODO:解析数据
    CHuxlTool::dump((BYTE*)m_buffer.data(), ret);
    return 0;
}

int HuxlClient::Send(void* buffer, size_t nSize)
{
    std::vector<char> data(nSize);
    memcpy(data.data(), buffer, nSize);
    if (m_vecSend.PushBack(data))return 0;
    return -1;
}

int HuxlClient::SendData(std::vector<char>& data)
{
    if (m_vecSend.Size()) {
        int ret = WSASend(m_sock, SendWSABuffer(), 1, &m_received, m_flags, &m_send->m_overlapped, NULL);
        if (ret != 0 && (WSAGetLastError() != WSA_IO_PENDING)) {
        CHuxlTool::ShowError();
            return -1;
        }
    }
    return 0;
}

HuxlServer::~HuxlServer() {
   closesocket(m_sock);
   auto it=m_client.begin();
   for (; it != m_client.end(); it++) {
       it->second.reset();
   }
   m_client.clear();
   CloseHandle(m_hIOCP);
   m_pool.Stop();
   WSACleanup();
}

bool HuxlServer::StartService()
{
    CreateSocket();
    if (bind(m_sock, (sockaddr*)&m_addr, sizeof(m_addr)) == -1) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
    }
    if (listen(m_sock, 3) == -1) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        return false;
    }
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
    if (m_hIOCP == NULL) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        m_hIOCP = INVALID_HANDLE_VALUE;
        return false;
    }
    CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
    m_pool.Invoke();
    m_pool.DispatchWoker(ThreadWorker(this, (FUNCTYPE)&HuxlServer::threadIocp));
    if (!NewAccept())return false;
    //m_pool.DispatchWoker(ThreadWorker(this, (FUNCTYPE)&HuxlServer::threadIocp));
    //m_pool.DispatchWoker(ThreadWorker(this, (FUNCTYPE)&HuxlServer::threadIocp));
    return true;
}

bool HuxlServer::NewAccept()
{
    PCLIENT pClient(new HuxlClient());//所有的人拿到的都是引用，一个shared_ptr
    pClient->SetOverlapped(pClient);
    m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
    if (!AcceptEx(m_sock, *pClient, *pClient, 0,
        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient)) {
        TRACE("%d\r\n", WSAGetLastError());
        if (WSAGetLastError() != WSA_IO_PENDING) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
    }
    return true;
}

void HuxlServer::BindNewSocket(SOCKET s)
{
    CreateIoCompletionPort((HANDLE)s, m_hIOCP, (ULONG_PTR)this, 0);
    

}

void HuxlServer::CreateSocket()
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    int opt = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

int HuxlServer::threadIocp()
{
    DWORD tranferred = 0;
    ULONG_PTR CompletionKey = 0;
    OVERLAPPED* lpOverlapped = NULL;
    //WARN if
    while(GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITE)) {
        if (CompletionKey) {
            HuxlOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, HuxlOverlapped, m_overlapped);
            TRACE("pOverlapped->m_operator %d\r\n", pOverlapped->m_operator);
            pOverlapped->m_server = this;
            switch (pOverlapped->m_operator) {
            case EAccept:
            {
                ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)lpOverlapped;
                //WARN
                /*m_pool.DispatchWoker(pOver->m_worker);*/
                m_pool.DispatchWoker(ThreadWorker(pOver, (FUNCTYPE)&ACCEPTOVERLAPPED::AcceptWorker));
            }
            break;
            case ERecv:
            {
                RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)lpOverlapped;
                m_pool.DispatchWoker(pOver->m_worker);
            }
            break;
            case ESend:
            {
                SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)lpOverlapped;
                m_pool.DispatchWoker(pOver->m_worker);
            }
            break;
            case EError:
            {
                ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)lpOverlapped;
                m_pool.DispatchWoker(pOver->m_worker);
            }
            break;
            }
        }
        else return -1;
    }
    return 0;
}
