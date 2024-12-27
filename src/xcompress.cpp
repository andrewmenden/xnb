#include <xcompress.h>
#include <cstdio>

namespace xcompress
{

uint8_t loadDll(xcompress* xcompress)
{
    xcompress->hModule = LoadLibraryA("xcompress32.dll");
    if (xcompress->hModule == NULL)
    {
        return 1;
    }
    HMODULE& hModule = xcompress->hModule;

    xcompress->XMemCompress = (XMemCompress_t)GetProcAddress(hModule, "XMemCompress");
    xcompress->XMemCreateCompressionContext = (XMemCreateCompressionContext_t)GetProcAddress(hModule, "XMemCreateCompressionContext");
    xcompress->XMemDestroyCompressionContext = (XMemDestroyCompressionContext_t)GetProcAddress(hModule, "XMemDestroyCompressionContext");
    xcompress->XMemDecompress = (XMemDecompress_t)GetProcAddress(hModule, "XMemDecompress");
    xcompress->XMemCreateDecompressionContext = (XMemCreateDecompressionContext_t)GetProcAddress(hModule, "XMemCreateDecompressionContext");
    xcompress->XMemDestroyDecompressionContext = (XMemDestroyDecompressionContext_t)GetProcAddress(hModule, "XMemDestroyDecompressionContext");

    if (xcompress->XMemCompress == NULL ||
        xcompress->XMemCreateCompressionContext == NULL ||
        xcompress->XMemDestroyCompressionContext == NULL ||
        xcompress->XMemDecompress == NULL ||
        xcompress->XMemCreateDecompressionContext == NULL ||
        xcompress->XMemDestroyDecompressionContext == NULL)
    {
        uint8_t which = 0;
        which |= xcompress->XMemCompress == NULL ? 1 : 0;
        which |= xcompress->XMemCreateCompressionContext == NULL ? 2 : 0;
        which |= xcompress->XMemDestroyCompressionContext == NULL ? 4 : 0;
        which |= xcompress->XMemDecompress == NULL ? 8 : 0;
        which |= xcompress->XMemCreateDecompressionContext == NULL ? 16 : 0;
        which |= xcompress->XMemDestroyDecompressionContext == NULL ? 32 : 0;
        return 2;
    }

    return 0;
}

void unloadDll(xcompress* xcompress)
{
    FreeLibrary((HMODULE)xcompress->hModule);
}

void compress(xcompress* xcompress,
    uint8_t*& compressedData, int32_t& compressedSize,
    uint8_t* data, int32_t dataSize)
{
    int compressionContext = 0;

    XMEMCODEC_PARAMETERS_LZX codecParams;
    codecParams.Flags = 0;
    codecParams.WindowSize = 64 * 1024;
    codecParams.CompressionPartitionSize = 256 * 1024;

    xcompress->XMemCreateCompressionContext(
        XMEMCODEC_TYPE::XMEMCODEC_LZX,
        codecParams, 0, compressionContext);
    if (compressedData != nullptr)
    {
        delete[] compressedData;
        compressedData = nullptr;
    }
    compressedData = new uint8_t[dataSize * 2];
    compressedSize = dataSize * 2;
    xcompress->XMemCompress(compressionContext,
                 compressedData, compressedSize,
                 data, dataSize);
    xcompress->XMemDestroyCompressionContext(compressionContext);
}

int compress(
    uint8_t*& compressedData, int32_t& compressedSize,
    uint8_t* data, int32_t dataSize)
{
    xcompress xcompress;
    loadDll(&xcompress);
    if (xcompress.hModule == NULL)
    {
        return 1;
    }

    compress(&xcompress, compressedData, compressedSize, data, dataSize);

    if (compressedData == nullptr)
    {
        return 2;
    }

    unloadDll(&xcompress);
    return 0;
}

}
