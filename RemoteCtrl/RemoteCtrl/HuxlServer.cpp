#include "pch.h"
#include "HuxlServer.h"
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
	INT local_length = 0, remote_length = 0;
	if (*(LPDWORD)*m_client.get() > 0) {
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16,
			sizeof(sockaddr_in) + 16,
			(sockaddr**)m_client->GetLocalAddr(), &local_length,//本地地址
			(sockaddr**)m_client->GetRemoteAddr(), &remote_length//远程地址
		);
		PCLIENT pClient(new HuxlClient());//所有的人拿到的都是引用，一个shared_ptr
		m_server->m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
		if (!m_server->NewAccept()) return -2;
	}
	return -1;
}

HuxlClient::HuxlClient() :m_isbusy(false), m_overlapped(new ACCEPTOVERLAPPED()) {
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_buffer.resize(1024);
	memset(&m_laddr, 0, sizeof(m_laddr));
	memset(&m_raddr, 0, sizeof(m_raddr));
}

void HuxlClient::SetOverlapped(PCLIENT& ptr) {
	m_overlapped->m_client = ptr;
}

HuxlClient::operator LPOVERLAPPED() {
	return &m_overlapped->m_overlapped;
}
