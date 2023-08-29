#pragma once
#include "HuxlThread.h"
#include "CEdoyunQueue.h"
#include <MSWSock.h>
#include <map>


enum HuxlOperator{
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};

class HuxlServer;
class HuxlClient;
typedef std::shared_ptr<HuxlClient> PCLIENT;

class HuxlOverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;//���� �μ�HuxlOperator
    std::vector<char> m_buffer;//������
    ThreadWorker m_worker;//������
    HuxlServer* m_server;//����������

};

template<HuxlOperator>class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

class HuxlClient {
public:
    HuxlClient();

    ~HuxlClient() {
        closesocket(m_sock);
    }

    void SetOverlapped(PCLIENT& ptr);
    operator SOCKET() {
        return m_sock;
    }

    operator PVOID() {
        return &m_buffer[0];
    }

    operator LPOVERLAPPED();

    operator LPDWORD() {
        return &m_received;
    }

    sockaddr_in* GetLocalAddr()  { return &m_laddr; }
    sockaddr_in* GetRemoteAddr() { return &m_raddr; }
private:
    SOCKET m_sock;
    DWORD m_received;
    std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
    std::vector<char> m_buffer;
    sockaddr_in m_laddr;
    sockaddr_in m_raddr;
    bool m_isbusy;
};

template<HuxlOperator>
class AcceptOverlapped :public HuxlOverlapped,ThreadFuncBase
{
public:
    AcceptOverlapped();
    int AcceptWorker();
    PCLIENT m_client;
};


template<HuxlOperator>
class RecvOverlapped :public HuxlOverlapped, ThreadFuncBase
{
public:
    RecvOverlapped() :m_operator(ERecv), m_worker(this, &RecvOverlapped::RecvWorker) {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024*256);
    }
    int RecvWorker() {
        //TODO:
    }
};



template<HuxlOperator>
class SendOverlapped :public HuxlOverlapped, ThreadFuncBase
{
public:
    SendOverlapped() :m_operator(ESend), m_worker(this, &SendOverlapped::SendWorker) {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024*256);
    }
    int SendWorker() {
        //TODO:
    }
};


template<HuxlOperator>
class ErrorOverlapped :public HuxlOverlapped, ThreadFuncBase
{
public:
    ErrorOverlapped() :m_operator(EError), m_worker(this, &ErrorOverlapped::ErrorWorker) {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024);
    }
    int ErrorWorker() {
        //TODO:
    }
};

typedef RecvOverlapped<ERecv> RECVOVERLAPPED;
typedef SendOverlapped<ESend> SENDOVERLAPPED;
typedef ErrorOverlapped<EError> ERROROVERLAPPED;




class HuxlServer :
    public ThreadFuncBase
{
public:
    HuxlServer(const std::string& ip="0.0.0.0",short port=9527):m_pool(10){
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_sock = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    ~HuxlServer(){}

    bool StartService() {
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

    bool NewAccept() {
        PCLIENT pClient(new HuxlClient());//���е����õ��Ķ������ã�һ��shared_ptr
        m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
        if (!AcceptEx(m_sock, *pClient, *pClient, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient)) {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }

    int AcceptClient() {
    
    };
private:
    void CreateSocket() {
        m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        int opt = 1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    }
    
    int threadIocp() {
        DWORD tranferred = 0;
        ULONG_PTR CompletionKey = 0;
        OVERLAPPED* lpOverlapped = NULL;
        if (GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITE)) {
            if (tranferred > 0 && (CompletionKey != 0)) {
                HuxlOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, HuxlOverlapped, m_overlapped);
                switch (pOverlapped->m_operator) {
                case EAccept:
                {
                    ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)lpOverlapped;
                    m_pool.DispatchWoker(pOver->m_worker);
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
public:
    std::map<SOCKET, std::shared_ptr<HuxlClient>> m_client;
private:
    HuxlThreadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sock;
    //map����ͻ��ˣ���Ҫά�ֿͻ��˵�״̬���ͻ���Ψһ����socket�׽��֣�ÿ���׽��ֻ�ά��һ������
    //ÿ���ͻ���������׷�ӵ�map����ȥ����Ҫ�Ŀͻ�����Ϣ��������*HuxlClient��
    sockaddr_in m_addr;
    CEdoyunQueue<HuxlClient> m_lstClient;
};



