#include <xnb.h>
#include <lzxDecoder.h>
#include <xcompress.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <json.hpp>

#include <fstream>

namespace xnb
{

//functions --------------------------------------------------------------------

int compressXnb(const char* inputFile, const char* outputFile)
{
    header header;
    typeReaders readers;
    std::stringstream datai;

    int ret = _quickRead(inputFile, &header, &readers, &datai);

    if (ret != 0)
    {
        return ret;
    }

    std::ofstream file(outputFile, std::ios::binary);

    _write<char>(file, 'X');
    _write<char>(file, 'N');
    _write<char>(file, 'B');
    _write<char>(file, 'w');
    _write<char>(file, 5);
    _write<char>(file, 0x80); //compressed

    std::stringstream data;
    _write7BitEncodedInt(data, readers.count); //type reader count
    for (int i = 0; i < readers.count; i++)
    {
        _writeString(data, readers.names[i]);
        _write<int32_t>(data, readers.versions[i]);
    }
    _write7BitEncodedInt(data, 0); //shared resource count

    data << datai.rdbuf();

    _writeXnbRest(file, data, true);

    return 0;
}

int uncompressXnb(const char* inputFile, const char* outputFile)
{
    header header;
    typeReaders readers;
    std::stringstream datai;

    int ret = _quickRead(inputFile, &header, &readers, &datai);

    if (ret != 0)
    {
        return ret;
    }

    std::ofstream file(outputFile, std::ios::binary);

    _write<char>(file, 'X');
    _write<char>(file, 'N');
    _write<char>(file, 'B');
    _write<char>(file, 'w');
    _write<char>(file, 5);
    /*_write<char>(file, 0x00); //uncompressed*/

    std::stringstream data;
    _write7BitEncodedInt(data, readers.count); //type reader count
    for (int i = 0; i < readers.count; i++)
    {
        _writeString(data, readers.names[i]);
        _write<int32_t>(data, readers.versions[i]);
    }
    _write7BitEncodedInt(data, 0); //shared resource count

    data << datai.rdbuf();

    _writeXnbRest(file, data, false);

    return 0;
}

int loadFromPng(const std::string& filename, texture2D& texture)
{
    int width, height, channels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    if (!data)
    {
        return 1;
    }

    texture.surfaceFormat = 0;
    texture.width = width;
    texture.height = height;
    texture.mipCount = 1;
    texture.mips.resize(1);
    texture.mips[0].size = width * height * 4;
    texture.mips[0].data.resize(width * height * 4);
    for (int i = 0; i < width * height * 4; i++)
    {
        texture.mips[0].data[i] = data[i];
    }

    stbi_image_free(data);

    return 0;
    return 0;
}

int writeToPng(const std::string& filename, const texture2D& texture)
{
    if (texture.mipCount != 1)
    {
        return 1;
    }

    if (texture.mips[0].data.size() != texture.width * texture.height * 4)
    {
        return 1;
    }

    stbi_write_png(filename.c_str(), texture.width, texture.height, 4, texture.mips[0].data.data(), texture.width * 4);

    return 0;
}

int loadFromXnb(const std::string& filename, texture2D& texture)
{
    header header;
    typeReaders readers;
    std::stringstream data;

    int ret = _quickRead(filename, &header, &readers, &data);

    if (ret != 0)
    {
        return ret;
    }

    _read<char>(data); //typeReaderIndex

    texture.surfaceFormat = _read<int32_t>(data);
    texture.width = _read<uint32_t>(data);
    texture.height = _read<uint32_t>(data);
    texture.mipCount = _read<uint32_t>(data);

    texture.mips.resize(texture.mipCount);
    for (uint32_t i = 0; i < texture.mipCount; i++)
    {
        texture.mips.push_back(mipMap());
        mipMap& mip = texture.mips[i];
        uint32_t mipSize = _read<uint32_t>(data);
        mip.size = mipSize;
        mip.data.resize(mipSize);
        for (uint32_t j = 0; j < mipSize; j++)
        {
            mip.data[j] = _read<char>(data);
        }
    }

    return 0;
}

int writeToXnb(const std::string& filename, const texture2D& texture, bool compress)
{
    std::ofstream file(filename, std::ios::binary);

    _write<char>(file, 'X');
    _write<char>(file, 'N');
    _write<char>(file, 'B');
    _write<char>(file, 'w');
    _write<char>(file, 5);

    std::stringstream data;
    _write7BitEncodedInt(data, 1); //type reader count
    std::string typeReaderName = "Microsoft.Xna.Framework.Content.Texture2DReader";
    _writeString(data, typeReaderName);
    _write<int32_t>(data, 0); //version
    _write7BitEncodedInt(data, 0); //shareed resource count

    //texture2D specific
    _write<char>(data, 1); //typeReaderIndex

    _write<int32_t>(data, texture.surfaceFormat);
    _write<uint32_t>(data, texture.width);
    _write<uint32_t>(data, texture.height);
    /*_write<uint32_t>(data, texture.mipCount);*/
    _write<uint32_t>(data, (uint32_t)texture.mips.size());

    for (uint32_t i = 0; i < (uint32_t)texture.mips.size(); i++)
    {
        const mipMap& mip = texture.mips[i];
        _write<uint32_t>(data, mip.size);
        for (uint32_t j = 0; j < mip.size; j++)
        {
            _write<char>(data, mip.data[j]);
        }
    }

    _writeXnbRest(file, data, compress);

    return 0;
}

int loadFromJson(const std::string& filename, textureAtlas& atlas)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return 1;
    }

    nlohmann::json j;
    file >> j;

    atlas.width = j["base_w"];
    atlas.height = j["base_h"];
    for (auto& item : j["frames"].items())
    {
        atlas.frames.push_back(spriteFrame());
        spriteFrame& frame = atlas.frames.back();
        frame.name = item.key();
        frame.rectX = item.value()["sprite_x"];
        frame.rectY = item.value()["sprite_y"];
        frame.rectWidth = item.value()["sprite_w"];
        frame.rectHeight = item.value()["sprite_h"];
        frame.offsetX = item.value()["bounding_box_x"];
        frame.offsetY = item.value()["bounding_box_y"];
        frame.sizeWidth = item.value()["bounding_box_w"];
        frame.sizeHeight = item.value()["bounding_box_h"];
    }

    return 0;
}

