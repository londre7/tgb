#ifndef _ATBIT_V2_H_
#define _ATBIT_V2_H_

#include <stdint.h>

template <typename T>
void SetBit(T& mask, size_t index)
{
	T temp = T(1) << index;
	mask = mask | temp;
}
template <typename T>
void UnsetBit(T& mask, size_t index)
{
	T temp = T(1) << index;
	temp = ~temp;
	mask = mask & temp;
}
template <typename T>
bool GetBit(T mask, size_t index)
{
	T Temp = T(1) << index;
	Temp = mask & Temp;
	Temp = Temp >> index;
	return Temp;
}

template <typename T>
SMAnsiString DecToBin(T Value)
{
	const size_t len = sizeof(T) * 8;
	char *Buffer = new char[len+1];
	Buffer[len] = '\0';

	for (size_t i=0, j=len-1; i<len; i++, j--)
	{
		Buffer[j] = GetBit<T>(Value, i) ? '1' : '0';
	}

	return SMAnsiString(Buffer, len+1);
}

#define SetBit8    SetBit<uint8_t>
#define UnsetBit8  UnsetBit<uint8_t>
#define GetBit8    GetBit<uint8_t>
#define SetBit16   SetBit<uint16_t>
#define UnsetBit16 UnsetBit<uint16_t>
#define GetBit16   GetBit<uint16_t>
#define SetBit32   SetBit<uint32_t>
#define UnsetBit32 UnsetBit<uint32_t>
#define GetBit32   GetBit<uint32_t>
#define SetBit64   SetBit<uint64_t>
#define UnsetBit64 UnsetBit<uint64_t>
#define GetBit64   GetBit<uint64_t>

#endif