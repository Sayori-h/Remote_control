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
    OVERLAPPED           m_overlapped;
    DWORD                m_operator;//操作 参见HuxlOperator
    std::vector<char>    m_buffer;//缓冲区
    ThreadWorker         m_worker;//处理函数
    HuxlServer*          m_server;//服务器对象
    HuxlClient*          m_client;//对应的客户端
    WSABUF               m_wsabuffer;
    virtual ~HuxlOverlapped() {
        m_buffer.clear();
    }
    HuxlOverlapped(){}
};

template<HuxlOperator>class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;
template<HuxlOperator>class RecvOverlapped;
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;
template<HuxlOperator>class SendOverlapped;
typedef SendOverlapped<ESend> SENDOVERLAPPED;


class HuxlClient:public ThreadFuncBase {
public:
    HuxlClient();
    //HuxlClient(const HuxlClient&  a);
    //HuxlClient& operator= (HuxlClient&) { return *this; }
    HuxlClient(const HuxlClient& a) {}
    HuxlClient& operator= (const HuxlClient&){ return *this; }
    ~HuxlClient() {
        m_buffer.clear();
        closesocket(m_sock);
        m_recv.reset();
        m_send.reset();
        m_overlapped.reset();
        m_vecSend.Clear();
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

    LPWSABUF RecvWSABuffer();
    LPWSAOVERLAPPED RecvOverlapped();
    LPWSABUF SendWSABuffer();
    LPWSAOVERLAPPED SendOverlapped();
    DWORD& flags() { return m_flags; }
    sockaddr_in* GetLocalAddr()  { return &m_laddr; }
    sockaddr_in* GetRemoteAddr() { return &m_raddr; }
    size_t GetBufferSize() const { return m_buffer.size(); }

    int Recv();
    //投递到队列里面去
    int Send(void* buffer, size_t nSize);
    int SendData(std::vector<char>& data);
private:
    SOCKET m_sock;
    DWORD m_received;
    DWORD m_flags;
    std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
    std::shared_ptr<RECVOVERLAPPED>   m_recv;
    std::shared_ptr<SENDOVERLAPPED>   m_send;
    std::vector<char> m_buffer;
    size_t          m_used;//已经使用的缓冲区大小
    sockaddr_in     m_laddr;
    sockaddr_in     m_raddr;
    bool            m_isbusy;
    HuxlSendQueue<std::vector<char>> m_vecSend;//发送数据队列
};

template<HuxlOperator>
class AcceptOverlapped :public HuxlOverlapped,ThreadFuncBase
{
public:
    AcceptOverlapped();
    int AcceptWorker();
    //PCLIENT m_client;
};


template<HuxlOperator>
class RecvOverlapped :public HuxlOverlapped, ThreadFuncBase
{
public:
    RecvOverlapped();
    int RecvWorker() {
        int ret = m_client->Recv();
        return ret;
    }
};


template<HuxlOperator>
class SendOverlapped :public HuxlOverlapped, ThreadFuncBase
{
public:
    SendOverlapped();
    int SendWorker() {
        
        return -1;
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
        return -1;
    }
};

typedef ErrorOverlapped<EError> ERROROVERLAPPED;


class HuxlServer :
    public ThreadFuncBase
{
public:
    HuxlServer(const std::string& ip = "0.0.0.0", short port = 9527) :m_pool(10) {
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_sock = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    ~HuxlServer();

    bool StartService();

    bool NewAccept();

    void BindNewSocket(SOCKET s);

private:
    void CreateSocket();
    
    int threadIocp();
public:
    std::map<SOCKET, std::shared_ptr<HuxlClient>> m_client;  //服务器接受到的客户端 
private:
    HuxlThreadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sock;
    //map：多客户端，需要维持客户端的状态，客户端唯一的是socket套接字，每个套接字会维护一个对象
    //每个客户端上来都追加到map里面去，想要的客户端信息都保存在*HuxlClient里
    sockaddr_in m_addr;
    CEdoyunQueue<HuxlClient> m_lstClient;
};




