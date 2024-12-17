#pragma once

#include <xnb/xnb.h>
#include <vector>

namespace xnb
{

//Primitive types --------------------------------

//System types -----------------------------------

//Graphics types ---------------------------------

struct mipMap
{
	uint32_t dataSize;
	std::vector<char> data;
};

struct texture2D
{
	int32_t surfaceFormat;
	uint32_t width;
	uint32_t height;
	uint32_t mipCount;
	std::vector<mipMap> mipMaps;
};

//Media types ------------------------------------

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

}

namespace xnb
{

//Primitives -------------------------------------

//System types -----------------------------------

//Graphics types ---------------------------------

//Microsoft.Xna.Framework.Content.Texture2DReader
void readTexture2D(xnb* xnb, void* target);

//Media types ------------------------------------

//Microsoft.Xna.Framework.Content.SoundEffectReader
void readSoundEffect(xnb* xnb, void* target);

}
