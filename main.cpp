#include <xnb.h>
#include <cstring>

std::string getExtension(const char* path)
{
    std::string ext;
    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '.')
        {
            ext = path + i + 1;
            break;
        }
    }
    return ext;
}

std::string stripTypeReader(const std::string& typeReader)
{
    std::string stripped;
    for (int i = 0; i < typeReader.size(); i++)
    {
        if (typeReader[i] == ',')
        {
            break;
        }
        stripped += typeReader[i];
    }
    return stripped;
}

int determineFileType(const char* path, bool input = true)
{
    std::string ext = getExtension(path);
    if (ext == "png")
    {
        return 0;
    }
    if (ext == "wav")
    {
        return 1;
    }
    if (ext == "json")
    {
        return 2;
    }
    int ret = 0;
    if (ext == "xnb")
    {
        ret |= 0b100;
        if (!input)
        {
            return ret;
        }
        xnb::header header;
        xnb::typeReaders typeReaders;
        std::stringstream data;

        xnb::_quickRead(path, &header, &typeReaders, &data);

        std::string reader = stripTypeReader(typeReaders.names[0]);

        if (reader == "Microsoft.Xna.Framework.Content.Texture2DReader")
        {
            return ret + 0;
        }
        if (reader == "Microsoft.Xna.Framework.Content.SoundEffectReader")
        {
            return ret + 1;
        }
        if (reader == "TextureAtlasContent.TextureAtlasReader")
        {
            return ret + 2;
        }
    }
    return -1;
}

int main(int argc, char** argv)
{
    if (argc != 3 && argc != 4)
    {
        printf("Usage: %s [-u] <inputFile> <outputFile>\n", argv[0]);
        printf("  -u: Write uncompressed XNB file (default is compressed if you have xcompress32.dll)\n");
        printf("  <inputFile>: The file to convert-- .png, .wav, .json, or .xnb\n");
        printf("  <outputFile>: The file to write to\n");
        return 1;
    }

    bool compressed = true;
    std::string inputFile;
    std::string outputFile;

    if (argc == 3)
    {
        inputFile = argv[1];
        outputFile = argv[2];
    }
    else if (argc == 4)
    {
        if (strcmp(argv[1], "-u") != 0)
        {
            printf("Usage: %s [-u] <inputFile> <outputFile>\n", argv[0]);
            printf("  -u: Write uncompressed XNB file (default is compressed if you have xcompress32.dll)\n");
            printf("  <inputFile>: The file to convert-- .png, .wav, .json, or .xnb\n");
            printf("  <outputFile>: The file to write to\n");
            return 1;
        }
        compressed = false;
        inputFile = argv[2];
        outputFile = argv[3];
    }

    int fileType = determineFileType(inputFile.c_str());
    int type = fileType & 0b11;
    bool xnb = fileType & 0b100;

    //not a big deal to just have all
    xnb::texture2D texture;
    xnb::soundEffect soundEffect;
    xnb::textureAtlas textureAtlas;

    switch (type)
    {
        case 0:
        {
            if (xnb)
            {
                xnb::loadFromXnb(inputFile, texture);
            }
            else
            {
                xnb::loadFromPng(inputFile, texture);
            }
            break;
        }
        case 1:
        {
            if (xnb)
            {
                xnb::loadFromXnb(inputFile, soundEffect);
            }
            else
            {
                xnb::loadFromWav(inputFile, soundEffect);
            }
            break;
        }
        case 2:
        {
            if (xnb)
            {
                xnb::loadFromXnb(inputFile, textureAtlas);
            }
            else
            {
                xnb::loadFromJson(inputFile, textureAtlas);
            }
            break;
        }
        default:
        {
            printf("Error: Unknown file type\n");
            return 1;
        }
    }

    fileType = determineFileType(outputFile.c_str(), false);
    xnb = fileType & 0b100;

    switch (type)
    {
        case 0:
        {
            if (xnb)
            {
                xnb::writeToXnb(outputFile, texture, compressed);
            }
            else
            {
                xnb::writeToPng(outputFile, texture);
            }
            break;
        }
        case 1:
        {
            if (xnb)
            {
                xnb::writeToXnb(outputFile, soundEffect, compressed);
            }
            else
            {
                xnb::writeToWav(outputFile, soundEffect);
            }
            break;
        }
        case 2:
        {
            if (xnb)
            {
                xnb::writeToXnb(outputFile, textureAtlas, compressed);
            }
            else
            {
                xnb::writeToJson(outputFile, textureAtlas);
            }
            break;
        }
        default:
        {
            printf("Error: Unknown file type\n");
            return 1;
        }
    }

    return 0;
}
