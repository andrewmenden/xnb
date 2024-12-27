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

#include <lzxDecoder.h>
#include <stdexcept>

LzxDecoder::LzxDecoder (int window) //DONE
{
	uint32_t wndsize = (uint32_t)(1 << window);
	int posn_slots;

	// setup proper exception
	if(window < 15 || window > 21) throw std::runtime_error("Unsupported window size range");

	// let's initialise our state
	/*m_state = new LzxState();*/
	m_state.actual_size = 0;
	m_state.window.resize(wndsize);
	for(int i = 0; i < wndsize; i++) m_state.window[i] = 0xDC;
	m_state.actual_size = wndsize;
	m_state.window_size = wndsize;
	m_state.window_posn = 0;

	/* initialize static tables */
	extra_bits.resize(52);
	for(int i = 0, j = 0; i <= 50; i += 2)
	{
		extra_bits[i] = extra_bits[i+1] = (uint8_t)j;
		if ((i != 0) && (j < 17)) j++;
	}
	position_base.resize(51);
	for(int i = 0, j = 0; i <= 50; i++)
	{
		position_base[i] = (uint32_t)j;
		j += 1 << extra_bits[i];
	}

	/* calculate required position slots */
	if(window == 20) posn_slots = 42;
	else if(window == 21) posn_slots = 50;
	else posn_slots = window << 1;

	m_state.R0 = m_state.R1 = m_state.R2 = 1;
	m_state.main_elements = (uint16_t)(LzxConstants::NUM_CHARS + (posn_slots << 3));
	m_state.header_read = 0;
	m_state.frames_read = 0;
	m_state.block_remaining = 0;
	m_state.block_type = LzxConstants::BLOCKTYPE::INVALID;
	m_state.intel_curpos = 0;
	m_state.intel_started = 0;
	m_state.intel_filesize = 0;

	// yo dawg i herd u liek arrays so we put arrays in ur arrays so u can array while u array
	m_state.PRETREE_table.resize((1 << LzxConstants::PRETREE_TABLEBITS) + (LzxConstants::PRETREE_MAXSYMBOLS << 1));
	m_state.PRETREE_len.resize(LzxConstants::PRETREE_MAXSYMBOLS + LzxConstants::LENTABLE_SAFETY);
	m_state.MAINTREE_table.resize((1 << LzxConstants::MAINTREE_TABLEBITS) + (LzxConstants::MAINTREE_MAXSYMBOLS << 1));
	m_state.MAINTREE_len.resize(LzxConstants::MAINTREE_MAXSYMBOLS + LzxConstants::LENTABLE_SAFETY);
	m_state.LENGTH_table.resize((1 << LzxConstants::LENGTH_TABLEBITS) + (LzxConstants::LENGTH_MAXSYMBOLS << 1));
	m_state.LENGTH_len.resize(LzxConstants::LENGTH_MAXSYMBOLS + LzxConstants::LENTABLE_SAFETY);
	m_state.ALIGNED_table.resize((1 << LzxConstants::ALIGNED_TABLEBITS) + (LzxConstants::ALIGNED_MAXSYMBOLS << 1));
	m_state.ALIGNED_len.resize(LzxConstants::ALIGNED_MAXSYMBOLS + LzxConstants::LENTABLE_SAFETY);
	/* initialise tables to 0 (because deltas will be applied to them) */
	for(int i = 0; i < LzxConstants::MAINTREE_MAXSYMBOLS; i++) m_state.MAINTREE_len[i] = 0;
	for(int i = 0; i < LzxConstants::LENGTH_MAXSYMBOLS; i++) m_state.LENGTH_len[i] = 0;
}

LzxDecoder::~LzxDecoder() 
{
}

