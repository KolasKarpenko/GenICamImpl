#include "CDevice.h"

#include "ZipExtract.h"

namespace gevdevice
{

static const uint32_t MGevHeartbeatTimeoutAddr = 0x0938;
static const uint32_t MGevAddressControl = 0x00000a00;
static const uint32_t MGevAddressDataStreamPacketSize = 0x00000d04;
static const uint32_t MGevAddressDataStreamTargetIP = 0x00000d18;
static const uint32_t MGevAddressDataStreamTargetPort = 0x00000d00;
static const uint32_t MGevAddressGenICamZipFileInfoAddressFirst = 0x00000200;
static const uint32_t MGevAddressGenICamZipFileInfoAddressSecond = 0x00000400;

static const uint16_t MGevFlagAscBit = 0x01;
static const uint16_t MGevMsgTypeDeviceDetect = 0x02;
static const uint16_t MGevMsgTypeReadConfigMemory = 0x80;
static const uint16_t MGevMsgTypeWriteConfigMemory = 0x82;
static const uint16_t MGevMsgTypeReadConfigMemoryBlock = 0x84;

static const uint32_t MGevValueExclusiveAccess = 0x00000001;
static const uint32_t MGevValueStopGrabValue = 0x01000000;

static const uint32_t RecvBlockMax = 528;
static const uint32_t RecvBlockHeader = 12;

static const uint16_t GevPort = 3956;
static const uint32_t BroadcastAddr = 0xFFFFFFFF;
static const uint16_t BroadcastPort = 12220;

static const uint16_t MGevStatusSuccess = 0x0000;
static const uint16_t MGevStatusNotInpemented = 0x8001;
static const uint16_t MGevSatusInvalidParameter = 0x8002;
static const uint16_t MGevStatusInvalidAddress = 0x8003;
static const uint16_t MGevStatusWriteProtect = 0x8004;
static const uint16_t MGevStatusBabAlingment = 0x8005;
static const uint16_t MGevStatusAccessDenided = 0x8006;
static const uint16_t MGevStatusBusy = 0x8007;
static const uint16_t MGevStatusLocalProblem = 0x8008;
static const uint16_t MGevStatusMessageMismatch = 0x8009;
static const uint16_t MGevStatusInvalidProtocol = 0x800A;
static const uint16_t MGevStatusNoMessage = 0x800B;
static const uint16_t MGevStatusPacketUnavailable = 0x800C;
static const uint16_t MGevStatusDataOverRun = 0x800D;
static const uint16_t MGevStatusInvalidHeader = 0x800E;
static const uint16_t MGevStatusError = 0xFFFF;

static std::string StringStatus(uint16_t status)
{
	switch (status) {
	case MGevStatusSuccess:
		return "MGevStatusSuccess";
	case MGevStatusNotInpemented:
		return "MGevStatusNotInpemented";
	case MGevSatusInvalidParameter:
		return "MGevSatusInvalidParameter";
	case MGevStatusInvalidAddress:
		return "MGevStatusInvalidAddress";
	case MGevStatusWriteProtect:
		return "MGevStatusWriteProtect";
	case MGevStatusBabAlingment:
		return "MGevStatusBabAlingment";
	case MGevStatusAccessDenided:
		return "MGevStatusAccessDenided";
	case MGevStatusBusy:
		return "MGevStatusBusy";
	case MGevStatusLocalProblem:
		return "MGevStatusLocalProblem";
	case MGevStatusMessageMismatch:
		return "MGevStatusMessageMismatch";
	case MGevStatusInvalidProtocol:
		return "MGevStatusInvalidProtocol";
	case MGevStatusNoMessage:
		return "MGevStatusNoMessage";
	case MGevStatusPacketUnavailable:
		return "MGevStatusPacketUnavailable";
	case MGevStatusDataOverRun:
		return "MGevStatusDataOverRun";
	case MGevStatusInvalidHeader:
		return "MGevStatusInvalidHeader";
	case MGevStatusError:
		return "MGevStatusError";
	default:
		break;
	}

	return "MGevStatusUnknown";
}

static int SendGevMessage(uint16_t msgtype, const std::vector<uint8_t>& message, const UdpPort& port)
{
	static std::atomic<uint16_t> messageid(0);

	uint8_t t[8];
	uint16_t * s = (uint16_t*)t;
	t[0] = 0x42;
	t[1] = 0x01;
	s[1] = UdpPort::Htons(msgtype);
	s[2] = UdpPort::Htons((uint16_t)message.size());
	s[3] = UdpPort::Htons(messageid);

	std::vector<uint8_t> data(8 + message.size(), 0);
	memcpy(data.data(), t, 8);
	memcpy(data.data() + 8, message.data(), message.size());
	
	if (port.Send(data)){
		int curr = messageid;
		messageid++;
		return curr;
	}

	return -1;
}

enum MGevPacketFormat
{
	Leader = 1,
	Trailer = 2,
	Payload = 3
};


PromiseContext CDevice::ms_promises;

CDevice::~CDevice()
{
	Disconnect();
}

bool CDevice::InitSystem()
{
	static bool isInit = false;

	if (!isInit){
		isInit = true;
		return UdpPort::InitSockets();
	}

	return true;
}

void CDevice::FinishSystem()
{
	ms_promises.Join();
	UdpPort::FinishSockets();
}

CDevice::CDevice(const UdpPort::Connection& connection, int apiTimeoutMs, const TFrameCallBack& frameCb, uint16_t packetSize, uint16_t apiPort, uint16_t imageStreamPort) :
	m_packetSize(packetSize),
	m_frameCb(frameCb),
	m_localIp(connection.localAddr),
	m_cameraIp(connection.cameraAddr),
	m_imageStreamPortNum(imageStreamPort),
	m_apiPortNum(apiPort),
	m_connected(false),
	m_apiTimeout(apiTimeoutMs),
	m_currentFrameDataPtr(nullptr)
{
}

void CDevice::OnApiDataHandler(const std::vector<uint8_t>& data)
{
	const uint16_t* s = (const uint16_t*)data.data();

	uint16_t msgstatus = UdpPort::Ntohs(s[0]);
	uint16_t msgtype = UdpPort::Ntohs(s[1]);
	uint16_t msgsize = UdpPort::Ntohs(s[2]);
	uint16_t msgid = UdpPort::Ntohs(s[3]);

	{
		std::lock_guard<std::mutex> lock(m_answerMessageMapMutex);
		std::map<int, OnReceaveMessage>::const_iterator foundAnswerIt = m_answerMessageMap.find(msgid);
		if (foundAnswerIt != m_answerMessageMap.end()) {
			std::vector<uint8_t> message(msgsize);
			memcpy(message.data(), data.data() + 8, msgsize);
			foundAnswerIt->second(message, msgtype, msgstatus);
			m_answerMessageMap.erase(foundAnswerIt->first);
		}
	}

	m_exitCondition.notify_one();
}

void CDevice::OnFrameStreamHandler(const std::vector<uint8_t>& data)
{
	const GVSP_Header *ptrGVSP_Header = (const GVSP_Header *) data.data();

	uint16_t status = UdpPort::Ntohs(ptrGVSP_Header->Status);
	uint8_t packetFormat = ptrGVSP_Header->Packet_Format;

	if (status == MGevStatusSuccess) {
		switch (packetFormat) {
		case MGevPacketFormat::Leader:{
			m_currentFrameDataPtr.reset(new FrameData(data));

			break;
		}
		case MGevPacketFormat::Payload:
		{
			if (m_currentFrameDataPtr){
				m_currentFrameDataPtr->InsertFrameData(data);
			}

			break;
		}
		case MGevPacketFormat::Trailer:
		{
			if (m_currentFrameDataPtr){
				if(m_currentFrameDataPtr->IsValid){
					m_frameCb(*m_currentFrameDataPtr);
				}
				else{
					std::stringstream ss;
					ss << "Lost package: " << m_currentFrameDataPtr->LostPackets;
					OnError(Error::FrameStreamError, ss.str());
				}
			}
			break;
		}
		default:
			// wrong Packet Format - drop packet
			break;
		}
	}
	else {
		std::stringstream ss;
		ss << "Can not grab frame. Status: " << StringStatus(status);
		OnError(Error::FrameStreamError, ss.str());
	}
}


CDevice::UdpConnnectionPromisePtr CDevice::FindAll(uint32_t waitTimeMs)
{
	return ms_promises.CreateAsync<std::vector<UdpPort::Connection>, std::string>(
		[waitTimeMs](
		const TPromise<std::vector<UdpPort::Connection>, std::string>::OnResolveFunc& resolve,
		const TPromise<std::vector<UdpPort::Connection>, std::string>::OnRejectFunc& /*reject*/,
		const TPromise<std::vector<UdpPort::Connection>, std::string>::OnProgressFunc& /*progress*/,
		const TPromise<std::vector<UdpPort::Connection>, std::string>::IsCanceledFunc& /*isCanceled*/
	){
		const auto interfaces = UdpPort::GetLocalAddressList();
		
		std::mutex mutex;
		std::vector<UdpPort::Connection> result;

		{
		std::vector<UdpPortPtr> broadcastList;
		for (const auto& i : interfaces){
			UdpPortPtr broadcast(new UdpPort);

			broadcast->AddHandler([i, &result, &mutex](const std::vector<uint8_t>& data, uint32_t addr, uint16_t /*port*/){
				const uint16_t* s = (const uint16_t*)data.data();
				uint16_t msgtype = UdpPort::Ntohs(s[1]);

				if (msgtype != MGevMsgTypeWriteConfigMemory + 1){
					return;
				}

				std::lock_guard<std::mutex> lock(mutex);
				UdpPort::Connection conn;
				conn.localAddr = i;
				conn.cameraAddr = addr;
				result.emplace_back(conn);
			});

			if (!broadcast->Start(i, BroadcastPort, BroadcastAddr, GevPort)) {
				continue;
			}

			unsigned int t[2];
			t[0] = UdpPort::Htonl(MGevAddressControl);
			t[1] = UdpPort::Htonl(MGevValueExclusiveAccess);
			std::vector<uint8_t> data(8);
			memcpy(data.data(), t, 8);

			SendGevMessage(MGevMsgTypeWriteConfigMemory, data, *broadcast);
			broadcastList.push_back(broadcast);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeMs));
		}

		resolve(result);
	});
}

