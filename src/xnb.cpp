#include <xnb/xnb.h>

#include <fstream>
#include <istream>
#include <sstream>
#include <lzx/LzxDecoder.h>

void stripTypeReaderName(std::string* typeReaderName)
{
	//Microsoft.Xna.Framework.Content.WhateverReader, junk -> Microsoft.Xna.Framework.Content.WhateverReader
	
	size_t pos = typeReaderName->find(',');
	if (pos != std::string::npos)
	{
		typeReaderName->erase(pos);
	}
}

namespace xnb
{

void load(xnb* xnb, const char* path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	//read first 8 bytes
	file.read(reinterpret_cast<char*>(xnb), 10);
	if (xnb->formatIdentifier[0] != 'X' || xnb->formatIdentifier[1] != 'N' || xnb->formatIdentifier[2] != 'B')
	{
		throw std::runtime_error("Invalid format identifier");
	}

	if (xnb->platform != 'w')
	{
		throw std::runtime_error("Invalid platform");
	}

	if (xnb->format != 5)
	{
		throw std::runtime_error("Invalid version");
	}
	
	std::istream* stream = &file;
	std::stringstream ss;
	if (xnb->flagBits & 0x80)
	{
		file.read(reinterpret_cast<char*>(&xnb->decompressedSize), sizeof(xnb->decompressedSize));

		char* decompressedData = new char[xnb->decompressedSize];
		uint32_t curr = file.tellg();
		file.seekg(0, std::ios::end);
		uint32_t size = file.tellg();
		file.seekg(curr, std::ios::beg);
		lzx::decompress(&file, decompressedData, xnb->decompressedSize, size);

		//TODO: avoid redundant copy by using a custom stream
		ss.write(decompressedData, xnb->decompressedSize);
		file.close();
		delete[] decompressedData;
		stream = &ss;
	}
	else if (xnb->flagBits != 0)
	{
		throw std::runtime_error("Invalid flag bits-- likely LZ4 compressed");
	}

	read7BitEncodedInt(stream, &xnb->typeReaderCount);
	for (int32_t i = 0; i < xnb->typeReaderCount; i++)
	{
		int32_t length;
		read7BitEncodedInt(stream, &length);
		char* buffer = new char[length];
		stream->read(buffer, length);
		xnb->typeReaders.push_back(std::string(buffer, length));
		stripTypeReaderName(&xnb->typeReaders.back());
		delete[] buffer;

		int32_t version;
		stream->read(reinterpret_cast<char*>(&version), sizeof(version));
		xnb->typeReaderVersions.push_back(version);
	}

	read7BitEncodedInt(stream, &xnb->sharedResourceCount);
	if (xnb->sharedResourceCount != 0)
	{
		throw std::runtime_error("Shared resources are not supported");
	}

	//read data
	std::streampos begin = stream->tellg();
	stream->seekg(0, std::ios::end);
	std::streampos end = stream->tellg();
	stream->seekg(begin, std::ios::beg);
	xnb->dataSize = end - begin;
	xnb->data = new char[xnb->dataSize];

	stream->read(xnb->data, xnb->dataSize);

	if (file.is_open())
	{
		file.close();
	}
}

void read7BitEncodedInt(std::istream* stream, int32_t* value)
{
	*value = 0;
	int32_t shift = 0;
	uint8_t byte;
	do
	{
		stream->read(reinterpret_cast<char*>(&byte), sizeof(byte));
		*value |= (byte & 0x7F) << shift;
		shift += 7;
	} while (byte & 0x80);
}

void registerReader(readerManager* manager, const char* typeReaderName, reader reader)
{
	manager->readers[typeReaderName] = reader;
}

void read(xnb* xnb, readerManager* readers, void* target)
{
	if (xnb->typeReaderCount < 1)
	{
		throw std::runtime_error("Invalid type reader count");
	}
	if (xnb->data == nullptr)
	{
		throw std::runtime_error("Data is not set");
	}
	uint8_t typeReaderIndex = xnb->data[0]-1;

	//check if reader is registered
	if (readers->readers.find(xnb->typeReaders[typeReaderIndex]) == readers->readers.end())
	{
		throw std::runtime_error("Type reader not registered");
	}

	readers->readers[xnb->typeReaders[typeReaderIndex]](xnb, target);
}

void read(const char* path, readerManager* readers, void* target)
{
	xnb xnb;
	load(&xnb, path);
	read(&xnb, readers, target);
}

}
