#pragma once

#include <Promise.h>
#include <GenApi/GenApi.h>

#include "UdpPort.h"
#include "FrameData.h"

namespace gevdevice{

class CDevice : public GenApi::CPortImpl
{
public:
	typedef std::shared_ptr<CDevice> CDevicePtr;
	typedef std::function<void(const FrameData& frame)> TFrameCallBack;
	typedef std::function<void(const std::string& error)> TErrorCallBack;
	typedef TPromise<std::vector<UdpPort::Connection>>::PromisePtr UdpConnnectionPromisePtr;

	~CDevice();

	static bool InitSystem();
	static void FinishSystem();

	static UdpConnnectionPromisePtr FindAll(uint32_t waitTimeMs = 500);

	static CDevicePtr Create(const UdpPort::Connection& connection, const TFrameCallBack& frameCb, uint16_t packetSize = 4004, uint16_t apiPort = 12220, uint16_t imageStreamPort = 12221);
	static CDevicePtr Create(const UdpPort::Connection& connection, const TFrameCallBack& frameCb, const TErrorCallBack& onErrorCb, uint16_t packetSize = 4004, uint16_t apiPort = 12220, uint16_t imageStreamPort = 12221);

	void Connect(uint32_t cameraControlIntervalMs = 3000);
	void Disconnect();
	bool IsConnected() const;

	// reimplemented (IBase)
	virtual GenApi::EAccessMode GetAccessMode() const override;

	const std::string& GetApiXml() const;
	GenApi::CNodeMapRef GetGenApi() const;

	uint32_t GetLocalIp() const;
	uint32_t GetCameraIp() const;

	// reimplemented (IPort)
	virtual void Read(void *pBuffer, int64_t Address, int64_t Length) override;
	virtual void Write(const void *pBuffer, int64_t Address, int64_t Length) override;

private:
	typedef std::function<void(const std::vector<uint8_t>& message, uint16_t messageType, uint16_t status)> OnReceaveMessage;

	CDevice(const UdpPort::Connection& connection, const TFrameCallBack& frameCb, uint16_t packetSize, uint16_t apiPort, uint16_t imageStreamPort);

	void OnApiDataHandler(const std::vector<uint8_t>& data);
	void OnFrameStreamHandler(const std::vector<uint8_t>& data);

	bool ReadMemoryBlock(uint32_t startaddress, uint32_t n, const std::function<void(const std::vector<uint8_t>& data, uint16_t status)>& answer);
	bool ReadRegisterMemory(uint32_t address, const std::function<void(const std::vector<uint8_t>& data, uint16_t status)>& answer);
	bool WriteRegisterMemory(uint32_t address, const std::vector<uint8_t>& value, const std::function<void(const std::vector<uint8_t>& data, uint16_t status)>& answer);

	TPromise<std::vector<uint8_t>>::PromisePtr ReadRegisterMemory(uint32_t address);
	TPromise<bool>::PromisePtr WriteRegisterMemory(uint32_t address, const std::vector<uint8_t>& data);
	TPromise<bool>::PromisePtr SendControl();
	TPromise<bool>::PromisePtr SendHeartBeat(uint32_t cameraControlIntervalMs);
	TPromise<std::string>::PromisePtr GetGenICamApiXml();

	void OnError(const std::string& err);

	uint16_t m_packetSize;
	uint32_t m_localIp;
	uint32_t m_cameraIp;
	uint16_t m_apiPortNum;
	uint16_t m_imageStreamPortNum;
	std::shared_ptr<std::thread> m_cameraControlThreadPtr;
	std::atomic<bool> m_connected;

	UdpPort m_apiPort;
	UdpPort m_frameStreamPort;

	std::string m_apiXml;
	GenApi::CNodeMapRef m_genApi;

	std::mutex m_answerMessageMapMutex;
	std::map<int, OnReceaveMessage> m_answerMessageMap;
	std::condition_variable m_exitCondition;

	TFrameCallBack m_frameCb;
	std::shared_ptr<FrameData> m_currentFrameDataPtr;

	TErrorCallBack m_errorCb;

	static PromiseContext ms_promises;
};

}