int32_t LzxDecoder::decompress(std::istream& inData, int32_t inLen, char*& outData, int32_t outLen)
{
	BitBuffer bitbuf(&inData);
	long startpos = inData.tellg();
	long endpos = (int32_t)inData.tellg() + inLen;

	/*byte[] window = m_state.window;*/
	std::vector<uint8_t>& window = m_state.window;

	uint32_t window_posn = m_state.window_posn;
	uint32_t window_size = m_state.window_size;
	uint32_t R0 = m_state.R0;
	uint32_t R1 = m_state.R1;
	uint32_t R2 = m_state.R2;
	uint32_t i, j;

	int togo = outLen, this_run, main_element, match_length, match_offset, length_footer, extra, verbatim_bits;
	int rundest, runsrc, copy_length, aligned_bits;

	bitbuf.initBitStream();

	/* read header if necessary */
	if(m_state.header_read == 0)
	{
		uint32_t intel = bitbuf.readBits(1);
		if(intel != 0)
		{
			// read the filesize
			i = bitbuf.readBits(16); j = bitbuf.readBits(16);
			m_state.intel_filesize = (int)((i << 16) | j);
		}
		m_state.header_read = 1;
	}

	/* main decoding loop */
	while(togo > 0)
	{
		/* last block finished, new block expected */
		if(m_state.block_remaining == 0)
		{
			// TODO may screw something up here
			if(m_state.block_type == LzxConstants::BLOCKTYPE::UNCOMPRESSED) {
				if((m_state.block_length & 1) == 1) inData.get(); /* realign bitstream to word */
				bitbuf.initBitStream();
			}

			m_state.block_type = (LzxConstants::BLOCKTYPE)bitbuf.readBits(3);;
			i = bitbuf.readBits(16);
			j = bitbuf.readBits(8);
			m_state.block_remaining = m_state.block_length = (uint32_t)((i << 8) | j);

			switch(m_state.block_type)
			{
				case LzxConstants::BLOCKTYPE::ALIGNED:
					for(i = 0, j = 0; i < 8; i++) { j = bitbuf.readBits(3); m_state.ALIGNED_len[i] = (uint8_t)j; }
					makeDecodeTable(LzxConstants::ALIGNED_MAXSYMBOLS, LzxConstants::ALIGNED_TABLEBITS,
										 m_state.ALIGNED_len, m_state.ALIGNED_table);
					/* rest of aligned header is same as verbatim */
				case LzxConstants::BLOCKTYPE::VERBATIM:
					readLengths(m_state.MAINTREE_len, 0, 256, bitbuf);
					readLengths(m_state.MAINTREE_len, 256, m_state.main_elements, bitbuf);
					makeDecodeTable(LzxConstants::MAINTREE_MAXSYMBOLS, LzxConstants::MAINTREE_TABLEBITS,
										 m_state.MAINTREE_len, m_state.MAINTREE_table);
					if(m_state.MAINTREE_len[0xE8] != 0) m_state.intel_started = 1;

					readLengths(m_state.LENGTH_len, 0, LzxConstants::NUM_SECONDARY_LENGTHS, bitbuf);
					makeDecodeTable(LzxConstants::LENGTH_MAXSYMBOLS, LzxConstants::LENGTH_TABLEBITS,
										 m_state.LENGTH_len, m_state.LENGTH_table);
					break;

				case LzxConstants::BLOCKTYPE::UNCOMPRESSED:
					m_state.intel_started = 1; /* because we can't assume otherwise */
					bitbuf.ensureBits(16); /* get up to 16 pad bits into the buffer */
					if(bitbuf.getBitsLeft() > 16) inData.seekg(-2, std::ios_base::cur); /* and align the bitstream! */
					uint8_t hi, mh, ml, lo;
					lo = (uint8_t)inData.get(); ml = (uint8_t)inData.get(); mh = (uint8_t)inData.get(); hi = (uint8_t)inData.get();
					R0 = (uint32_t)(lo | ml << 8 | mh << 16 | hi << 24);
					lo = (uint8_t)inData.get(); ml = (uint8_t)inData.get(); mh = (uint8_t)inData.get(); hi = (uint8_t)inData.get();
					R1 = (uint32_t)(lo | ml << 8 | mh << 16 | hi << 24);
					lo = (uint8_t)inData.get(); ml = (uint8_t)inData.get(); mh = (uint8_t)inData.get(); hi = (uint8_t)inData.get();
					R2 = (uint32_t)(lo | ml << 8 | mh << 16 | hi << 24);
					break;

				default:
					return -1; // TODO throw proper exception
			}
		}

		/* buffer exhaustion check */
		if(inData.tellg() > (startpos + inLen))
		{
			/* it's possible to have a file where the next run is less than
						 * 16 bits in size. In this case, the READ_HUFFSYM() macro used
						 * in building the tables will exhaust the buffer, so we should
						 * allow for this, but not allow those accidentally read bits to
						 * be used (so we check that there are at least 16 bits
						 * remaining - in this boundary case they aren't really part of
						 * the compressed data)
					 */
			//Debug.WriteLine("WTF");

			if(inData.tellg() > (startpos+inLen+2) || bitbuf.getBitsLeft() < 16) return -1; //TODO throw proper exception
		}

		while((this_run = (int)m_state.block_remaining) > 0 && togo > 0)
		{
			if(this_run > togo) this_run = togo;
			togo -= this_run;
			m_state.block_remaining -= (uint32_t)this_run;

			/* apply 2^x-1 mask */
			window_posn &= window_size - 1;
			/* runs can't straddle the window wraparound */
			if((window_posn + this_run) > window_size)
				return -1; //TODO throw proper exception

			switch(m_state.block_type)
			{
				case LzxConstants::BLOCKTYPE::VERBATIM:
					while(this_run > 0)
					{
						main_element = (int)readHuffSym(m_state.MAINTREE_table, m_state.MAINTREE_len,
																			LzxConstants::MAINTREE_MAXSYMBOLS, LzxConstants::MAINTREE_TABLEBITS,
																			bitbuf);
						if(main_element < LzxConstants::NUM_CHARS)
						{
							/* literal: 0 to NUM_CHARS-1 */
							window[window_posn++] = (uint8_t)main_element;
							this_run--;
						}
						else
					{
							/* match: NUM_CHARS + ((slot<<3) | length_header (3 bits)) */
							main_element -= LzxConstants::NUM_CHARS;

							match_length = main_element & LzxConstants::NUM_PRIMARY_LENGTHS;
							if(match_length == LzxConstants::NUM_PRIMARY_LENGTHS)
							{
								length_footer = (int)readHuffSym(m_state.LENGTH_table, m_state.LENGTH_len,
																				 LzxConstants::LENGTH_MAXSYMBOLS, LzxConstants::LENGTH_TABLEBITS,
																				 bitbuf);
								match_length += length_footer;
							}
							match_length += LzxConstants::MIN_MATCH;

							match_offset = main_element >> 3;

							if(match_offset > 2)
							{
								/* not repeated offset */
								if(match_offset != 3)
								{
									extra = extra_bits[match_offset];
									verbatim_bits = (int)bitbuf.readBits((uint8_t)extra);
									match_offset = (int)position_base[match_offset] - 2 + verbatim_bits;
								}
								else
							{
									match_offset = 1;
								}

								/* update repeated offset LRU queue */
								R2 = R1; R1 = R0; R0 = (uint32_t)match_offset;
							}
							else if(match_offset == 0)
							{
								match_offset = (int)R0;
							}
							else if(match_offset == 1)
							{
								match_offset = (int)R1;
								R1 = R0; R0 = (uint32_t)match_offset;
							}
							else /* match_offset == 2 */
						{
								match_offset = (int)R2;
								R2 = R0; R0 = (uint32_t)match_offset;
							}

							rundest = (int)window_posn;
							this_run -= match_length;

							/* copy any wrapped around source data */
							if(window_posn >= match_offset)
							{
								/* no wrap */
								runsrc = rundest - match_offset;
							}
							else
						{
								runsrc = rundest + ((int)window_size - match_offset);
								copy_length = match_offset - (int)window_posn;
								if(copy_length < match_length)
								{
									match_length -= copy_length;
									window_posn += (uint32_t)copy_length;
									while(copy_length-- > 0) window[rundest++] = window[runsrc++];
									runsrc = 0;
								}
							}
							window_posn += (uint32_t)match_length;

							/* copy match data - no worries about destination wraps */
							while(match_length-- > 0) window[rundest++] = window[runsrc++];
						}
					}
					break;

				case LzxConstants::BLOCKTYPE::ALIGNED:
					while(this_run > 0)
					{
						main_element = (int)readHuffSym(m_state.MAINTREE_table, m_state.MAINTREE_len,
																			LzxConstants::MAINTREE_MAXSYMBOLS, LzxConstants::MAINTREE_TABLEBITS,
																			bitbuf);

						if(main_element < LzxConstants::NUM_CHARS)
						{
							/* literal 0 to NUM_CHARS-1 */
							window[window_posn++] = (uint8_t)main_element;
							this_run--;
						}
						else
					{
							/* match: NUM_CHARS + ((slot<<3) | length_header (3 bits)) */
							main_element -= LzxConstants::NUM_CHARS;

							match_length = main_element & LzxConstants::NUM_PRIMARY_LENGTHS;
							if(match_length == LzxConstants::NUM_PRIMARY_LENGTHS)
							{
								length_footer = (int)readHuffSym(m_state.LENGTH_table, m_state.LENGTH_len,
																				 LzxConstants::LENGTH_MAXSYMBOLS, LzxConstants::LENGTH_TABLEBITS,
																				 bitbuf);
								match_length += length_footer;
							}
							match_length += LzxConstants::MIN_MATCH;

							match_offset = main_element >> 3;

							if(match_offset > 2)
							{
								/* not repeated offset */
								extra = extra_bits[match_offset];
								match_offset = (int)position_base[match_offset] - 2;
								if(extra > 3)
								{
									/* verbatim and aligned bits */
									extra -= 3;
									verbatim_bits = (int)bitbuf.readBits((uint8_t)extra);
									match_offset += (verbatim_bits << 3);
									aligned_bits = (int)readHuffSym(m_state.ALIGNED_table, m_state.ALIGNED_len,
																				 LzxConstants::ALIGNED_MAXSYMBOLS, LzxConstants::ALIGNED_TABLEBITS,
																				 bitbuf);
									match_offset += aligned_bits;
								}
								else if(extra == 3)
								{
									/* aligned bits only */
									aligned_bits = (int)readHuffSym(m_state.ALIGNED_table, m_state.ALIGNED_len,
																				 LzxConstants::ALIGNED_MAXSYMBOLS, LzxConstants::ALIGNED_TABLEBITS,
																				 bitbuf);
									match_offset += aligned_bits;
								}
								else if (extra > 0) /* extra==1, extra==2 */
								{
									/* verbatim bits only */
									verbatim_bits = (int)bitbuf.readBits((uint8_t)extra);
									match_offset += verbatim_bits;
								}
								else /* extra == 0 */
							{
									/* ??? */
									match_offset = 1;
								}

								/* update repeated offset LRU queue */
								R2 = R1; R1 = R0; R0 = (uint32_t)match_offset;
							}
							else if( match_offset == 0)
							{
								match_offset = (int)R0;
							}
							else if(match_offset == 1)
							{
								match_offset = (int)R1;
								R1 = R0; R0 = (uint32_t)match_offset;
							}
							else /* match_offset == 2 */
						{
								match_offset = (int)R2;
								R2 = R0; R0 = (uint32_t)match_offset;
							}

							rundest = (int)window_posn;
							this_run -= match_length;

							/* copy any wrapped around source data */
							if(window_posn >= match_offset)
							{
								/* no wrap */
								runsrc = rundest - match_offset;
							}
							else
						{
								runsrc = rundest + ((int)window_size - match_offset);
								copy_length = match_offset - (int)window_posn;
								if(copy_length < match_length)
								{
									match_length -= copy_length;
									window_posn += (uint32_t)copy_length;
									while(copy_length-- > 0) window[rundest++] = window[runsrc++];
									runsrc = 0;
								}
							}
							window_posn += (uint32_t)match_length;

							/* copy match data - no worries about destination wraps */
							while(match_length-- > 0) window[rundest++] = window[runsrc++];
						}
					}
					break;

				case LzxConstants::BLOCKTYPE::UNCOMPRESSED:
					{
						if(((int)inData.tellg() + this_run) > endpos) return -1; //TODO throw proper exception
						/*byte[] temp_buffer = new byte[this_run];*/
						/*inData.Read(temp_buffer, 0, this_run);*/
						/*temp_buffer.CopyTo(window, (int)window_posn);*/
						/*window_posn += (uint)this_run;*/
						std::vector<uint8_t> temp_buffer(this_run);
						inData.read((char*)&temp_buffer[0], this_run);
						memcpy(&window[window_posn], &temp_buffer[0], this_run);
						window_posn += (uint32_t)this_run;
					}
					break;
				default:
					return -1; //TODO throw proper exception
			}
		}
	}

	if(togo != 0) return -1; //TODO throw proper exception
	int start_window_pos = (int)window_posn;
	if(start_window_pos == 0) start_window_pos = (int)window_size;
	start_window_pos -= outLen;
	/*outData.Write(window, start_window_pos, outLen);*/
	/*outData.write((char*)&window[start_window_pos], outLen);*/
	memcpy(outData, &window[start_window_pos], outLen);
	//print 100 bytes of outData
	outData += outLen;

	m_state.window_posn = window_posn;
	m_state.R0 = R0;
	m_state.R1 = R1;
	m_state.R2 = R2;

	// being so fr, i don't get the below code-- you read from the outData?
	// i dont think the data was initialized to anything in the first place,
	// so that makes no sense. i'll just return 0 for now
	if((m_state.frames_read++ < 32768) && m_state.intel_filesize != 0)
	{
		throw std::runtime_error("Intel E8 decoding not implemented");
	}
	return 0;

	// TODO finish intel E8 decoding
	/* intel E8 decoding */
	/*if((m_state.frames_read++ < 32768) && m_state.intel_filesize != 0)*/
	/*{*/
	/*	if(outLen <= 6 || m_state.intel_started == 0)*/
	/*	{*/
	/*		m_state.intel_curpos += outLen;*/
	/*	}*/
	/*	else*/
	/*	{*/
	/*		int dataend = outLen - 10;*/
	/*		uint32_t curpos = (uint32_t)m_state.intel_curpos;*/
	/**/
	/*		m_state.intel_curpos = (int)curpos + outLen;*/
	/**/
	/*		while(outData.tellp() < dataend)*/
	/*		{*/
					/*if(outData.ReadByte() != 0xE8) { curpos++; continue; }*/
	/*		}*/
	/*	}*/
	/*	return -1;*/
	/*}*/
	/*return 0;*/
}

