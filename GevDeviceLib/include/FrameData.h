#pragma once

#include <vector>
#include <cstdint>

namespace gevdevice{

/**	GeV Stream Protocol Header.
*/
typedef
struct _GVSP_Header
{
	uint16_t Status;
	uint16_t Block_ID;
	uint8_t Packet_Format;
	uint8_t Packet_ID_High8;
	uint16_t Packet_ID_Low16;
} GVSP_Header;

/**	GVSP Data Leader.
*/
typedef
struct _GVSP_Leader
{
	GVSP_Header GvspHeader;
	uint16_t Reserved;
	uint16_t PayloadType;
	uint32_t TimestampHigh;
	uint32_t TimestampLow;
} GVSP_Leader;

/**	GVSP Data Payload.
*/
typedef
struct _GVSP_Payload
{
	GVSP_Header GvspHeader;
	uint8_t Data;
} GVSP_Payload;

/**	GVSP Data Trailer.
*/
typedef
struct _GVSP_Trailer
{
	GVSP_Header GvspHeader;
	uint16_t Reserved;
	uint16_t PayloadType;
} GVSP_Trailer;

struct FrameData
{
	FrameData(const std::vector<uint8_t>& header);

	bool IsValid;
	size_t LostPackets;

	void InsertFrameData(const std::vector<uint8_t>& data);

	std::vector<uint8_t> Header;
	std::vector<uint8_t> Data;
	uint64_t Timestamp;

private:
	uint32_t m_lastPacketId;
};

}