CDevice::CDevicePtr CDevice::Create(const UdpPort::Connection& connection, const TFrameCallBack& frameCb, int apiTimeoutMs, uint16_t packetSize, uint16_t apiPort, uint16_t imageStreamPort)
{
	CDevicePtr cameraPtr(new CDevice(connection, apiTimeoutMs, frameCb, packetSize, apiPort, imageStreamPort));
	return cameraPtr;
}

CDevice::CDevicePtr CDevice::Create(const UdpPort::Connection & connection, const TFrameCallBack & frameCb, const TErrorCallBack & onErrorCb, int apiTimeoutMs, uint16_t packetSize, uint16_t apiPort, uint16_t imageStreamPort)
{
	CDevicePtr cameraPtr = Create(connection, frameCb, apiTimeoutMs, packetSize, apiPort, imageStreamPort);
	cameraPtr->m_errorCb = onErrorCb;
	return cameraPtr;
}

bool CDevice::ReadMemoryBlock(uint32_t startaddress, uint32_t n, const std::function<void(const std::vector<uint8_t>& data, uint16_t status)>& answer)
{
	uint32_t t[2];
	t[0] = UdpPort::Htonl(startaddress);
	uint32_t requestn = n > (RecvBlockMax - RecvBlockHeader) ? RecvBlockMax - RecvBlockHeader : n;
	requestn = ((requestn + 3) / 4) * 4;
	t[1] = UdpPort::Htonl(requestn);
	std::vector<uint8_t> message(8);
	memcpy(message.data(), t, 8);

	std::lock_guard<std::mutex> lock(m_answerMessageMapMutex);
	int id = SendGevMessage(MGevMsgTypeReadConfigMemoryBlock, message, m_apiPort);

	if (id < 0){
		return false;
	}

	m_answerMessageMap.insert(std::make_pair(id, [answer, n](const std::vector<uint8_t>& message, uint16_t messageType, uint16_t status){
		if (messageType != MGevMsgTypeReadConfigMemoryBlock + 1){
			return;
		}

		if (message.size() < 4){
			return;
		}

		std::vector<uint8_t> messageMinusAddress(message.size() - 4);
		memcpy(messageMinusAddress.data(), message.data() + 4, messageMinusAddress.size());

		if (messageMinusAddress.size() > n){
			messageMinusAddress.erase(messageMinusAddress.begin() + n, messageMinusAddress.end());
		}

		answer(messageMinusAddress, status);
	}));

	return true;
}

