#include <xnb/xnb.h>
#include <xnb/readers.h>
#include <xnb/bufferReader.h>

namespace xnb
{

void readTexture2D(xnb* xnb, void* target)
{
	texture2D* texture = static_cast<texture2D*>(target);
	bufferReader reader(xnb->data, xnb->dataSize);

	//first byte is just the type reader index
	reader.seek(1, origin::current);

	reader.read(&texture->surfaceFormat);
	reader.read(&texture->width);
	reader.read(&texture->height);
	reader.read(&texture->mipCount);
	texture->mipMaps.resize(texture->mipCount);

	printf("surfaceFormat: %d\n", texture->surfaceFormat);
	printf("width: %d\n", texture->width);
	printf("height: %d\n", texture->height);
	printf("mipCount: %d\n", texture->mipCount);

	for (uint32_t i = 0; i < texture->mipCount; i++)
	{
		mipMap mip;
		reader.read(&mip.dataSize);
		mip.data.resize(mip.dataSize);
		reader.read(mip.data.data(), mip.dataSize);
		texture->mipMaps.push_back(mip);
	}
}

void readSoundEffect(xnb* xnb, void* target)
{
	soundEffect* sound = static_cast<soundEffect*>(target);
	bufferReader reader(xnb->data, xnb->dataSize);

	//first byte is just the type reader index
	reader.seek(1, origin::current);

	reader.read(&sound->formatSize);
	reader.read(&sound->wFormatTag);
	reader.read(&sound->nChannels);
	reader.read(&sound->nSamplesPerSec);
	reader.read(&sound->nAvgBytesPerSec);
	reader.read(&sound->nBlockAlign);
	reader.read(&sound->wBitsPerSample);
	reader.read(&sound->cbSize);
	reader.read(&sound->dataSize);
	sound->data.resize(sound->dataSize);
	reader.read(sound->data.data(), sound->dataSize);
	reader.read(&sound->loopStart);
	reader.read(&sound->loopLength);
	reader.read(&sound->duration);
}

void registerDefaultReaders(readerManager* manager)
{
	registerReader(manager, "Microsoft.Xna.Framework.Content.Texture2DReader", readTexture2D);
	registerReader(manager, "Microsoft.Xna.Framework.Content.SoundEffectReader", readSoundEffect);
}

}
