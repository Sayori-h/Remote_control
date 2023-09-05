#pragma once
#include <WinSock2.h>
#include <memory>
enum class HTYPE{
	HTypeTCP=1,
	HTypeUDP
};

class HSockaddrIn {
public:
	HSockaddrIn() {
		memset(&m_addr, 0, sizeof(m_addr));
		m_port = -1;
	}
	HSockaddrIn(sockaddr_in addr) {

		memcpy(&m_addr, &addr, sizeof(addr));
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);

	}
	HSockaddrIn(UINT nIP, short nPort) {
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = htonl(nIP);
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = nPort;
	}
	HSockaddrIn(const std::string& strIP, short nPort) {
		m_ip = strIP;
		m_port = nPort;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	HSockaddrIn(const HSockaddrIn& addr) {
		memcpy(&m_addr, &addr.m_addr, sizeof m_addr);
		m_ip = addr.m_ip;
		m_port = addr.m_port;
	}
	HSockaddrIn& operator=(const HSockaddrIn& addr) {
		if (this != &addr) {
			memcpy(&m_addr, &addr.m_addr, sizeof m_addr);
			m_ip = addr.m_ip;
			m_port = addr.m_port;
		}
		return *this;
	}
	operator sockaddr* ()const {
		return (sockaddr*)&m_addr;

	}
	operator void* ()const {
		return (void*)&m_addr;
	}
	void Update() {
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	std::string GetIP() const {
		return m_ip;
	}
	short GetPort()const { return m_port; }
	inline int Size()const { return sizeof sockaddr_in; }
private:
	sockaddr_in m_addr;
	std::string m_ip;
	short m_port;
};

class HBuffer :public std::string {
public:
	HBuffer(const char*str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, size());
	}
	HBuffer(size_t size=0):std::string() {
		if (size > 0) {
			resize(size);
			memset(*this, 0, this->size());
		}
	}
	HBuffer(void* buffer,size_t size):std::string(){
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
		

	~HBuffer() {
		std::string::~basic_string();
	}
	operator char* () const{ return (char*)c_str(); }
	operator const char* ()const { return c_str(); }
	operator BYTE* ()const { return(BYTE*)c_str(); }
	operator void* ()const { return(void*)c_str(); }
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};

class HSocket
{
public:
	HSocket(::HTYPE nType=HTYPE::HTypeTCP,int nProtocol=0){
		m_socket = socket(PF_INET, (int)nType, nProtocol);
		m_type = nType;
		m_protocol = nProtocol;
	}

	HSocket(const HSocket& sock) {
		m_socket = socket(PF_INET, (int)sock.m_type, m_protocol);
		m_type = sock.m_type;
		m_protocol = sock.m_protocol;
		m_addr = sock.m_addr;
	}

	~HSocket(){
		close();
	}

	HSocket& operator=(const HSocket& sock) {
		if (this != &sock) {
			m_socket = socket(PF_INET, (int)sock.m_type, m_protocol);
			m_type = sock.m_type;
			m_protocol = sock.m_protocol;
			m_addr = sock.m_addr;
		}
		return *this;
	}

	operator SOCKET () const{
		return m_socket;
	}

	operator SOCKET (){
		return m_socket;
	}

	bool operator==(SOCKET sock) const{
		return m_socket == sock;
	}
	int listen(int backlog=5) {
		if (m_type != HTYPE::HTypeTCP) return -1;
		return ::listen(m_socket, backlog);
	}
	int bind(const std::string& ip, short port) {
		m_addr = HSockaddrIn(ip, port);
		return ::bind(m_socket, m_addr, m_addr.Size());

	}
	int accept(){}
	int connect(const std::string& ip, short port) {

	}
	int send(const HBuffer& buffer){
		return ::send(m_socket, buffer, buffer.size(), 0);

	}
	int recv(HBuffer& buffer){
		return ::recv(m_socket, buffer, buffer.size(), 0);
	}
	int sendto(HBuffer& buffer,const HSockaddrIn& to){
		return ::sendto(m_socket, buffer, buffer.size(), 0, to, to.Size());
	}
	int recvfrom(HBuffer& buffer,HSockaddrIn& from){
		int len = from.Size();
		int ret=::recvfrom(m_socket, buffer, buffer.size(), 0, from, &len);
		if (ret > 0) {
			from.Update();
		}
		return ret;
	}
	void close() {
		if (m_socket != INVALID_SOCKET) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		}
		
	}
private:
	SOCKET m_socket;
	HTYPE m_type;
	int m_protocol;
	HSockaddrIn m_addr;
};

typedef std::shared_ptr<HSocket> HSOCKET;