bool CDevice::ReadRegisterMemory(uint32_t address, const std::function<void(const std::vector<uint8_t>& data, uint16_t status)>& answer)
{
	uint32_t t[1];
	t[0] = UdpPort::Htonl(uint32_t(address));
	std::vector<uint8_t> data(4);
	memcpy(data.data(), t, 4);

	std::lock_guard<std::mutex> lock(m_answerMessageMapMutex);
	int id = SendGevMessage(MGevMsgTypeReadConfigMemory, data, m_apiPort);
	
	if (id < 0){
		return false;
	}

	m_answerMessageMap.insert(std::make_pair(id, [answer](const std::vector<uint8_t>& message, uint16_t messageType, uint16_t status){
		if (messageType != MGevMsgTypeReadConfigMemory + 1){
			return;
		}

		answer(message, status);
	}));

	return true;
}

bool CDevice::WriteRegisterMemory(uint32_t address, const std::vector<uint8_t>& value, const std::function<void(const std::vector<uint8_t>& data, uint16_t status)>& answer)
{
	if (value.size() > 4){
		return false;
	}

	const uint32_t intValue =*((const uint32_t*) value.data());
	uint32_t t[2];
	t[0] = UdpPort::Htonl(address);
	t[1] = UdpPort::Htonl(intValue);
	std::vector<uint8_t> data(8);
	memcpy(data.data(), t, 8);

	std::lock_guard<std::mutex> lock(m_answerMessageMapMutex);
	int id = SendGevMessage(MGevMsgTypeWriteConfigMemory, data, m_apiPort);
	
	if (id < 0){
		return false;
	}

	m_answerMessageMap.insert(std::make_pair(id, [answer](const std::vector<uint8_t>& message, uint16_t messageType, uint16_t status){
		if (messageType != MGevMsgTypeWriteConfigMemory + 1){
			return;
		}

		answer(message, status);
	}));

	return true;
}

