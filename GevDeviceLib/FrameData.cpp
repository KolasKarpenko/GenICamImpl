#include "FrameData.h"

#include "UdpPort.h"

#include <iostream>

namespace gevdevice
{

FrameData::FrameData(const std::vector<uint8_t>& header) : Header(header) , IsValid(true), LostPackets(0), m_lastPacketId(0)
{
	const GVSP_Leader *ptrGVSP_Leader = (const GVSP_Leader *) header.data();
	Timestamp = ((uint64_t)(UdpPort::Ntohl(ptrGVSP_Leader->TimestampHigh)) << 32) | (uint64_t) (UdpPort::Ntohl(ptrGVSP_Leader->TimestampLow));
}

void FrameData::InsertFrameData(const std::vector<uint8_t>& data)
{
	const GVSP_Header *ptrGVSP_Header = (const GVSP_Header *) data.data();
	uint32_t packetId = ((ptrGVSP_Header->Packet_ID_High8 & 0xFF) << 16) + (UdpPort::Ntohs(ptrGVSP_Header->Packet_ID_Low16) & 0x0000FFFF);
	
	if (packetId - m_lastPacketId != 1){
		IsValid = false;
		LostPackets++;
	}

	m_lastPacketId = packetId;

	Data.insert(Data.end(), data.begin() + sizeof(GVSP_Header), data.end());
}

}