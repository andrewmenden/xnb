#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

namespace xnb
{
struct xnb
{
	//3+1+1+1+4=10 bytes
	uint8_t formatIdentifier[3] = {'X', 'N', 'B'};
	uint8_t platform = 'w';
	uint8_t format = 5;
	uint8_t flagBits = 0;
	uint32_t compressedSize = 0;

	//only if flagBits & 0x80
	uint32_t decompressedSize = 0;

	//could be compressed
	int32_t typeReaderCount = 0;
	std::vector<std::string> typeReaders;
	std::vector<int32_t> typeReaderVersions;
	int32_t sharedResourceCount = 0;
	char* data = nullptr;

	uint32_t dataSize = 0;

	//no shared resources
};

void load(xnb* xnb, const char* path);

void read7BitEncodedInt(std::ifstream* stream, int32_t* value);
}