TPromise<std::vector<uint8_t>, std::string>::PromisePtr CDevice::ReadRegisterMemory(uint32_t address)
{
	return ms_promises.Create<std::vector<uint8_t>, std::string>([this, address](
			const TPromise<std::vector<uint8_t>, std::string>::OnResolveFunc& resolve,
			const TPromise<std::vector<uint8_t>, std::string>::OnRejectFunc& reject,
			const TPromise<std::vector<uint8_t>, std::string>::OnProgressFunc& /*progress*/,
			const TPromise<std::vector<uint8_t>, std::string>::IsCanceledFunc& /*isCanceled*/
	){
			if (!ReadRegisterMemory(address, [resolve, reject, address](const std::vector<uint8_t>& data, uint16_t status){
				if (status == MGevStatusSuccess){
					resolve(data);
				}
				else {
					std::stringstream errs;
					errs << "Can not read register. Address: " << address << " status: " << StringStatus(status);
					reject(errs.str());
				}
			})){
				std::stringstream errs;
				errs << "Can not read register. Address: " << address;
				reject(errs.str());
			};
	});
}

TPromise<bool, std::string>::PromisePtr CDevice::WriteRegisterMemory(uint32_t address, const std::vector<uint8_t>& data)
{
	return ms_promises.Create<bool, std::string>([this, address, data](
		const TPromise<bool, std::string>::OnResolveFunc& resolve,
		const TPromise<bool, std::string>::OnRejectFunc& reject,
		const TPromise<bool, std::string>::OnProgressFunc& /*progress*/,
		const TPromise<bool, std::string>::IsCanceledFunc& /*isCanceled*/
	){
		if (!WriteRegisterMemory(uint32_t(address), data, [resolve, reject, address](const std::vector<uint8_t>& /*data*/, uint16_t status){
			if (status == MGevStatusSuccess){
				resolve(true);
			} else {
				std::stringstream errs;
				errs << "Can not write register. Address: " << address << " status: " << StringStatus(status);
				reject(errs.str());
			}
		})){
			std::stringstream errs;
			errs << "Can not write register. Address: " << address;
			reject(errs.str());
		};
	});
}

