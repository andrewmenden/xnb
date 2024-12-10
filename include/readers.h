#include <xnb.h>

struct soundEffect
{
	//2+2+4+4+2+2+2+4=22 bytes
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;

	uint32_t dataSize;
	char *data;

	int32_t loopStart;
	int32_t loopLength;
	int32_t duration;
};

namespace xnb
{
void readSoundEffect(xnb* xnb, soundEffect* soundEffect);

void readBE(char* data, char* dest, uint32_t size);
void readLE(char* data, char* dest, uint32_t size);

}
