#pragma once
#include <stdint.h>
#include <mutex>

enum ERROR_CODE
{
	ERR_OK  = 0,
	ERR_PARAM,          //无效参数
	ERR_NOT_ENOUGH_READ,     //没有足够的内容读取
	ERR_NOT_ENOUGH_WRITE,    //没有足够的缓存写入
};

class CBufferImp
{
public:
	explicit CBufferImp(int32_t nSize);
	~CBufferImp(void);

	int GetUsedSize();

	int GetCapacity();

	int ReadBytes(char* buf, int32_t readLen);

	int WriteBytes(const char* buf, int32_t writeLen);

private:
	CBufferImp(const CBufferImp&){}
	CBufferImp& operator&=(const CBufferImp&){}

private:
	int32_t _nCapacity; //缓存大小
	char* _pBuf;    //缓存首地址
	int32_t _nReadOffset;   //读取偏移
	int32_t _nWriteOffset;  //写入偏移
	bool _bFlag;  //用于对_nReadOffset 、_nWriteOffset相等时进行判定是空还是写满
	std::recursive_mutex _mtx;

};
