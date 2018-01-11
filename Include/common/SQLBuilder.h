/*
 * SQL.h
 * 
 */
#ifndef _COMMON_SQL_H_
#define _COMMON_SQL_H_

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

namespace ezim
{
	class CSQLBufferH
	{
	public:
		CSQLBufferH() { assert(false); };
		CSQLBufferH(uint32 capacity) { _capacity = capacity; _buffer = new char[_capacity]; _pos = 0; };
		~CSQLBufferH() { delete[] _buffer; _buffer = 0; _capacity = 0; _pos = 0;};
		uint32 getCapacity() { return _capacity; };
		char* getBasePtr() { return _buffer; };
		char* getPtr() { return _buffer + _pos; };
		uint32 getPosition() { return _pos; };
		uint32 incPosition(uint32 inc) { return (_pos += inc); };
		uint32 getRemaining() { return _capacity - _pos; };
		void reset() { _pos = 0; };
	private:
		uint32 _capacity;
		uint32 _pos;
		char* _buffer;
	};

	template<int len>
	class CSQLBufferS
	{
	public:
		CSQLBufferS() { _pos = 0; };
		CSQLBufferS(uint32 capacity) { _pos = 0; };
		uint32 getCapacity() { return len; };
		char* getBasePtr() { return _buffer; };
		char* getPtr() { return _buffer + _pos; };
		uint32 getPosition() { return _pos; };
		uint32 incPosition(uint32 inc) { return (_pos += inc); };
		uint32 getRemaining() { return len - _pos; };
		void reset() { _pos = 0; };
	private:
		char _buffer[len];
		uint32 _pos;
	};

	template<typename T>
	class CSQLBuilder
	{
	public:
		CSQLBuilder() {};
		CSQLBuilder(uint32 capacity)
			: buffer(capacity) 
		{
			*buffer.getPtr()=0;
		};

		uint32 getLength()
		{
			return buffer.getPosition();
		}

		char* getSQL()
		{
			return buffer.getBasePtr();
		}

		char* getPtr()
		{
			return buffer.getPtr();
		}

		uint32 getRemaining()
		{
			return buffer.getRemaining();
		}

		uint32 incPosition(uint32 inc)
		{
			return buffer.incPosition(inc);
		}

		void reset()
		{
			buffer.reset();
		}

		CSQLBuilder& operator <<(const char* str)
		{
			size_t len = strlen(str);
			if(buffer.getRemaining() < len+1)
			{
				throw std::runtime_error("CSQLBuilder capacity of buffer is lacking!");
			}

			memcpy(buffer.getPtr(), str, len);
			buffer.incPosition((uint32)len);
			*buffer.getPtr() = 0;
			return *this;
		}
		
 		CSQLBuilder& operator <<(uint64_t value)
		{
			if(buffer.getRemaining() < 22)
			{
				throw std::runtime_error("CSQLBuilder capacity of buffer is lacking!");
			}
			char* ptr = buffer.getPtr();
			
			sprintf(ptr, "%llu", value);

			while(*buffer.getPtr() != 0) buffer.incPosition(1);
			return *this;
		}

		bool append(void* data, uint32 len)
		{
			if(buffer.getRemaining() < len+1)
			{
				return false;
			}

            memcpy(buffer.getPtr(), data, len);
			buffer.incPosition(len);
			*buffer.getPtr() = 0;
			return true;
		}
	private:
		T buffer;
	};

	typedef CSQLBufferS<128> TSQLBuffer128;
	typedef CSQLBuilder<TSQLBuffer128> CSQLBuilder128;

	typedef CSQLBufferS<256> TSQLBuffer256;
	typedef CSQLBuilder<TSQLBuffer256> CSQLBuilder256;

	typedef CSQLBufferS<512> TSQLBuffer512;
	typedef CSQLBuilder<TSQLBuffer512> CSQLBuilder512;

	typedef CSQLBufferS<1024> TSQLBuffer1024;
	typedef CSQLBuilder<TSQLBuffer1024> CSQLBuilder1024;

	typedef CSQLBufferS<1024*4> TSQLBuffer4K;
	typedef CSQLBuilder<TSQLBuffer4K> CSQLBuilder4K;

	typedef CSQLBufferS<1024*8> TSQLBuffer8K;
	typedef CSQLBuilder<TSQLBuffer8K> CSQLBuilder8K;

	typedef CSQLBufferS<1024*16> TSQLBuffer16K;
	typedef CSQLBuilder<TSQLBuffer16K> CSQLBuilder16K;

	typedef CSQLBufferS<1024*32> TSQLBuffer32K;
	typedef CSQLBuilder<TSQLBuffer32K> CSQLBuilder32K;

	typedef CSQLBufferH TSQLBufferH;
	typedef CSQLBuilder<TSQLBufferH> CSQLBuilderH;
}

#endif /*_COMMON_SQL_H_*/

