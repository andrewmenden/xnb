#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

//xnb = header + (decompressed size) + typeReaders + 0x00 + data
//if uncompressed, decompressed size is not present
//compression will compress typeReaders + 0x00 + data

//every type:
//  load from unique file: .png, .wav, .json
//  write to unique file: .png, .wav, .json
//  load from xnb file: .xnb
//  write to xnb file: .xnb

namespace xnb
{

//structs ----------------------------------------------------------------------

struct texture2d;
struct textureAtlas;
struct soundEffect;
struct header;

struct mipMap
{
    uint32_t size = 0;
    std::vector<char> data;
};

struct texture2D
{
	int32_t surfaceFormat = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t mipCount = 1;
	std::vector<mipMap> mips;
};

struct spriteFrame
{
    std::string name;
    int32_t rectX;
    int32_t rectY;
    int32_t rectWidth;
    int32_t rectHeight;
    float offsetX;
    float offsetY;
    float sizeWidth;
    float sizeHeight;
};

struct textureAtlas
{
    std::vector<spriteFrame> frames;
    int32_t width;
    int32_t height;
};

struct soundEffect
{
    uint32_t formatSize;

    //wave format ex
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;

    uint32_t dataSize;
    std::vector<char> data;

    int32_t loopStart;
    int32_t loopLength;
    int32_t duration;
};

struct header
{
	uint8_t formatIdentifier[3] = {'X', 'N', 'B'};
	uint8_t platform = 'w';
	uint8_t version = 5;
	uint8_t flagBits = 0; //0x80 = compressed
	uint32_t fileSize = 0;
};

struct typeReaders
{
    int32_t count;
    std::vector<std::string> names;
    std::vector<int32_t> versions;
};

//functions --------------------------------------------------------------------

int compressXnb(const char* inputFile, const char* outputFile);
int uncompressXnb(const char* inputFile, const char* outputFile);

int loadFromPng(const std::string& filename, texture2D& texture);
int writeToPng(const std::string& filename, const texture2D& texture);
int loadFromXnb(const std::string& filename, texture2D& texture);
int writeToXnb(const std::string& filename, const texture2D& texture, bool compress = false);

int loadFromJson(const std::string& filename, textureAtlas& atlas);
int writeToJson(const std::string& filename, const textureAtlas& atlas);
int loadFromXnb(const std::string& filename, textureAtlas& atlas);
int writeToXnb(const std::string& filename, const textureAtlas& atlas, bool compress = false);

int loadFromWav(const std::string& filename, soundEffect& sound);
int writeToWav(const std::string& filename, const soundEffect& sound);
int loadFromXnb(const std::string& filename, soundEffect& sound);
int writeToXnb(const std::string& filename, const soundEffect& sound, bool compress = false);

//helpers ----------------------------------------------------------------------

//expects the stream to be at the start of the file
int _readXnbHeader(std::istream& stream, header& header);

//expects the stream to be at the start of the file
int _handleXnbCompression(std::istream& stream, std::stringstream* decompressedStream);

//will return uncompressed typeReaders and move the stream to the start of the data
//expects the stream to be at the start of the readers
int _readXnbReaders(std::istream& stream, typeReaders& readers);

//will return uncompressed typeReaders + 0x00 + data
//expects the stream to be at the start of the data
std::stringstream _readXnbData(std::istream& stream);

int _writeXnbRest(std::ostream& file, std::stringstream& data, bool compress);

int _quickRead(const std::string& filename,
    header* header, typeReaders* readers, std::stringstream* data);

template<typename T>
T _read(std::istream& stream)
{
    T value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}


template<typename T>
void _write(std::ostream& stream, const T& value)
{
    stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

int32_t _read7BitEncodedInt(std::istream& stream);
void _write7BitEncodedInt(std::ostream& stream, int32_t value);

std::string _readString(std::istream& stream);
void _writeString(std::ostream& stream, const std::string& value);

}