int writeToJson(const std::string& filename, const textureAtlas& atlas)
{
    nlohmann::json j;
    j["base_w"] = atlas.width;
    j["base_h"] = atlas.height;
    for (const spriteFrame& frame : atlas.frames)
    {
        nlohmann::json frameJ;
        frameJ["sprite_x"] = frame.rectX;
        frameJ["sprite_y"] = frame.rectY;
        frameJ["sprite_w"] = frame.rectWidth;
        frameJ["sprite_h"] = frame.rectHeight;
        frameJ["bounding_box_x"] = frame.offsetX;
        frameJ["bounding_box_y"] = frame.offsetY;
        frameJ["bounding_box_w"] = frame.sizeWidth;
        frameJ["bounding_box_h"] = frame.sizeHeight;
        j["frames"][frame.name] = frameJ;
    }

    std::ofstream file(filename);
    if (!file.is_open())
    {
        return 1;
    }

    file << j.dump(4);

    return 0;
}

int loadFromXnb(const std::string& filename, textureAtlas& atlas)
{
    header header;
    typeReaders readers;
    std::stringstream data;

    int ret = _quickRead(filename, &header, &readers, &data);

    if (ret != 0)
    {
        return ret;
    }

    _read<char>(data); //type reader

    int32_t frameCount = _read<int32_t>(data);
    atlas.frames.resize(frameCount);
    for (int32_t i = 0; i < frameCount; i++)
    {
        spriteFrame& frame = atlas.frames[i];
        _read<char>(data); //type reader
        frame.name = _readString(data);
        _read<char>(data); //type reader
        frame.rectX = _read<int32_t>(data);
        frame.rectY = _read<int32_t>(data);
        frame.rectWidth = _read<int32_t>(data);
        frame.rectHeight = _read<int32_t>(data);
        frame.offsetX = _read<float>(data);
        frame.offsetY = _read<float>(data);
        frame.sizeWidth = _read<float>(data);
        frame.sizeHeight = _read<float>(data);
    }
    atlas.width = _read<int32_t>(data);
    atlas.height = _read<int32_t>(data);

    return 0;
}