TPromise<bool, std::string>::PromisePtr CDevice::SendControl()
{
	std::vector<uint8_t> data(4);
	memcpy(data.data(), &MGevValueExclusiveAccess, 4);

	return WriteRegisterMemory(MGevAddressControl, data);
}

TPromise<bool, std::string>::PromisePtr CDevice::SendHeartBeat(uint32_t cameraControlIntervalMs)
{
	std::vector<uint8_t> data(4);
	uint32_t beat = cameraControlIntervalMs * 4;
	memcpy(data.data(), &beat, 4);

	return WriteRegisterMemory(MGevHeartbeatTimeoutAddr, data);
}

void CDevice::Connect(uint32_t cameraControlIntervalMs)
{
	if (m_connected){
		return;
	}

	std::string error;

	m_apiPort.AddHandler([this](const std::vector<uint8_t>& data, uint32_t /*addr*/, uint16_t /*port*/){OnApiDataHandler(data);});
	m_frameStreamPort.AddHandler([this](const std::vector<uint8_t>& data, uint32_t /*addr*/, uint16_t /*port*/){OnFrameStreamHandler(data);});
	m_apiPort.Start(m_localIp, m_apiPortNum, m_cameraIp, GevPort);
	m_frameStreamPort.Start(m_localIp, m_imageStreamPortNum, m_cameraIp, GevPort, 0, 1024*1024*4);

	uint32_t localIp = UdpPort::Htonl(m_localIp);
	std::vector<uint8_t> ipBuff(4);
	memcpy(ipBuff.data(), &localIp, 4);

	bool ok;
	auto sendControl = SendControl();
	if (sendControl->Result(ok, error, m_apiTimeout)){
		auto sendIp = WriteRegisterMemory(MGevAddressDataStreamTargetIP, ipBuff);
		if (sendIp->Result(ok, error, m_apiTimeout)){
			std::vector<uint8_t> portBuff(2);
			memcpy(portBuff.data(), &m_imageStreamPortNum, 2);
			auto sendPort = WriteRegisterMemory(MGevAddressDataStreamTargetPort, portBuff);
			if (sendPort->Result(ok, error, m_apiTimeout)){
				std::vector<uint8_t> packSizeBuff(2);
				memcpy(packSizeBuff.data(), &m_packetSize, 2);
				auto sendPackSize = WriteRegisterMemory(MGevAddressDataStreamPacketSize, packSizeBuff);
				sendPackSize->Result(ok, error, m_apiTimeout);
			}
		}
	}
	else{
		Disconnect();
		OnError(Error::APIError, error);
		return;
	}

	auto xmlPromise = GetGenICamApiXml();
	std::string xml;
	if (!xmlPromise->Result(xml, error)) {
		Disconnect();
		OnError(Error::APIError, error);
		return;
	}

	try{
		m_genApi._LoadXMLFromString(xml.c_str());
		if(!m_genApi._Connect(this)){
			Disconnect();
			OnError(Error::APIError, "GenICam API connection error.");
			return;
		}
	}
	catch(const std::exception& e){
		Disconnect();
		OnError(Error::APIError, e.what());
		return;
	}

	m_apiXml = xml;

	m_connected = true;
	m_cameraControlThreadPtr.reset(new std::thread([this, cameraControlIntervalMs](){
		while (m_connected)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(cameraControlIntervalMs));

			for (int i = 0; i < 5; ++i){
				auto controlPromise = SendHeartBeat(cameraControlIntervalMs);

				bool result = false;
				std::string error;
				if (controlPromise->Result(result, m_apiTimeout)){
					break;
				}
				else{
					m_connected = false;
					OnError(Error::ConnectionError, "Head beat timeout");
				}
			}
		}

		m_apiPort.Stop();
		m_frameStreamPort.Stop();
	}));

	if (!error.empty()){
		Disconnect();
		OnError(Error::ConnectionError, error);
	}
}

