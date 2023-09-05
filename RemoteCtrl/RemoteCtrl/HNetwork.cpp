#include "pch.h"
#include "HNetwork.h"

HServer::HServer(HServerParameter& param):m_stop(false),m_args(NULL)
{
	m_params = param;
	m_thread.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&HServer::threadFunc));
}

HServer::~HServer()
{
	Stop();
}

int HServer::Invoke(void* arg)
{
	m_sock.reset(new HSocket(m_params.m_type));
	//SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
	while (*m_sock == INVALID_SOCKET) {
		printf("%s(%d):%s ERROR(%d)!!\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError());
		return -1;
	}
	if (m_params.m_type == HTYPE::HTypeTCP) {
		if (m_sock->listen() == -1) {
			return -2;
		}
	}
	HSockaddrIn client;
	if (-1 == m_sock->bind(m_params.m_ip,m_params.m_port)) {
		printf("%s(%d):%s ERROR(%d)!!\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError());
		return -3;
	}
	if (m_thread.Start() == false)return -4;
	m_args = arg;
	return 0;
}

int HServer::Send(HSOCKET& client, const HBuffer& buffer)
{
	int ret=m_sock->send(buffer);//TODO:待优化，发送虽然成功，但是不完整！！
	if (m_params.m_send)m_params.m_send(m_args,client,ret);
	return ret;
}

int HServer::Sendto(HSockaddrIn& addr, HBuffer& buffer)
{
	int ret=m_sock->sendto(buffer, addr);//TODO:待优化，发送虽然成功，但是不完整！！
	if (m_params.m_sendto)m_params.m_sendto(m_args,addr,ret);
	return ret;
}

int HServer::Stop()
{
	if (m_stop == false) {
		m_sock->close();
		m_stop = true;
		m_thread.Stop();
	}
	return 0;
}

int HServer::threadFunc()
{
	if (m_params.m_type == HTYPE::HTypeTCP) {
		return threadTCPFunc();
	}
	else {
		return threadUDPFunc();
	}
	
	return 0;
}

int HServer::threadTCPFunc()
{
	return 0;
}

int HServer::threadUDPFunc()
{
	HBuffer buf(1024 * 256);
	HSockaddrIn client;
	int ret = 0;
	while (!m_stop) {
		ret = m_sock->recvfrom(buf, client);
		if (ret > 0) {
			client.Update();
			if (m_params.m_recvfrom != NULL) {
				m_params.m_recvfrom(m_args, buf, client);
			}
		}
		else {
			printf("%s(%d):%s ERROR(%d)!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			break;
		}
	}
	if (m_stop == false)m_stop = true;
	m_sock->close();
	printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

HServerParameter::HServerParameter(const std::string& ip, short port,
	HTYPE type, AcceptFunc acceptf, RecvFunc recvf, SendFunc sendf,
	RecvFromFunc recvfromf, SendToFunc sendtof)
{
	m_ip = ip;
	m_port = port;
	m_type = type;
	m_accept = acceptf;
	m_recv = recvf;
	m_send = sendf;
	m_recvfrom=recvfromf;
	m_sendto = sendtof;
}

HServerParameter& HServerParameter::operator<<(AcceptFunc func)
{
	m_accept = func;
	return *this;
}

HServerParameter& HServerParameter::operator<<(RecvFunc func)
{
	m_recv = func;
	return *this;

}

HServerParameter& HServerParameter::operator<<(SendFunc func)
{
	m_send = func;
	return *this;

}

HServerParameter& HServerParameter::operator<<(RecvFromFunc func)
{
	m_recvfrom = func;
	return *this;

}

HServerParameter& HServerParameter::operator<<(SendToFunc func)
{
	m_sendto = func;
	return *this;

}

HServerParameter& HServerParameter::operator<<(const std::string& ip)
{
	m_ip = ip;
	return *this;

}

HServerParameter& HServerParameter::operator<<(short port)
{
	m_port = port;
	return *this;

}

HServerParameter& HServerParameter::operator<<(HTYPE type)
{
	m_type = type;
	return *this;

}

HServerParameter& HServerParameter::operator>>(AcceptFunc& func)
{
	func = m_accept;
	return *this;

}

HServerParameter& HServerParameter::operator>>(RecvFunc& func)
{
	func = m_recv;
	return *this;
}

HServerParameter& HServerParameter::operator>>(SendFunc& func)
{
	func = m_send;
	return *this;
}

HServerParameter& HServerParameter::operator>>(RecvFromFunc& func)
{
	func = m_recvfrom;
	return *this;
}

HServerParameter& HServerParameter::operator>>(SendToFunc& func)
{
	func = m_sendto;
	return *this;
}

HServerParameter& HServerParameter::operator>>(std::string& ip)
{
	ip = m_ip;
	return *this;
}

HServerParameter& HServerParameter::operator>>(short& port)
{
	port = m_port;
	return *this;
}

HServerParameter& HServerParameter::operator>>(HTYPE& type)
{
	type = m_type;
	return *this;
}

HServerParameter::HServerParameter(const HServerParameter& param)
{
	m_ip = param.m_ip;
	m_port = param.m_port;
	m_type = param.m_type;
	m_accept = param.m_accept;
	m_recv = param.m_recv;
	m_send = param.m_send;
	m_recvfrom = param.m_recvfrom;
	m_sendto = param.m_sendto;
}

HServerParameter& HServerParameter::operator=(const HServerParameter& param)
{
	if (this != &param) {
		m_ip = param.m_ip;
		m_port = param.m_port;
		m_type = param.m_type;
		m_accept = param.m_accept;
		m_recv = param.m_recv;
		m_send = param.m_send;
		m_recvfrom = param.m_recvfrom;
		m_sendto = param.m_sendto;
	}
	return *this;
}
