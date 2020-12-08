#include "UdpPort.h"

#include <iostream>

#include <list>
#include <cassert>
#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ifaddrs.h>

namespace gevdevice{

class UdpPortImpl
{
public:
	UdpPortImpl() : m_started(false)
	{}

	int sockfd;
	sockaddr_in remote;

	std::shared_ptr<std::thread> m_threadPtr;
	std::shared_ptr<std::thread> m_queueThreadPtr;
	std::atomic<bool> m_started;
	std::atomic<bool> m_queueStarted;
	std::vector<UdpPort::ReceaveHandler> m_heandlerList;
	
	std::mutex m_bufferMutex;
	
	struct msg
	{
		msg() {}
		
		msg(const sockaddr_in& from, const uint8_t* data, int length)
		: from(from), data(data, data + length)
		{
		}
		
		sockaddr_in from;
		std::vector<uint8_t> data;
	};

	std::list<msg> m_buffer;
};

bool UdpPort::InitSockets()
{
	return true;
}


void UdpPort::FinishSockets()
{
}

UdpPort::UdpPort()
{
	m_implPtr.reset(new UdpPortImpl());
}

UdpPort::~UdpPort()
{
	Stop();
}

std::vector<uint32_t> UdpPort::GetLocalAddressList()
{
	std::vector<uint32_t> result;

	struct ifaddrs *addrs,*tmp;
	struct sockaddr_in *sockaddr_ipv4;

	if (getifaddrs(&addrs) == -1){
		return result;
	}
	
	tmp = addrs;

	while (tmp) {
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
			sockaddr_ipv4 = (struct sockaddr_in *) tmp->ifa_addr;
			result.push_back(sockaddr_ipv4->sin_addr.s_addr);
		}
			

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);

	return result;
}

void UdpPort::AddHandler(const ReceaveHandler & handler)
{
	assert(!m_implPtr->m_started);
	m_implPtr->m_heandlerList.push_back(handler);
}

bool UdpPort::Start(uint32_t localip, uint16_t localport, uint32_t remoteip, uint16_t remoteport, const int sndbuf, const int rcvbuf)
{
	if (m_implPtr->m_started){
		return false;
	}

	m_implPtr->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_implPtr->sockfd == -1) {
		return false;
	}
	
	int yes = 1;
	if (setsockopt(m_implPtr->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		return false;
	}

	if (remoteip == 0xFFFFFFFF) {
		int broadcast = 1;
		if (setsockopt(m_implPtr->sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1){
			return false;
		}
	
	}

	if (sndbuf != 0) {
		setsockopt(m_implPtr->sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(int));
	}

	if (rcvbuf != 0) {
		setsockopt(m_implPtr->sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(int));
	}

	listen(m_implPtr->sockfd, 5);

	sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = localip;
	local.sin_port = htons(localport);

	if (bind(m_implPtr->sockfd, (sockaddr*)&local, sizeof(local)) == -1) {
		close(m_implPtr->sockfd);
		return false;
	}
	
	memset(&m_implPtr->remote, 0, sizeof(m_implPtr->remote));
	m_implPtr->remote.sin_family = AF_INET;
	m_implPtr->remote.sin_port = htons(remoteport);
	m_implPtr->remote.sin_addr.s_addr = remoteip;

	std::condition_variable cv;

	m_implPtr->m_threadPtr.reset(new std::thread([this, &cv](){
		m_implPtr->m_started = true;
		cv.notify_one();

		const int tn = 1024 * 16;
		uint8_t t[tn];

		sockaddr_in from;
		socklen_t fromlen = sizeof(sockaddr_in);
			
		while (m_implPtr->m_started){
			fd_set setread;
			FD_ZERO(&setread);
			FD_SET(m_implPtr->sockfd, &setread);

			timeval timeout_directly = { 1, 0 };

			int sel = select(m_implPtr->sockfd + 1, &setread, NULL, NULL, &timeout_directly);
			
			if (sel < 0){
				break;
			}
			else if (sel == 0){
				continue;
			}

			int recvd = recvfrom(m_implPtr->sockfd, t, tn, MSG_WAITALL, (sockaddr*)&from, &fromlen);
			if (recvd > 0){
				std::unique_lock<std::mutex> lock(m_implPtr->m_bufferMutex);
				m_implPtr->m_buffer.emplace(m_implPtr->m_buffer.end(), from, t, recvd);
			}
		}
	}));

	m_implPtr->m_queueThreadPtr.reset(new std::thread([this, &cv](){
		m_implPtr->m_queueStarted = true;
		cv.notify_one();

		while (m_implPtr->m_queueStarted){
			std::list<UdpPortImpl::msg> msgs;
			{
				std::unique_lock<std::mutex> lock(m_implPtr->m_bufferMutex);
				std::swap(msgs, m_implPtr->m_buffer);
			}

			if (msgs.empty()){
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;			
			}
			else {
				for (const auto& m : msgs) {
					for (const auto& handler : m_implPtr->m_heandlerList) {
						handler(m.data, m.from.sin_addr.s_addr, m.from.sin_port);
					}
				}
			}
		}
	}));

	std::mutex m;
	std::unique_lock<std::mutex> lk(m);
	cv.wait(lk, [this] {
		return IsStarted();
	});

	return true;
}

void UdpPort::Stop()
{
	if (!IsStarted()){
		return;
	}

	m_implPtr->m_started = false;
	m_implPtr->m_queueStarted = false;

	if (m_implPtr->m_threadPtr){
		m_implPtr->m_threadPtr->join();
	}

	close(m_implPtr->sockfd);
	
	if (m_implPtr->m_queueThreadPtr){
		m_implPtr->m_queueThreadPtr->join();
	}

	m_implPtr->m_heandlerList.clear();
}

bool UdpPort::IsStarted()
{
	return m_implPtr->m_started && m_implPtr->m_queueStarted;
}

bool UdpPort::Send(const std::vector<uint8_t>& data) const
{
	return sendto(m_implPtr->sockfd, data.data(), int(data.size()), 0, (sockaddr*)&m_implPtr->remote, sizeof(sockaddr_in)) != 0;
}

uint16_t UdpPort::Htons(uint16_t val)
{
	return htons(val);
}

uint32_t UdpPort::Htonl(uint32_t val)
{
	return htonl(val);
}

uint16_t UdpPort::Ntohs(uint16_t val)
{
	return ntohs(val);
}

uint32_t UdpPort::Ntohl(uint32_t val)
{
	return ntohl(val);
}

uint64_t UdpPort::MacAddress(uint32_t ip)
{
	//TODO: to implement
	return 0;
}

}