int writeToXnb(const std::string& filename, const textureAtlas& atlas, bool compress)
{
    std::ofstream file(filename, std::ios::binary);

    _write<char>(file, 'X');
    _write<char>(file, 'N');
    _write<char>(file, 'B');
    _write<char>(file, 'w');
    _write<char>(file, 5);

    std::stringstream data;
    _write7BitEncodedInt(data, 3); //type reader count
    std::string typeReaderName1 = "TextureAtlasContent.TextureAtlasReader, TextureAtlasContentPC, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null";
    std::string typeReaderName2 = "TextureAtlasContent.TextureRegionReader, TextureAtlasContentPC, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null";
    std::string typeReaderName3 = "Microsoft.Xna.Framework.Content.RectangleReader";
    _writeString(data, typeReaderName1);
    _write<int32_t>(data, 0); //version
    _writeString(data, typeReaderName2);
    _write<int32_t>(data, 0); //version
    _writeString(data, typeReaderName3);
    _write<int32_t>(data, 0); //version
    _write7BitEncodedInt(data, 0); //shared resource count

    //texture2D specific
    _write<char>(data, 1); //typeReaderIndex

    _write<int32_t>(data, (int32_t)atlas.frames.size());
    for (const spriteFrame& frame : atlas.frames)
    {
        _write<char>(data, 2); //typeReaderIndex
        _writeString(data, frame.name);
        _write<char>(data, 3); //typeReaderIndex
        _write<int32_t>(data, frame.rectX);
        _write<int32_t>(data, frame.rectY);
        _write<int32_t>(data, frame.rectWidth);
        _write<int32_t>(data, frame.rectHeight);
        _write<float>(data, frame.offsetX);
        _write<float>(data, frame.offsetY);
        _write<float>(data, frame.sizeWidth);
        _write<float>(data, frame.sizeHeight);
    }
    _write<int32_t>(data, atlas.width);
    _write<int32_t>(data, atlas.height);

    _writeXnbRest(file, data, compress);

    return 0;
}

int loadFromWav(const std::string& filename, soundEffect& sound)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return 1;
    }

    char riff[4];
    file.read(riff, 4);
    if (riff[0] != 'R' || riff[1] != 'I' || riff[2] != 'F' || riff[3] != 'F')
    {
        return 1;
    }

    int32_t fileSize = 0; 
    _read<int32_t>(file); //file size

    char wave[4];
    file.read(wave, 4);

    if (wave[0] != 'W' || wave[1] != 'A' || wave[2] != 'V' || wave[3] != 'E')
    {
        return 1;
    }

    char fmt[4];
    file.read(fmt, 4);

    if (fmt[0] != 'f' || fmt[1] != 'm' || fmt[2] != 't' || fmt[3] != ' ')
    {
        return 1;
    }

    //20
    uint32_t blockSize = _read<uint32_t>(file);
    if (blockSize != 16)
    {
        return 1;
    }

    sound.formatSize = 0x12;
    sound.wFormatTag = _read<uint16_t>(file);
    sound.nChannels = _read<uint16_t>(file);
    sound.nSamplesPerSec = _read<uint32_t>(file);
    sound.nAvgBytesPerSec = _read<uint32_t>(file);
    sound.nBlockAlign = _read<uint16_t>(file);
    sound.wBitsPerSample = _read<uint16_t>(file);
    /*sound.cbSize = _read<uint16_t>(file);*/
    sound.cbSize = 0;

    char data[4];
    file.read(data, 4);

    if (data[0] != 'd' || data[1] != 'a' || data[2] != 't' || data[3] != 'a')
    {
        return 1;
    }

    sound.dataSize = _read<uint32_t>(file);
    sound.data.resize(sound.dataSize);
    file.read(reinterpret_cast<char*>(sound.data.data()), sound.dataSize);

    sound.loopStart = 0;
    sound.loopLength = sound.dataSize / sound.nBlockAlign;
    sound.duration =
        (int)(1000 * sound.dataSize /
        (sound.nChannels * sound.wBitsPerSample * sound.nSamplesPerSec / 8));

    return 0;
}

int writeToWav(const std::string& filename, const soundEffect& sound)
{
    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return 1;
    }

    file.write("RIFF", 4);
    _write<int32_t>(file, sound.dataSize + 36);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    _write<uint32_t>(file, 16); 

    _write<uint16_t>(file, sound.wFormatTag);
    _write<uint16_t>(file, sound.nChannels);
    _write<uint32_t>(file, sound.nSamplesPerSec);
    _write<uint32_t>(file, sound.nAvgBytesPerSec);
    _write<uint16_t>(file, sound.nBlockAlign);
    _write<uint16_t>(file, sound.wBitsPerSample);

    file.write("data", 4);
    _write<uint32_t>(file, sound.dataSize);
    file.write(reinterpret_cast<const char*>(sound.data.data()), sound.dataSize);

    return 0;
}