void CDevice::Disconnect()
{
	m_connected = false;

	m_apiPort.Stop();
	m_frameStreamPort.Stop();

	if (m_cameraControlThreadPtr){
		m_cameraControlThreadPtr->join();
	}
}

bool CDevice::IsConnected() const
{
	return m_connected;
}

GenApi::EAccessMode CDevice::GetAccessMode() const
{
	return GenApi::RW;
}

void CDevice::Read(void * pBuffer, int64_t Address, int64_t Length)
{
	uint8_t *bufferPtr = (uint8_t *) pBuffer;

	while (Length > 0) {
		uint32_t length;

		if (Length > 4) {
			length = 4;
		}
		else {
			length = uint32_t(Length);
		}

		auto readRegister = ReadRegisterMemory(uint32_t(Address));

		std::vector<uint8_t> value;
		std::string error;
		if (!readRegister->Result(value, m_apiTimeout)){
			OnError(Error::APIError, error);
			break;
		}

		memcpy(bufferPtr, value.data(), length);

		Length -= length;
		bufferPtr += length;
		Address += length;
	}
}

void CDevice::Write(const void * pBuffer, int64_t Address, int64_t Length)
{
	bool ok;
	std::string error;
 
	const uint8_t* bufferPtr = (const uint8_t*)pBuffer;
	std::vector<uint8_t> buffer(4);

	while (Length > 0) {
		uint32_t value;
		uint32_t length;

		if (Length > 4) {
			length = 4;
		}
		else {
			length = uint32_t(Length);
		}
		value = 0;

		memcpy(&value, bufferPtr, length);
		value = UdpPort::Ntohl(value);
		memcpy(buffer.data(), &value, 4);

		auto writeRegister = WriteRegisterMemory(uint32_t(Address), buffer);

		if (!writeRegister->Result(ok, error, m_apiTimeout)){
			if (m_errorCb){
				m_errorCb(Error::APIError, error);
			}
			break;
		}

		Length -= length;
		bufferPtr += length;
		Address += length;
	}
}

std::string CDevice::ErrorTypeToString(Error type)
{
	switch (type)
	{
	case gevdevice::CDevice::ConnectionError:
		return "ConnectionError";
	case gevdevice::CDevice::FrameStreamError:
		return "FrameStreamError";
	case gevdevice::CDevice::APIError:
		return "APIError";
	default:
		break;
	}

	return "UnknownError";
}

const std::string & CDevice::GetApiXml() const
{
	return m_apiXml;
}

GenApi::CNodeMapRef CDevice::GetGenApi() const
{
	return m_genApi;
}

uint32_t CDevice::GetLocalIp() const
{
	return m_localIp;
}

uint32_t CDevice::GetCameraIp() const
{
	return m_cameraIp;
}