// TODO make returns throw exceptions

int32_t LzxDecoder::makeDecodeTable(uint32_t nsyms, uint32_t nbits, std::vector<uint8_t>& length, std::vector<uint16_t>& table)
{
	uint16_t sym;
	uint32_t leaf;
	uint8_t bit_num = 1;
	uint32_t fill;
	uint32_t pos			= 0; /* the current position in the decode table */
	uint32_t table_mask		= (uint32_t)(1 << (int)nbits);
	uint32_t bit_mask		= table_mask >> 1; /* don't do 0 length codes */
	uint32_t next_symbol	= bit_mask;	/* base of allocation for long codes */

	/* fill entries for codes short enough for a direct mapping */
	while (bit_num <= nbits )
	{
		for(sym = 0; sym < nsyms; sym++)
		{
			if(length[sym] == bit_num)
			{
				leaf = pos;

				if((pos += bit_mask) > table_mask) return 1; /* table overrun */

				/* fill all possible lookups of this symbol with the symbol itself */
				fill = bit_mask;
				while(fill-- > 0) table[leaf++] = sym;
			}
		}
		bit_mask >>= 1;
		bit_num++;
	}

	/* if there are any codes longer than nbits */
	if(pos != table_mask)
	{
		/* clear the remainder of the table */
		for(sym = (uint16_t)pos; sym < table_mask; sym++) table[sym] = 0;

		/* give ourselves room for codes to grow by up to 16 more bits */
		pos <<= 16;
		table_mask <<= 16;
		bit_mask = 1 << 15;

		while(bit_num <= 16)
		{
			for(sym = 0; sym < nsyms; sym++)
			{
				if(length[sym] == bit_num)
				{
					leaf = pos >> 16;
					for(fill = 0; fill < bit_num - nbits; fill++)
					{
						/* if this path hasn't been taken yet, 'allocate' two entries */
						if(table[leaf] == 0)
						{
							table[(next_symbol << 1)] = 0;
							table[(next_symbol << 1) + 1] = 0;
							table[leaf] = (uint16_t)(next_symbol++);
						}
						/* follow the path and select either left or right for next bit */
						leaf = (uint16_t)(table[leaf] << 1);
						if(((pos >> (int)(15-fill)) & 1) == 1) leaf++;
					}
					table[leaf] = sym;

					if((pos += bit_mask) > table_mask) return 1;
				}
			}
			bit_mask >>= 1;
			bit_num++;
		}
	}

	/* full talbe? */
	if(pos == table_mask) return 0;

	/* either erroneous table, or all elements are 0 - let's find out. */
	for(sym = 0; sym < nsyms; sym++) if(length[sym] != 0) return 1;
	return 0;
}