int loadFromXnb(const std::string& filename, soundEffect& sound)
{
    header header;
    typeReaders readers;
    std::stringstream data;

    int ret = _quickRead(filename, &header, &readers, &data);

    if (ret != 0)
    {
        return ret;
    }

    _read<char>(data); //type reader

    sound.formatSize = _read<uint32_t>(data);
    sound.wFormatTag = _read<uint16_t>(data);
    sound.nChannels = _read<uint16_t>(data);
    sound.nSamplesPerSec = _read<uint32_t>(data);
    sound.nAvgBytesPerSec = _read<uint32_t>(data);
    sound.nBlockAlign = _read<uint16_t>(data);
    sound.wBitsPerSample = _read<uint16_t>(data);
    sound.cbSize = _read<uint16_t>(data);

    sound.dataSize = _read<uint32_t>(data);
    sound.data.resize(sound.dataSize);
    for (uint32_t i = 0; i < sound.dataSize; i++)
    {
        sound.data[i] = _read<char>(data);
    }

    sound.loopStart = _read<uint32_t>(data);
    sound.loopLength = _read<uint32_t>(data);
    sound.duration = _read<uint32_t>(data);

    return 0;
}

int writeToXnb(const std::string& filename, const soundEffect& sound, bool compress)
{
    std::ofstream file(filename, std::ios::binary);

    _write<char>(file, 'X');
    _write<char>(file, 'N');
    _write<char>(file, 'B');
    _write<char>(file, 'w');
    _write<char>(file, 5);

    std::stringstream data;
    _write7BitEncodedInt(data, 1); //type reader count
    std::string typeReaderName = "Microsoft.Xna.Framework.Content.SoundEffectReader";
    _writeString(data, typeReaderName);
    _write<int32_t>(data, 0); //version
    _write7BitEncodedInt(data, 0); //shared resource count

    //texture2D specific
    _write<char>(data, 1); //typeReaderIndex

    _write<uint32_t>(data, sound.formatSize);
    _write<uint16_t>(data, sound.wFormatTag);
    _write<uint16_t>(data, sound.nChannels);
    _write<uint32_t>(data, sound.nSamplesPerSec);
    _write<uint32_t>(data, sound.nAvgBytesPerSec);
    _write<uint16_t>(data, sound.nBlockAlign);
    _write<uint16_t>(data, sound.wBitsPerSample);
    _write<uint16_t>(data, sound.cbSize);

    _write<uint32_t>(data, sound.dataSize);
    for (uint32_t i = 0; i < sound.dataSize; i++)
    {
        _write<char>(data, sound.data[i]);
    }
    _write<uint32_t>(data, sound.loopStart);
    _write<uint32_t>(data, sound.loopLength);
    _write<uint32_t>(data, sound.duration);

    _writeXnbRest(file, data, compress);

    return 0;
}


//helpers ----------------------------------------------------------------------


int _readXnbHeader(std::istream& stream, header& header)
{
    /*header = _read<xnb::header>(stream);*/
    header.formatIdentifier[0] = _read<char>(stream);
    header.formatIdentifier[1] = _read<char>(stream);
    header.formatIdentifier[2] = _read<char>(stream);
    header.platform = _read<char>(stream);
    header.version = _read<char>(stream);
    header.flagBits = _read<char>(stream);
    header.fileSize = _read<uint32_t>(stream);
    return 0;
}

