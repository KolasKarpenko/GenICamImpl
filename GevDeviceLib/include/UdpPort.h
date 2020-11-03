#pragma once


// STL includes
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <sstream>


namespace gevdevice
{


class UdpPortImpl;

// Uses only ip v4 // todo
class UdpPort
{
public:
	typedef std::function<void(const std::vector<uint8_t>& data, uint32_t remoteip, uint16_t remoteport)> ReceaveHandler;

	struct Connection
	{
		Connection() : localAddr(0), cameraAddr(0){}

		uint32_t localAddr;
		uint32_t cameraAddr;
	};

	UdpPort();
	~UdpPort();

	static bool InitSockets();
	static void FinishSockets();

	static std::vector<uint32_t> GetLocalAddressList();

	void AddHandler(const ReceaveHandler& handler);

	bool Start(uint32_t localip, uint16_t localport, uint32_t remoteip, uint16_t remoteport, const int sndbuf = 0, const int rcvbuf = 0);
	void Stop();

	bool IsStarted();

	bool Send(const std::vector<uint8_t>& data) const;

	static uint16_t Htons(uint16_t val);
	static uint32_t Htonl(uint32_t val);

	static uint16_t Ntohs(uint16_t val);
	static uint32_t Ntohl(uint32_t val);

	static uint32_t CreateIpAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
	{
		uint8_t addrBytes[4];
		addrBytes[0] = a;
		addrBytes[1] = b;
		addrBytes[2] = c;
		addrBytes[3] = d;
		return *(uint32_t *)addrBytes;
	}

	static uint32_t CreateIpAddress(const std::string& address)
	{
		uint32_t ipbytes[4];
		sscanf(address.c_str(), "%d.%d.%d.%d", &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0]);
		return CreateIpAddress(uint8_t(ipbytes[3]), uint8_t(ipbytes[2]), uint8_t(ipbytes[1]), uint8_t(ipbytes[0]));
	}

	static std::string IpAddressToString(uint32_t addr)
	{
		const uint8_t* addrBytes = (const uint8_t*)(&addr);
		uint8_t a = addrBytes[0];
		uint8_t b = addrBytes[1];
		uint8_t c = addrBytes[2];
		uint8_t d = addrBytes[3];

		std::stringstream ss;
		ss << (int)a << "." << (int)b << "." << (int)c << "." << (int)d;
		return ss.str();
	}

private:
	std::shared_ptr<UdpPortImpl> m_implPtr;
};

typedef std::shared_ptr<UdpPort> UdpPortPtr;


}