// TODO throw exceptions instead of returns
void LzxDecoder::readLengths(std::vector<uint8_t>& lens, uint32_t first, uint32_t last, BitBuffer& bitbuf)
{
	uint32_t x, y;
	int z;

	// hufftbl pointer here?

	for(x = 0; x < 20; x++)
	{
		y = bitbuf.readBits(4);
		m_state.PRETREE_len[x] = (uint8_t)y;
	}
	makeDecodeTable(LzxConstants::PRETREE_MAXSYMBOLS, LzxConstants::PRETREE_TABLEBITS,
								 m_state.PRETREE_len, m_state.PRETREE_table);

	for(x = first; x < last;)
	{
		z = (int)readHuffSym(m_state.PRETREE_table, m_state.PRETREE_len,
											 LzxConstants::PRETREE_MAXSYMBOLS, LzxConstants::PRETREE_TABLEBITS, bitbuf);
		if(z == 17)
		{
			y = bitbuf.readBits(4); y += 4;
			while(y-- != 0) lens[x++] = 0;
		}
		else if(z == 18)
		{
			y = bitbuf.readBits(5); y += 20;
			while(y-- != 0) lens[x++] = 0;
		}
		else if(z == 19)
		{
			y = bitbuf.readBits(1); y += 4;
			z = (int)readHuffSym(m_state.PRETREE_table, m_state.PRETREE_len,
												LzxConstants::PRETREE_MAXSYMBOLS, LzxConstants::PRETREE_TABLEBITS, bitbuf);
			z = lens[x] - z; if(z < 0) z += 17;
			while(y-- != 0) lens[x++] = (uint8_t)z;
		}
		else
	{
			z = lens[x] - z; if(z < 0) z += 17;
			lens[x++] = (uint8_t)z;
		}
	}
}

