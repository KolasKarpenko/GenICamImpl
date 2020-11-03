#include "ImageData.h"

#include "UdpPort.h"

namespace gevdevice
{

ImageData::ImageData() : Timestamp(0), IsValid(false)
{
}

ImageData::ImageData(const FrameData& frameData) : Timestamp(frameData.Timestamp), IsValid(frameData.IsValid)
{
	if (!IsValid){
		LostPackets = frameData.LostPackets;
		return;
	}

	const GVSP_Image_Leader *ptrGVSP_Image_Leader = (const GVSP_Image_Leader *) frameData.Header.data();

	PixelType = static_cast<GVSP_PIXEL_TYPES>(UdpPort::Ntohl(ptrGVSP_Image_Leader->PixelType));
	SizeX = UdpPort::Ntohl(ptrGVSP_Image_Leader->SizeX);
	SizeY = UdpPort::Ntohl(ptrGVSP_Image_Leader->SizeY);
	OffsetX = UdpPort::Ntohl(ptrGVSP_Image_Leader->OffsetX);
	OffsetY = UdpPort::Ntohl(ptrGVSP_Image_Leader->OffsetY);
	PaddingX = UdpPort::Ntohs(ptrGVSP_Image_Leader->PaddingX);
	PaddingY = UdpPort::Ntohs(ptrGVSP_Image_Leader->PaddingY);
	Bitmap = frameData.Data;
}


void fif_Demosaic_algBilinear_patternRGGB_inRaw8_outBGR24(uint32_t sizeX, uint32_t sizeY, const uint8_t* pInData, std::vector<uint8_t>& pOutData)
{
	const uint8_t *rowm1, *row0, *rowp1;

	uint16_t x, y;
	short v;
	uint8_t r, g, b;


	for (y = 1; y < sizeY - 1; y++) 
	{
		for (x = 1; x < sizeX - 1; x++)
		{
			row0  = pInData + x + y * sizeX;
			rowm1 = row0 - sizeX;
			rowp1 = row0 + sizeX;

			r = *row0;
			g = *row0;
			b = *row0;

			if (y % 2 == 0)
			{
				if (x % 2 == 1)
				{
					// berechne: R at green in R row, B column.
					v = (*(row0 - 1) // (row,col+1)
						+ *(row0 + 1) // (row, col-1)
						) >> 1;

					if (v > 255)
						r = 255;
					else if(v < 0)
						r = 0;
					else 
						r = (uint8_t)v;

					// berechne: B at green in R row, B column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						b = 255;
					else if(v < 0)
						b = 0;
					else
						b = (uint8_t)v;
				} else {
					// berechne: B at red in R row, R column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						b = 255;
					else if(v < 0)
						b = 0;
					else
						b = (uint8_t)v;

					// berechne: G at R locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						g = 255;
					else if(v < 0)
						g = 0;
					else
						g = (uint8_t)v;
				}
			} else {
				if (x % 2 == 1)
				{
					// berechne: R at blue in B row, B column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						r = 255;
					else if(v < 0)
						r = 0;
					else
						r = (uint8_t)v;

					// berechne: G at B locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						g = 255;
					else if(v < 0)
						g = 0;
					else
						g = (uint8_t)v;
				} else {
					// berechne: R at green in B row, R column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						r = 255;
					else if(v < 0)
						r = 0;
					else
						r = (uint8_t)v;

					// berechne: B at green in B row, R column.
					v = (*(row0+1) // (row,col+1)
						+ *(row0-1) // (row,col-1)
						) >> 1;

					if (v > 255)
						b = 255;
					else if(v < 0)
						b = 0;
					else
						b = (uint8_t)v;
				}
			}

			uint32_t offset = (x + y * sizeX) * 3;
			pOutData[offset++] = r; // offset
			pOutData[offset++] = g; // offset + 1
			pOutData[offset] = b; // offset + 2
		}
	}
}

void fif_Demosaic_algBilinear_patternGRBG_inRaw8_outBGR24(uint32_t sizeX, uint32_t sizeY, const uint8_t* pInData, std::vector<uint8_t>& pOutData)
{
	const uint8_t *rowm1, *row0, *rowp1;

	uint16_t x, y;
	short v;
	uint8_t r, g, b;

	for (y = 1; y < sizeY - 1; y++)
	{
		for (x = 1; x < sizeX - 1; x++)
		{
			row0  = pInData + x + y * sizeX;
			rowm1 = row0 - sizeX;
			rowp1 = row0 + sizeX;

			r = *row0;
			g = *row0;
			b = *row0;

			if (y % 2 == 0)
			{
				if (x % 2 == 0)
				{
					// // berechne: R at green in R row, B column.
					v = (*(row0 - 1) // (row,col+1)
						+ *(row0 + 1) // (row, col-1)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: B at green in R row, B column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;
				} else {
					// // berechne: B at red in R row, R column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;

					// // berechne: G at R locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					g = (uint8_t)v;
				}
			} else {
				if (x % 2 == 0)
				{
					// // berechne: R at blue in B row, B column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: G at B locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					g = (uint8_t)v;
				} else {
					// // berechne: R at green in B row, R column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: B at green in B row, R column.
					v = (*(row0+1) // (row,col+1)
						+ *(row0-1) // (row,col-1)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;
				}
			}

			uint32_t offset = (x + y * sizeX) * 3;
			pOutData[offset++] = r; // offset
			pOutData[offset++] = g; // offset + 1
			pOutData[offset] = b; // offset + 2
		}
	}
}

void fif_Demosaic_algBilinear_patternBGGR_inRaw8_outBGR24(uint32_t sizeX, uint32_t sizeY, const uint8_t* pInData, std::vector<uint8_t>& pOutData)
{
	const uint8_t *rowm1, *row0, *rowp1;

	uint16_t x, y;
	short v;
	uint8_t r, g, b;

	for (y = 1; y < sizeY - 1; y++) 
	{
		for (x = 1; x < sizeX - 1; x++)
		{
			row0  = pInData + x + y * sizeX;
			rowm1 = row0 - sizeX;
			rowp1 = row0 + sizeX;

			r = *row0;
			g = *row0;
			b = *row0;

			if (y % 2 == 0)
			{
				if (x % 2 == 1)
				{
					// // berechne: R at green in B row, R column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: B at green in B row, R column.
					v = (*(row0+1) // (row,col+1)
						+ *(row0-1) // (row,col-1)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;
				} else {
					// // berechne: R at blue in B row, B column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: G at B locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					g = (uint8_t)v;
				}
			} else {
				if (x % 2 == 1)
				{
					// // berechne: B at red in R row, R column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;

					// // berechne: G at R locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					g = (uint8_t)v;
				} else {
					// // berechne: R at green in R row, B column.
					v = (*(row0 - 1) // (row,col+1)
						+ *(row0 + 1) // (row, col-1)
						) >> 1;	

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: B at green in R row, B column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;
				}
			}

			uint32_t offset = (x + y * sizeX) * 3;
			pOutData[offset++] = r; // offset
			pOutData[offset++] = g; // offset + 1
			pOutData[offset] = b; // offset + 2
		}
	}
}

void fif_Demosaic_algBilinear_patternGBRG_inRaw8_outBGR24(uint32_t sizeX, uint32_t sizeY, const uint8_t* pInData, std::vector<uint8_t>& pOutData)
{
	const uint8_t *rowm1, *row0, *rowp1;

	uint16_t x, y;
	short v;
	uint8_t r, g, b;

	for (y = 1; y < sizeY - 1; y++) 
	{
		for (x = 1; x < sizeX - 1; x++)
		{
			row0  = pInData + x + y * sizeX;
			rowm1 = row0 - sizeX;
			rowp1 = row0 + sizeX;

			r = *row0;
			g = *row0;
			b = *row0;

			if (y % 2 == 0)
			{
				if (x % 2 == 0)
				{
					// // berechne: R at green in B row, R column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: B at green in B row, R column.
					v = (*(row0+1) // (row,col+1)
						+ *(row0-1) // (row,col-1)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;
				} else {
					// // berechne: R at blue in B row, B column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: G at B locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					g = (uint8_t)v;
				}
			} else {
				if (x % 2 == 0)
				{
					// // berechne: B at red in R row, R column.
					v = (*(rowm1-1) // (row-1,col-1)
						+ *(rowm1+1) // (row-1,col+1)
						+ *(rowp1+1) // (row+1,col+1)
						+ *(rowp1-1) // (row+1,col-1)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;

					// // berechne: G at R locations.
					v = (*(row0-1) // (row,col-1)
						+ *(rowm1) // (row-1,col)
						+ *(row0+1) // (row,col+1)
						+ *(rowp1) // (row+1,col)
						) >> 2;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					g = (uint8_t)v;
				} else {
					// // berechne: R at green in R row, B column.
					v = (*(row0 - 1) // (row,col+1)
						+ *(row0 + 1) // (row, col-1)
						) >> 1;	

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					r = (uint8_t)v;

					// // berechne: B at green in R row, B column.
					v = (*(rowm1) // (row-1,col)
						+ *(rowp1) // (row+1,col)
						) >> 1;

					if (v > 255)
						v = 255;
					else if( v < 0)
						v = 0;

					b = (uint8_t)v;
				}
			}

			uint32_t offset = (x + y * sizeX) * 3;
			pOutData[offset++] = r; // offset
			pOutData[offset++] = g; // offset + 1
			pOutData[offset] = b; // offset + 2
		}
	}
}

ImageData ImageData::Demosaic(const ImageData & image)
{
	const uint8_t* pInData = image.Bitmap.data();
	ImageData result;
	result.Timestamp = image.Timestamp;
	result.SizeX = image.SizeX;
	result.SizeY = image.SizeY;
	result.Bitmap.resize(3*image.SizeX*image.SizeY);
	result.IsValid = true;
	result.PixelType = GVSP_PIX_RGB8_PACKED;

	switch(image.PixelType) {
	case GVSP_PIX_BAYRG8:
		fif_Demosaic_algBilinear_patternRGGB_inRaw8_outBGR24(image.SizeX, image.SizeY, pInData, result.Bitmap);
		break;
	case GVSP_PIX_BAYGR8:
		fif_Demosaic_algBilinear_patternGRBG_inRaw8_outBGR24(image.SizeX, image.SizeY, pInData, result.Bitmap);
		break;
	case GVSP_PIX_BAYBG8:
		fif_Demosaic_algBilinear_patternBGGR_inRaw8_outBGR24(image.SizeX, image.SizeY, pInData, result.Bitmap);
		break;
	case GVSP_PIX_BAYGB8:
		fif_Demosaic_algBilinear_patternGBRG_inRaw8_outBGR24(image.SizeX, image.SizeY, pInData, result.Bitmap);
		break;
	default:
		return image;
	}

	return result;
}

}