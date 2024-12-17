#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

/*
Xnb file reader.

XNB files are read using the `load` function to load the file into memory,
then `read` is called which will interpret which reader to use from `registeredReaders`.
After this, the correct reader will be called to read the data into the target.

If a reader is not found, an exception will be thrown. You can register a reader
by providing a name and a void function(xnb* xnb, name, void* target) to `registerReader`.

Ideally xnb->data should be a pointer to the uncompressed data, but this is not implemented.
*/

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

void read7BitEncodedInt(std::istream* stream, int32_t* value);

typedef void (*reader)(xnb*, void*);

struct readerManager
{
	std::unordered_map<std::string, reader> readers;
};

void registerReader(readerManager* manager, const char* typeReaderName, reader reader);
void registerDefaultReaders(readerManager* manager);

/// <summary>
/// Reads the data from the xnb
/// </summary>
/// <param name="xnb">The xnb file-- should already have been loaded</param>
/// <param name="readers">The registered readers</param>
/// <param name="target">The target to read the data into</param>
void read(xnb* xnb, readerManager* manager, void* target);

}
