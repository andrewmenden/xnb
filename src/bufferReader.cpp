#include <xnb/bufferReader.h>

#include <stdexcept>
#include <cstring>

namespace xnb
{

bufferReader::bufferReader(char* buffer, uint32_t bufferSize, endianness endianness)
{
	set(buffer, bufferSize, endianness);
}

bufferReader::bufferReader(endianness endianness)
{
	set(nullptr, 0, endianness);
}

void bufferReader::set(char* buffer, uint32_t bufferSize, endianness endianness)
{
	m_buffer = buffer;
	m_bufferSize = bufferSize;
	m_bufferPosition = 0;

	m_endianness = endianness;
}

uint32_t bufferReader::size()
{
	return m_bufferSize;
}

uint32_t bufferReader::tell()
{
	return m_bufferPosition;
}

uint32_t bufferReader::position()
{
	return m_bufferPosition;
}

void bufferReader::seek(uint32_t position, origin origin)
{
	switch (origin)
	{
	case origin::begin:
		m_bufferPosition = position;
		break;
	case origin::current:
		m_bufferPosition += position;
		break;
	case origin::end:
		m_bufferPosition = m_bufferSize - position;
		break;
	}
}

void bufferReader::peak7BitEncodedInt(int32_t* value)
{
	uint32_t previousPosition = m_bufferPosition;
	read7BitEncodedInt(value);
	seek(previousPosition);
}

void bufferReader::peak(char* buffer, uint8_t length)
{
	switch (m_endianness)
	{
		case endianness::little:
			peakLE(buffer, length);
			break;
		case endianness::big:
			peakBE(buffer, length);
			break;
	}
}

void bufferReader::peakLE(char* buffer, uint8_t length)
{
	validate(length);
	memcpy(buffer, m_buffer + m_bufferPosition, length);
}

void bufferReader::peakBE(char* buffer, uint8_t length)
{
	validate(length);
	for (uint8_t i = 0; i < length; i++)
	{
		buffer[i] = m_buffer[m_bufferPosition + length - i - 1];
	}
}

//reading
void bufferReader::read7BitEncodedInt(int32_t* value)
{
	*value = 0;
	int32_t shift = 0;
	uint8_t byte;
	do
	{
		byte = m_buffer[m_bufferPosition++];
		*value |= (byte & 0x7F) << shift;
		shift += 7;
	} while (byte & 0x80);
}

void bufferReader::read(char* buffer, uint8_t length)
{
	switch (m_endianness)
	{
		case endianness::little:
			readLE(buffer, length);
			break;
		case endianness::big:
			readBE(buffer, length);
			break;
	}
}

void bufferReader::readLE(char* buffer, uint8_t length)
{
	validate(length);
	memcpy(buffer, m_buffer + m_bufferPosition, length);
	m_bufferPosition += length;
}

void bufferReader::readBE(char* buffer, uint8_t length)
{
	validate(length);
	for (uint8_t i = 0; i < length; i++)
	{
		buffer[i] = m_buffer[m_bufferPosition + length - i - 1];
	}
	m_bufferPosition += length;
}

void bufferReader::validate(uint8_t length)
{
	if (m_bufferPosition + length > m_bufferSize)
	{
		throw std::runtime_error("Buffer overflow");
	}
}

}
