#include "smkeylist.h"
#ifdef _WIN32
#include <cstring>
#else
#include <string.h>
#endif

SMKeyList::SMKeyList(const SMKeyList& list)
{
	alloc(list.allocsize, list.numelem);
	memcpy(data, list.data, numelem*sizeof(uint64_t));
	mempadding = list.mempadding;
}

void SMKeyList::alloc(size_t sz, size_t _numelem) 
{
	data = new uint64_t[sz]; 
	allocsize = sz; 
	numelem = _numelem;
}
void SMKeyList::free()
{ 
	if (data) { delete[] data; data = nullptr; } 
}
void SMKeyList::realloc(size_t newsize)
{
	uint64_t* newdata = new uint64_t[newsize]; 
	memcpy(newdata, data, numelem*sizeof(uint64_t));
	free();
	data = newdata;
	allocsize = newsize;
}

void SMKeyList::AddKey(uint64_t key)
{
	size_t index = 0;
	if (IsExist(key, index))
		return;

	if (numelem >= allocsize)
		realloc(allocsize + mempadding);

	memmove(&data[index+1], &data[index], (numelem-index)*sizeof(uint64_t));
	data[index] = key;
	numelem++;
}
uint64_t SMKeyList::GetKey(size_t index) const
{
	if (index >= numelem)
		index = numelem - 1;
	return data[index];
}

bool SMKeyList::IsExist(uint64_t key) const
{
	size_t i1, i2, i;
	uint64_t l1;
	const uint64_t *__data = data;

	if (!numelem) { return false; }
	if (numelem == 1) { return (key == __data[0]); }
	if (numelem == 2) { return ((key == __data[0]) || (key == __data[1])); }

	l1 = __data[numelem - 1];
	if (key > l1) return false;
	if (key == l1) return true;

	l1 = __data[0];
	if (key < l1) return false;
	if (key == l1) return true;

	i1 = 0;
	i2 = numelem - 1;
	i = (i1 + i2) >> 1;
	while (i > i1)
	{
		l1 = __data[i];
		if (l1 == key) return true;
		if (l1 < key) i1 = i;
		else i2 = i;
		i = (i1 + i2) >> 1;
	}
	return false;
}

bool SMKeyList::IsExist(uint64_t key, size_t &index) const
{
	size_t i1, i2, i;
	uint64_t l1;
	uint64_t* __data = data;

	if (!numelem) { index = 0; return false; }

	l1 = __data[numelem - 1];
	if (key > l1) { index = numelem; return false; }
	if (key == l1) { index = numelem-1; return true; }

	l1 = __data[0];
	if (key < l1) {  index = 0; return false; }
	if (key == l1) { index = 0; return true; }

	i1 = 0;
	i2 = numelem - 1;
	i = (i1 + i2) >> 1;
	while (i > i1)
	{
		l1 = __data[i];
		if (l1 == key) { index = i; return true; }
		if (l1 < key) i1 = i;
		else i2 = i;
		i = (i1 + i2) >> 1;
	}
	if (i1 != i2) index = i2;
	else index = i1 + 1;
	return false;
}

uint64_t SMKeyList::RemoveKey(uint64_t key)
{
	size_t index;
	return (IsExist(key, index)) ? RemoveKeyByIndex(index) : 0ULL;
}

uint64_t SMKeyList::RemoveKeyByIndex(size_t index)
{
	if (index >= numelem)
		return 0ULL;

	uint64_t _key = data[index];
	memmove(&data[index], &data[index+1], (numelem-index+1)*sizeof(uint64_t));
	numelem--;
	return _key;
}

void SMUnsortedKeyList::AddKey(uint64_t key)
{
	if (IsExist(key))
		return;
	if (numelem >= allocsize)
		realloc(allocsize + mempadding);
	data[numelem++] = key;
}