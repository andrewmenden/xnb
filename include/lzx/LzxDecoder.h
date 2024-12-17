/*
 * C++ port of the below
*/
/* This file was derived from libmspack
 * (C) 2003-2004 Stuart Caie.
 * (C) 2011 Ali Scissons.
 *
 * The LZX method was created by Jonathan Forbes and Tomi Poutanen, adapted
 * by Microsoft Corporation.
 *
 * This source file is Dual licensed; meaning the end-user of this source file
 * may redistribute/modify it under the LGPL 2.1 or MS-PL licenses.
 */ 
/* GNU LESSER GENERAL PUBLIC LICENSE version 2.1
 * LzxDecoder is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1 
 */
/* 
 * MICROSOFT PUBLIC LICENSE
 * This source code is subject to the terms of the Microsoft Public License (Ms-PL). 
 *  
 * Redistribution and use in source and binary forms, with or without modification, 
 * is permitted provided that redistributions of the source code retain the above 
 * copyright notices and this file header. 
 *  
 * Additional copyright notices should be appended to the list above. 
 * 
 * For details, see <http://www.opensource.org/licenses/ms-pl.html>. 
 */
/*
 * This derived work is recognized by Stuart Caie and is authorized to adapt
 * any changes made to lzxd.c in his libmspack library and will still retain
 * this dual licensing scheme. Big thanks to Stuart Caie!
 * 
 * DETAILS
 * This file is a pure C# port of the lzxd.c file from libmspack, with minor
 * changes towards the decompression of XNB files. The original decompression
 * software of LZX encoded data was written by Suart Caie in his
 * libmspack/cabextract projects, which can be located at 
 * http://http://www.cabextract.org.uk/
 */


#pragma once
#include <cstdint>
#include <vector>
#include <istream>

namespace LzxConstants
{
const uint16_t MIN_MATCH =				2;
const uint16_t MAX_MATCH =				257;
const uint16_t NUM_CHARS =				256;
enum class BLOCKTYPE
{
	INVALID = 0,
	VERBATIM = 1,
	ALIGNED = 2,
	UNCOMPRESSED = 3
};
const uint16_t PRETREE_NUM_ELEMENTS =	20;
const uint16_t ALIGNED_NUM_ELEMENTS =	8;
const uint16_t NUM_PRIMARY_LENGTHS =	7;
const uint16_t NUM_SECONDARY_LENGTHS = 249;

const uint16_t PRETREE_MAXSYMBOLS = 	PRETREE_NUM_ELEMENTS;
const uint16_t PRETREE_TABLEBITS =		6;
const uint16_t MAINTREE_MAXSYMBOLS = 	NUM_CHARS + 50*8;
const uint16_t MAINTREE_TABLEBITS = 	12;
const uint16_t LENGTH_MAXSYMBOLS = 	NUM_SECONDARY_LENGTHS + 1;
const uint16_t LENGTH_TABLEBITS =		12;
const uint16_t ALIGNED_MAXSYMBOLS = 	ALIGNED_NUM_ELEMENTS;
const uint16_t ALIGNED_TABLEBITS = 	7;

const uint16_t LENTABLE_SAFETY =		64;
}

struct LzxState
{
	uint32_t R0, R1, R2;
	uint16_t main_elements;
	int32_t header_read;
	LzxConstants::BLOCKTYPE	block_type;
	uint32_t block_length;
	uint32_t block_remaining;
	uint32_t frames_read;
	int32_t	intel_filesize;
	int32_t	intel_curpos;
	int32_t	intel_started;		/* have we seen any translateable data yet?	*/

	std::vector<uint16_t>	PRETREE_table;
	std::vector<uint8_t> PRETREE_len;
	std::vector<uint16_t>	MAINTREE_table;
	std::vector<uint8_t> MAINTREE_len;
	std::vector<uint16_t>	LENGTH_table;
	std::vector<uint8_t> LENGTH_len;
	std::vector<uint16_t> ALIGNED_table;
	std::vector<uint8_t> ALIGNED_len;

	// NEEDED MEMBERS
	// CAB actualsize
	// CAB window
	// CAB window_size
	// CAB window_posn
	uint32_t actual_size;
	std::vector<uint8_t> window;
	uint32_t window_size;
	uint32_t window_posn;
};

class BitBuffer;

class LzxDecoder
{
public:
	std::vector<uint32_t> position_base;
	std::vector<uint8_t> extra_bits;
private:
	LzxState m_state;

public:
	LzxDecoder(int32_t window = 16);
	~LzxDecoder();

	int32_t decompress(std::istream& inData, int32_t inLen, char*& outData, int32_t outLen);
private:
	int32_t makeDecodeTable(uint32_t nsyms, uint32_t nbits, std::vector<uint8_t>& length, std::vector<uint16_t>& table);
	void readLengths(std::vector<uint8_t>& lens, uint32_t first, uint32_t last, BitBuffer& bitbuf);
	uint32_t readHuffSym(std::vector<uint16_t>& table, std::vector<uint8_t>& lengths, uint32_t nsyms, uint32_t nbits, BitBuffer& bitbuf);
};

class BitBuffer
{
private:
	uint32_t buffer;
	uint8_t bitsleft;
	std::istream* byteStream;
public:
	BitBuffer(std::istream* stream);

	void initBitStream();
	void ensureBits(uint8_t bits);
	uint32_t peekBits(uint8_t bits);
	void removeBits(uint8_t bits);
	uint32_t readBits(uint8_t bits);
	uint32_t getBuffer();
	uint8_t getBitsLeft();
};


namespace lzx
{

void decompress(std::istream* stream, char* decompressed, int32_t decompressedSize, int32_t compressedSize);

}
