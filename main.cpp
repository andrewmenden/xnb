#include <xnb.h>
#include <cstdio>

int main()
{
	std::string path = "C:/Users/andre/Downloads/exampleWav.xnb";
	xnb::xnb xnb;
	xnb::load(&xnb, path.c_str());

	printf("Format: %c%c%c\n", xnb.formatIdentifier[0], xnb.formatIdentifier[1], xnb.formatIdentifier[2]);
	printf("Platform: %c\n", xnb.platform);
	printf("Format: %d\n", xnb.format);
	//using hex
	printf("Flag bits: 0x%02x\n", xnb.flagBits);
	printf("Compressed size: %d\n", xnb.compressedSize);
	if (xnb.flagBits & 0x80)
	{
		printf("Decompressed size: %d\n", xnb.decompressedSize);
	}
	printf("Type reader count: %d\n", xnb.typeReaderCount);
	for (int32_t i = 0; i < xnb.typeReaderCount; i++)
	{
		printf("Type reader %d: %s\n", i, xnb.typeReaders[i].c_str());
		printf("Type reader version %d: %d\n", i, xnb.typeReaderVersions[i]);
	}
	printf("Shared resource count: %d\n", xnb.sharedResourceCount);
	printf("Data size: %d\n", xnb.dataSize);
}
