#ifndef CUSTOM_BUFFER_HEADER
#define CUSTOM_BUFFER_HEADER

#include <stdint.h>

#define BUF_MEMPADDING 8192ULL
class SMBaseIO
{
	protected:
		uint8_t *buffer;
		size_t  allocsize;
		size_t  golovkaIO;
		size_t  mempadding;

		void alloc(size_t sz, size_t _golovkaIO=0ULL)
		{
			buffer = new uint8_t[sz];
			allocsize = sz;
			golovkaIO = _golovkaIO;
		}
		void free() 
		{ 
			if (buffer) 
			{ 
				delete[] buffer; 
				buffer = nullptr; 
			} 
		}
		void realloc(size_t newsize)
		{
			allocsize += newsize;
			uint8_t* newbuf = new uint8_t[allocsize];
			memcpy(newbuf, buffer, golovkaIO*sizeof(uint8_t));
			free();
			buffer = newbuf;
		}
		void reset() { golovkaIO = 0ull; }

	public:
		SMBaseIO() : mempadding(BUF_MEMPADDING) { alloc(mempadding); }
		SMBaseIO(size_t sz) : mempadding(sz) { alloc(mempadding); }
		~SMBaseIO() { free(); }

		uint8_t* GetBufferPtr() const { return buffer; }
		size_t GetAllocSize() const { return allocsize; }
		size_t GetMemPadding() const { return mempadding; }
		virtual size_t GetGolovkaIO() const = 0;
};
class SMOutBuffer : public SMBaseIO
{
	protected:
		bool NeededReallocation(size_t putsz) 
		{ 
			return ((golovkaIO + putsz) > allocsize); 
		}

	public:
		SMOutBuffer() : SMBaseIO() {}
		SMOutBuffer(size_t sz) : SMBaseIO(sz) {}

		size_t GetWritePos() const { return golovkaIO; }
		virtual size_t GetGolovkaIO() const { return GetWritePos(); }

		inline void PutBlock(const void* block, size_t bsize)
		{
			if (NeededReallocation(bsize)) realloc(bsize+mempadding);
			memcpy(&buffer[golovkaIO], block, bsize);
			golovkaIO += bsize;
		}
		void PutZeroByte() { uint8_t zero = 0u; PutBlock(reinterpret_cast<void*>(&zero), sizeof(zero)); }
		void PutInt(int pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutInt8(int8_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutInt16(int16_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutInt32(int32_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutInt64(int64_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutUInt(unsigned int pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutUInt8(uint8_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutUInt16(uint16_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutUInt32(uint32_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutUInt64(uint64_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutChar(char pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutUChar(unsigned char pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutWchar(wchar_t pval) { PutBlock(reinterpret_cast<void*>(&pval), sizeof(pval)); }
		void PutCharStr(const char* str, size_t strl = 0ULL) { PutBlock(str, (strl)?strl:strlen(str)+1); }
		void PutCharStr(const SMAnsiString& str) { PutBlock(str.c_str(), str.length()+1); }
};
// TODO: надо доделать
class SMInBuffer : public SMBaseIO
{
	public:
		SMInBuffer() : SMBaseIO() {}
		SMInBuffer(size_t sz) : SMBaseIO(sz) {}

		size_t GetReadPos() const { return golovkaIO; }
		virtual size_t GetGolovkaIO() const { return GetReadPos(); }

		void GetBlock(void* block, size_t bsize) {}
		void GetInt(int& pval) {}
		void GetInt8(int8_t& pval) {}
		void GetInt16(int16_t& pval) {}
		void GetInt32(int32_t& pval) {}
		void GetInt64(int64_t& pval) {}
		void GetUInt(unsigned int &pval) {}
		void GetUInt8(uint8_t& pval) {}
		void GetUInt16(uint16_t& pval) {}
		void GetUInt32(uint32_t& pval) {}
		void GetUInt64(uint64_t& pval) {}
		void GetChar(char &pval) {}
		void GetUChar(unsigned char &pval) {}
		void GetWchar(wchar_t& pval) {}
		void GetStr(char* str, size_t bsize) {}
		void GetStr(SMAnsiString& str) {}
};

#endif