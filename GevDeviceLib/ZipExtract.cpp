#include "ZipExtract.h"

#include <zlib.h>

namespace gevdevice{

//
// according to zip file specification: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
// we assume there will be only one file in the zip
//
void ZipFileGetOfsAndSize(const unsigned char * buf, unsigned int & ofs, unsigned int & sizecompressed, unsigned int & sizeuncompressed)
{
	unsigned int compressionmethod = *((const unsigned short int*)(buf + 8));

	sizecompressed = *((const unsigned int*)(buf + 18));
	sizeuncompressed = *((const unsigned int*)(buf + 22));
	unsigned int filenamelen = *((const unsigned short int*)(buf + 26));
	unsigned int extrafilelen = *((const unsigned short int*)(buf + 28));

	ofs = 30 + filenamelen + extrafilelen;
}


//
// These functions are exactly the same as uncompress() and uncompress2() except for the call to inflateInit().
// That got replaced with a call to inflateInit2() with windowbits set to -15, which allows for raw deflate.
//
int uncompress2raw(Bytef *dest, uLongf *destLen, const Bytef *source, uLong *sourceLen)
{
	z_stream stream;
	int err;
	const uInt max = (uInt)-1;
	uLong len, left;
	Byte buf[1];    /* for detection of incomplete stream when *destLen == 0 */

	len = *sourceLen;
	if (*destLen) {
		left = *destLen;
		*destLen = 0;
	}
	else {
		left = 1;
		dest = buf;
	}

	stream.next_in = (z_const Bytef *)source;
	stream.avail_in = 0;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;

	// https://stackoverflow.com/questions/8409764/zlib-raw-deflate-inflate-output-mismatch
	// -15 allows for raw inflates
	err = inflateInit2(&stream, -15);
	if (err != Z_OK) return err;

	stream.next_out = dest;
	stream.avail_out = 0;

	do {
		if (stream.avail_out == 0) {
			stream.avail_out = left > (uLong)max ? max : (uInt)left;
			left -= stream.avail_out;
		}
		if (stream.avail_in == 0) {
			stream.avail_in = len > (uLong)max ? max : (uInt)len;
			len -= stream.avail_in;
		}
		err = inflate(&stream, Z_NO_FLUSH);
	} while (err == Z_OK);

	*sourceLen -= len + stream.avail_in;
	if (dest != buf)
		*destLen = stream.total_out;
	else if (stream.total_out && err == Z_BUF_ERROR)
		left = 1;

	inflateEnd(&stream);
	return err == Z_STREAM_END ? Z_OK :
		err == Z_NEED_DICT ? Z_DATA_ERROR :
		err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR :
		err;
}

int uncompressraw(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
	return uncompress2raw(dest, destLen, source, &sourceLen);
}

bool ZipExtract::Extract(std::vector<uint8_t>& outData, const std::vector<uint8_t>& inData)
{
	outData.clear();

	unsigned int ofs;
	unsigned int sizecompressed;
	unsigned int sizeuncompressed;
	ZipFileGetOfsAndSize(inData.data(), ofs, sizecompressed, sizeuncompressed);
	unsigned int size = sizeuncompressed + 1;
	uint8_t* buff = new uint8_t[size];// + 1 for manually adding a 0 at the end for termination

	if(uncompressraw((Bytef*)buff, (uLongf*)&size, inData.data() + ofs, sizecompressed) == Z_OK){
		for (unsigned int i = 0; i < size; ++i){
			outData.push_back(buff[i]);
		}
		return true;
	}

	return false;
}

}
