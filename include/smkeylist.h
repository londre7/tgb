#ifndef SMKEYLIST_HEADER
#define SMKEYLIST_HEADER

#include <stdlib.h>
#include <stdint.h>

#define KEYLIST_PADDING 10 // резервируем место на KEYLIST_PADDING элементов сразу, как только превысим - переаллоцируем

class SMKeyList
{
	protected:
		uint64_t *data;      // указатель на память с данными
		size_t   allocsize;  // количество выделенных элементов
		size_t   numelem;    // количетсво реальных элементов
		size_t   mempadding; // ??

		// управляем памятью
		void alloc(size_t sz, size_t _numelem = 0);
		void free();
		void realloc(size_t newsize);

	public:
		SMKeyList(void) { mempadding = KEYLIST_PADDING; alloc(mempadding); }
		SMKeyList(size_t _mempadding) { mempadding = _mempadding; alloc(mempadding); }
		SMKeyList(const SMKeyList& list);

		~SMKeyList() { free(); } // пока нет нужды делать виртуальным

		size_t GetNumElem(void) const { return numelem; }
		size_t GetAllocSize(void) const { return allocsize; }
		bool IsExist(uint64_t key) const;
		bool IsExist(uint64_t key, size_t &index) const;
		virtual void AddKey(uint64_t key);
		uint64_t RemoveKey(uint64_t key);
		uint64_t RemoveKeyByIndex(size_t index);
		uint64_t GetKey(size_t index) const;
		
		// совместимость с STL
		uint64_t at(size_t index) const { return GetKey(index); }
		size_t size() const { return numelem; }
		void push_back(uint64_t key) { AddKey(key); }

		// перегрузим операторы
		SMKeyList& operator<< (uint64_t key) { AddKey(key); return *this; }
		uint64_t operator>> (uint64_t key) { return RemoveKey(key); }
};

// TODO: надо проверить корректность IsExist
class SMUnsortedKeyList : public SMKeyList
{
	typedef SMKeyList inherited;
	public:
		SMUnsortedKeyList(): inherited() {}
		SMUnsortedKeyList(size_t _mempadding): inherited(_mempadding) {}
		virtual void AddKey(uint64_t key);
};

#endif // SMKEYLIST_HEADER