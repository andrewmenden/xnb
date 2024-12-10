#include <xnb.h>

#include <fstream>
#include <istream>
#include <sstream>

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
		throw std::runtime_error("Invalid format");
	}
	
	std::istream* stream = &file;
	std::stringstream ss;
	if (xnb->flagBits & 0x80)
	{
		file.read(reinterpret_cast<char*>(&xnb->decompressedSize), sizeof(xnb->decompressedSize));
		throw std::runtime_error("Compressed files are not supported");
	}

	read7BitEncodedInt(&file, &xnb->typeReaderCount);
	for (int32_t i = 0; i < xnb->typeReaderCount; i++)
	{
		int32_t length;
		read7BitEncodedInt(&file, &length);
		char* buffer = new char[length];
		file.read(buffer, length);
		xnb->typeReaders.push_back(std::string(buffer, length));
		delete[] buffer;

		int32_t version;
		file.read(reinterpret_cast<char*>(&version), sizeof(version));
		xnb->typeReaderVersions.push_back(version);
	}

	read7BitEncodedInt(&file, &xnb->sharedResourceCount);

	//read data
	std::streampos begin = file.tellg();
	file.seekg(0, std::ios::end);
	std::streampos end = file.tellg();
	file.seekg(begin, std::ios::beg);
	xnb->dataSize = end - begin;
	xnb->data = new char[xnb->dataSize];

	file.read(xnb->data, xnb->dataSize);
	file.close();
}

void read7BitEncodedInt(std::ifstream* stream, int32_t* value)
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

}