uint32_t LzxDecoder::readHuffSym(std::vector<uint16_t>& table, std::vector<uint8_t>& lengths, uint32_t nsyms, uint32_t nbits, BitBuffer& bitbuf)
{
	uint32_t i, j;
	bitbuf.ensureBits(16);
	if((i = table[bitbuf.peekBits((uint8_t)nbits)]) >= nsyms)
	{
		j = (uint32_t)(1 << (int)((sizeof(uint32_t)*8) - nbits));
		do
		{
			j >>= 1; i <<= 1; i |= (bitbuf.getBuffer() & j) != 0 ? (uint32_t)1 : 0;
			if(j == 0) return 0; // TODO throw proper exception
		} while((i = table[i]) >= nsyms);
	}
	j = lengths[i];
	bitbuf.removeBits((uint8_t)j);

	return i;
}

BitBuffer::BitBuffer(std::istream* stream)
{
	byteStream = stream;
	initBitStream();
}

void BitBuffer::initBitStream()
{
	buffer = 0;
	bitsleft = 0;
}

void BitBuffer::ensureBits(uint8_t bits)
{
	while(bitsleft < bits) {
		int lo = (uint8_t)byteStream->get();
		int hi = (uint8_t)byteStream->get();
		//int amount2shift = sizeof(uint)*8 - 16 - bitsleft;
		buffer |= (uint32_t)(((hi << 8) | lo) << (sizeof(uint32_t)*8 - 16 - bitsleft));
		bitsleft += 16;
	}
}