TPromise<std::string, std::string>::PromisePtr CDevice::GetGenICamApiXml()
{
	return ms_promises.CreateAsync<std::string, std::string>([this](
		const TPromise<std::string, std::string>::OnResolveFunc& resolve,
		const TPromise<std::string, std::string>::OnRejectFunc& reject,
		const TPromise<std::string, std::string>::OnProgressFunc& progress,
		const TPromise<std::string, std::string>::IsCanceledFunc& /*isCanceled*/
	){

		auto readBegin = ms_promises.Create<std::vector<uint8_t>, std::string>([this](
			const TPromise<std::vector<uint8_t>, std::string>::OnResolveFunc& resolve,
			const TPromise<std::vector<uint8_t>, std::string>::OnRejectFunc& reject,
			const TPromise<std::vector<uint8_t>, std::string>::OnProgressFunc& /*progress*/,
			const TPromise<std::vector<uint8_t>, std::string>::IsCanceledFunc& /*isCanceled*/
		){
			ReadMemoryBlock(MGevAddressGenICamZipFileInfoAddressFirst, 512, [resolve, reject](const std::vector<uint8_t>& data, uint16_t status){
				if (status == MGevStatusSuccess){
					resolve(data);
				}
				else {
					std::stringstream errs;
					errs << "Can not read api archive. Status: " << StringStatus(status);
					reject(errs.str());
				}
			});
		});

		std::vector<uint8_t> first;
		std::string error;
		if (!readBegin->Result(first, error)){
			reject(error);
			return;
		}

		char* exts = strchr((char*)first.data(), '.');

		if (exts == NULL){
			reject("GenICam api file was not found.");
			return;
		}

		char* addrs = strchr(exts + 1, ';');

		if (addrs == NULL){
			reject("GenICam api file begin was not found.");
			return;
		}

		char* sizes = strchr(addrs + 1, ';');

		if (sizes == NULL){
			reject("GenICam api file size was not found.");
			return;
		}

		std::string ext(exts + 1, addrs - exts - 1);
		uint32_t addr;
		uint32_t size;
		sscanf(addrs + 1, "%x", &addr);
		sscanf(sizes + 1, "%x", &size);

		if (size == 0) {
			resolve("");
		}

		const int totalSize = int(size);

		std::vector<uint8_t> file;

		while (size)
		{
			auto readFile = ms_promises.Create<std::vector<uint8_t>, std::string>([this, addr, size](
				const TPromise<std::vector<uint8_t>, std::string>::OnResolveFunc& resolve,
				const TPromise<std::vector<uint8_t>, std::string>::OnRejectFunc& reject,
				const TPromise<std::vector<uint8_t>, std::string>::OnProgressFunc& /*progress*/,
				const TPromise<std::vector<uint8_t>, std::string>::IsCanceledFunc& /*isCanceled*/
			){
				ReadMemoryBlock(addr, size, [resolve, reject](const std::vector<uint8_t>& data, uint16_t status){
					if (status == MGevStatusSuccess){
						resolve(data);
					}
					else {
						std::stringstream errs;
						errs << "Can not read api archive. Status: " << StringStatus(status);
						reject(errs.str());
					}
				});
			});

			std::vector<uint8_t> filePart;
			if (!readFile->Result(filePart, error)){
				reject(error);
			}

			file.insert(file.end(), filePart.begin(), filePart.end());

			size -= uint32_t(filePart.size());
			addr += uint32_t(filePart.size());

			progress(100 - 100*(totalSize - int(file.size()))/totalSize);
		}

		if (ext == "xml") {
			std::string xml((const char*)file.data(), file.size());
			resolve(xml);
		}
		else if (ext == "zip") {
			ZipExtract zip;
			std::vector<uint8_t> xmlData;
			if (!zip.Extract(xmlData, file)){
				reject("Can not extract zip archive.");
			}
			else{
				std::string xmlStr((const char*)xmlData.data(), xmlData.size());
				resolve(xmlStr);
			}
		}
		else {
			reject("Unsupported file format.");
		}
	});
}

void CDevice::OnError(Error errType, const std::string & err)
{
	if (m_errorCb){
		m_errorCb(errType, err);
	}
}

}
