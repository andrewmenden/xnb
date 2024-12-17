#pragma once

#include <cstdint>

namespace xnb
{

enum class origin
{
	begin,
	current,
	end
};

enum class endianness
{
	little,
	big
};

class bufferReader
{
public:
	bufferReader(char* buffer, uint32_t bufferSize, endianness endianness = endianness::little);
	bufferReader(endianness endianness = endianness::little);
	void set(char* buffer, uint32_t bufferSize, endianness endianness = endianness::little);

	//size ------------------------------------------
	uint32_t size();

	//tell ------------------------------------------
	uint32_t tell();
	uint32_t position();

	//seeking ---------------------------------------
	void seek(uint32_t position, origin origin = origin::begin);

	//peeking ---------------------------------------
	void peak7BitEncodedInt(int32_t* value);

	void peak(char* buffer, uint8_t length);
	void peakLE(char* buffer, uint8_t length);
	void peakBE(char* buffer, uint8_t length);

	template<typename T>
	inline void peak(T* value) { peak(reinterpret_cast<char*>(value), sizeof(T)); }

	template<typename T>
	inline void peakLE(T* value) { peakLE(reinterpret_cast<char*>(value), sizeof(T)); }

	template<typename T>
	inline void peakBE(T* value) { peakBE(reinterpret_cast<char*>(value), sizeof(T)); }

	//reading ---------------------------------------
	void read7BitEncodedInt(int32_t* value);
	
	void read(char* buffer, uint8_t length);
	void readLE(char* buffer, uint8_t length);
	void readBE(char* buffer, uint8_t length);

	template<typename T>
	inline void read(T* value) { read(reinterpret_cast<char*>(value), sizeof(T)); }

	template<typename T>
	inline void readLE(T* value) { readLE(reinterpret_cast<char*>(value), sizeof(T)); }

	template<typename T>
	inline void readBE(T* value) { readBE(reinterpret_cast<char*>(value), sizeof(T)); }
private:
	void validate(uint8_t length);
private:
	char* m_buffer = nullptr;
	uint32_t m_bufferSize = 0;
	uint32_t m_bufferPosition = 0;

	endianness m_endianness = endianness::little;
};

}