uint32_t BitBuffer::peekBits(uint8_t bits)
{
	return (buffer >> ((sizeof(uint32_t)*8) - bits));
}

void BitBuffer::removeBits(uint8_t bits)
{
	buffer <<= bits;
	bitsleft -= bits;
}

uint32_t BitBuffer::readBits(uint8_t bits)
{
	uint32_t ret = 0;

	if(bits > 0)
	{
		ensureBits(bits);
		ret = peekBits(bits);
		removeBits(bits);
	}

	return ret;
}

uint32_t BitBuffer::getBuffer()
{
	return buffer;
}

uint8_t BitBuffer::getBitsLeft()
{
	return bitsleft;
}


namespace lzx
{

void decompress(std::istream* stream, char* decompressed, int32_t decompressedSize, int32_t compressedSize)
{
	LzxDecoder decoder;

	if (decompressed == nullptr) decompressed = new char[decompressedSize];
	char* curr = decompressed;

	uint32_t startPos = stream->tellg();
	uint32_t pos = startPos;

	while (pos - startPos < compressedSize)
	{
		// the compressed stream is seperated into blocks that will decompress
		// into 32Kb or some other size if specified.
		// normal, 32Kb output blocks will have a short indicating the size
		// of the block before the block starts
		// blocks that have a defined output will be preceded by a byte of value
		// 0xFF (255), then a short indicating the output size and another
		// for the block size
		// all shorts for these cases are encoded in big endian order
		int hi = stream->get();
		int lo = stream->get();
		int block_size = (hi << 8) | lo;
		int frame_size = 0x8000; // frame size is 32Kb by default
		// does this block define a frame size?
		if (hi == 0xFF)
		{
			hi = lo;
			lo = (uint8_t)stream->get();
			frame_size = (hi << 8) | lo;
			hi = (uint8_t)stream->get();
			lo = (uint8_t)stream->get();
			block_size = (hi << 8) | lo;
			pos += 5;
		}
		else
		{
			pos += 2;
		}

		// either says there is nothing to decode
		if (block_size == 0 || frame_size == 0)
			break;

		decoder.decompress(*stream, block_size, curr, frame_size);
		pos += block_size;

		// reset the position of the input just incase the bit buffer
		// read in some unused bytes
		stream->seekg(pos, std::ios_base::beg);
	}

	/*if (decompressedStream.Position != decompressedSize)*/
	/*{*/
	/*	throw new ContentLoadException("Decompression failed.");*/
	/*}*/
}

}
