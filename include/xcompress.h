#pragma once

#include <windows.h>
#include <cstdint>

enum XMEMCODEC_TYPE
{
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
};

struct XMEMCODEC_PARAMETERS_LZX
{
    int32_t Flags;
    int32_t WindowSize;
    int32_t CompressionPartitionSize;
};


typedef int32_t (__stdcall *XMemCompress_t)(int32_t context,
    uint8_t* dest, int32_t& destSize,
    uint8_t* src, int32_t srcSize);


typedef int32_t (__stdcall *XMemCreateCompressionContext_t)(XMEMCODEC_TYPE codecType,
    XMEMCODEC_PARAMETERS_LZX& codecParams,
    int32_t flags, int32_t& context);

typedef void (__stdcall *XMemDestroyCompressionContext_t)(int32_t context);

typedef int32_t (__stdcall *XMemDecompress_t)(int32_t context,
    uint8_t*& dest, int32_t& destSize,
    uint8_t* src, int32_t srcSize);

typedef int32_t (__stdcall *XMemCreateDecompressionContext_t)(XMEMCODEC_TYPE codecType,
    XMEMCODEC_PARAMETERS_LZX& codecParams,
    int32_t flags, int32_t& context);

typedef void (__stdcall *XMemDestroyDecompressionContext_t)(int32_t context);

namespace xcompress
{

struct xcompress
{
    HMODULE hModule;

    XMemCompress_t XMemCompress;
    XMemCreateCompressionContext_t XMemCreateCompressionContext;
    XMemDestroyCompressionContext_t XMemDestroyCompressionContext;
    XMemDecompress_t XMemDecompress;
    XMemCreateDecompressionContext_t XMemCreateDecompressionContext;
    XMemDestroyDecompressionContext_t XMemDestroyDecompressionContext;
};

uint8_t loadDll(xcompress* xcompress);
void unloadDll(xcompress* xcompress);

void compress(xcompress* xcompress,
    uint8_t*& compressedData, int32_t& compressedSize,
    uint8_t* data, int32_t dataSize);

int compress(
    uint8_t*& compressedData, int32_t& compressedSize,
    uint8_t* data, int32_t dataSize);

}