int _handleXnbCompression(std::istream& stream, std::stringstream* decompressedStream)
{
    char x = _read<char>(stream); //X
    char n = _read<char>(stream); //N
    char b = _read<char>(stream); //B
    char platform = _read<char>(stream); //w
    char version = _read<char>(stream); //5
    char flags = _read<char>(stream);

    if (flags == 0x00)
    {
        stream.seekg(0, std::ios::beg);
        *decompressedStream << stream.rdbuf();
        return 0;
    }

    if (flags & 0x40)
    {
        printf("LZ4 compression not supported\n");
        stream.seekg(0, std::ios::beg);
        return -1;
    }

    if (!(flags & 0x80))
    {
        printf("Unknown compression type\n");
        stream.seekg(0, std::ios::beg);
        return -1;
    }

    //decompress
    uint32_t fileSize = _read<uint32_t>(stream);
    uint32_t uncompressedDataSize = _read<uint32_t>(stream);

    char* decompressedData = new char[uncompressedDataSize];
    uint32_t curr = stream.tellg();
    stream.seekg(0, std::ios::end);
    uint32_t size = stream.tellg();
    stream.seekg(curr, std::ios::beg);
    lzx::decompress(&stream, decompressedData, uncompressedDataSize, size);

    _write<char>(*decompressedStream, x);
    _write<char>(*decompressedStream, n);
    _write<char>(*decompressedStream, b);
    _write<char>(*decompressedStream, platform);
    _write<char>(*decompressedStream, version);
    flags = 0x00; //uncompressed
    _write<char>(*decompressedStream, flags);
    _write<uint32_t>(*decompressedStream, uncompressedDataSize + 10);
    decompressedStream->write(decompressedData, uncompressedDataSize);

    delete[] decompressedData;

    return 0;
}

int _readXnbReaders(std::istream& stream, typeReaders& readers)
{
    readers.count = _read7BitEncodedInt(stream);
    for (int i = 0; i < readers.count; i++)
    {
        readers.names.push_back(_readString(stream));
        readers.versions.push_back(_read<int32_t>(stream));
    }
    return 0;
}

std::stringstream _readXnbData(std::istream& stream)
{
    char sharedResource = _read<char>(stream);
    std::stringstream data;
    data << stream.rdbuf();

    return std::move(data);
}

int _writeXnbRest(std::ostream& file, std::stringstream& data, bool compress)
{
    uint32_t uncompressedSize = data.tellp();
    uint8_t* uncompressedData = new uint8_t[uncompressedSize];
    data.read(reinterpret_cast<char*>(uncompressedData), uncompressedSize);

    if (compress)
    {
        int32_t compressedSize = 0;
        uint8_t* compressedData = nullptr;

        /*char ret = xcompress::compress(compressedData, compressedSize,*/
                                       /*uncompressedData, uncompressedSize);*/

        char ret = 0;
        xcompress::xcompress functions;
        xcompress::loadDll(&functions);
        if (functions.hModule == NULL)
        {
            ret = 1;
        }

        if (ret == 0)
        {
            xcompress::compress(&functions, compressedData, compressedSize,
                                uncompressedData, uncompressedSize);
        }

        if (compressedData == nullptr)
        {
            ret = 2;
        }

        xcompress::unloadDll(&functions);


        if (ret == 0)
        {
            _write<char>(file, '\x80');
            //header + flags + compressed size + uncompressed size + data
            compressedSize += 5 + 1 + 4 + 4; //total size
            _write<uint32_t>(file, compressedSize);
            _write<uint32_t>(file, uncompressedSize);
            file.write(reinterpret_cast<char*>(compressedData), compressedSize);
            delete[] compressedData;
            delete[] uncompressedData;
            return 0;
        }
    }

    _write<char>(file, 0);
    //header + flags + uncompressed size + data
    _write<uint32_t>(file, uncompressedSize + 5 + 1 + 4);
    file.write(reinterpret_cast<char*>(uncompressedData), uncompressedSize);

    return 0;
}

int _quickRead(const std::string& filename, header* header, typeReaders* readers, std::stringstream* data)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        return 1;
    }

    std::stringstream uncompressed;
    _handleXnbCompression(file, &uncompressed);

    _readXnbHeader(uncompressed, *header);
    _readXnbReaders(uncompressed, *readers);
    *data = _readXnbData(uncompressed);

    return 0;
}

int32_t _read7BitEncodedInt(std::istream& stream)
{
    int32_t value = 0;
    int32_t shift = 0;
    uint8_t byte;
    do
    {
        stream.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        value |= (byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);
    return value;
}

void _write7BitEncodedInt(std::ostream& stream, int32_t value)
{
    do
    {
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (value > 0)
        {
            byte |= 0x80;
        }
        stream.write(reinterpret_cast<char*>(&byte), sizeof(byte));
    } while (value > 0);
    return;
}

std::string _readString(std::istream& stream)
{
    int32_t length = _read7BitEncodedInt(stream);
    std::string value;
    value.resize(length);
    stream.read(&value[0], length);
    return value;
}

void _writeString(std::ostream& stream, const std::string& value)
{
    _write7BitEncodedInt(stream, value.size());
    stream.write(value.c_str(), value.size());
    return;
}

} // namespace xnb
