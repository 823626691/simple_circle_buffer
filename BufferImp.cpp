#include "BufferImp.h"
#include <assert.h>

CBufferImp::CBufferImp(int32_t nSize):
	_nReadOffset(0),
	_nWriteOffset(0),
	_bFlag(false)
{
	_pBuf = new char[nSize];
	_nCapacity = nSize;
}

CBufferImp::~CBufferImp(void)
{
	delete[] _pBuf;
}

/**used 计算
**1.对于 （_nWriteOffset > _nReadOffset） used = _nWriteOffset - _nReadOffset 字节
**2.对于 （_nWriteOffset < _nReadOffset） used = _nWriteOffset - _nReadOffset + _nCapacity 字节
**3.对于 （_nWriteOffset == _nReadOffset） used = 0 或者 used = _nCapacity
*/
int CBufferImp::GetUsedSize()
{
	std::lock_guard<std::recursive_mutex> lock(_mtx);

	int nUsed = 0;
	if(_nReadOffset < _nWriteOffset)
	{
		nUsed = _nWriteOffset - _nReadOffset;
	}
	else if(_nReadOffset > _nWriteOffset)
	{
		nUsed = _nWriteOffset - _nReadOffset + _nCapacity;
	}
	else
	{
		nUsed = _bFlag ? _nCapacity : 0;
	}

	return nUsed;
}

int CBufferImp::GetCapacity()
{
	return _nCapacity;
}

int CBufferImp::ReadBytes(char* buf, int32_t readLen)
{
	if(buf == nullptr || readLen <= 0)
	{
		return ERR_PARAM;
	}

	int nUsed = GetUsedSize();
	if( readLen > nUsed)
	{
		return ERR_NOT_ENOUGH_READ;
	}
	
	std::lock_guard<std::recursive_mutex> lock(_mtx);
	if(_nReadOffset < _nWriteOffset)
	{
		memcpy(buf, _pBuf + _nReadOffset, readLen);
		_nReadOffset += readLen;	
		assert(_nReadOffset <= _nWriteOffset);	      //到这里读取后_nReadOffset必定小于等于_nWriteOffset
	}
	else if(_nReadOffset > _nWriteOffset)
	{
		int nTmp = _nCapacity - _nReadOffset;         //读到结尾字节数
		if(readLen <= nTmp)
		{
			memcpy(buf, _pBuf + _nReadOffset, readLen);
			_nReadOffset = (_nReadOffset + readLen) % _nCapacity;
		}
		else
		{
			memcpy(buf, _pBuf + _nReadOffset, nTmp);   //复制到结尾
			memcpy(buf + nTmp, _pBuf, readLen - nTmp); //复制剩余部分
			_nReadOffset = readLen - nTmp;
			assert(_nReadOffset <= _nWriteOffset);	   //到这里读取后_nReadOffset必定小于等于_nWriteOffset
		}
	}
	else
	{
		int nTmp = _nCapacity - _nReadOffset;          //读到结尾字节数
		if(readLen <= nTmp)
		{
			memcpy(buf, _pBuf + _nReadOffset, readLen);
			_nReadOffset = (_nReadOffset + readLen) % _nCapacity;
		}
		else
		{
			memcpy(buf, _pBuf + _nReadOffset, nTmp);   //复制到结尾
			memcpy(buf + nTmp, _pBuf, readLen - nTmp); //复制剩余部分
			_nReadOffset = readLen - nTmp;
			assert(_nReadOffset <= _nWriteOffset);	   //到这里读取后_nReadOffset必定小于等于_nWriteOffset
		}
	}

	if(_nReadOffset == _nWriteOffset)
	{
		_bFlag = false;
	}

	return ERR_OK;
}

int CBufferImp::WriteBytes(const char* buf, int32_t writeLen)
{
	if(buf == nullptr || writeLen <=0)
	{
		return ERR_PARAM;
	}

	int nUsed = GetUsedSize();
	if( writeLen > _nCapacity - nUsed)
	{
		return ERR_NOT_ENOUGH_WRITE;
	}

	std::lock_guard<std::recursive_mutex> lock(_mtx);

	if(_nWriteOffset > _nReadOffset)
	{
		int nTmp = _nCapacity - _nWriteOffset;       //写到结尾字节数
		if( nTmp >= writeLen)
		{
			memcpy(_pBuf + _nWriteOffset, buf, writeLen);
			_nWriteOffset = (_nWriteOffset + writeLen) % _nCapacity;
		}
		else
		{
			memcpy(_pBuf + _nWriteOffset, buf, nTmp); //写到缓存结尾
			memcpy(_pBuf, buf, writeLen - nTmp);      //写剩余部分
			_nWriteOffset = writeLen - nTmp;
			assert(_nWriteOffset <= _nReadOffset);    //到这里读取后_nReadOffset必定大于等于_nWriteOffset)
		}
	}
	else if(_nWriteOffset < _nReadOffset)
	{
		memcpy(_pBuf + _nWriteOffset, buf, writeLen);
		_nWriteOffset += writeLen;
		assert(_nWriteOffset <= _nReadOffset);       //到这里读取后_nReadOffset必定大于等于_nWriteOffset)
	}
	else
	{
		int nTmp = _nCapacity - _nWriteOffset;       //写到结尾字节数
		if( nTmp >= writeLen)
		{
			memcpy(_pBuf + _nWriteOffset, buf, writeLen);
			_nWriteOffset = (_nWriteOffset + writeLen) % _nCapacity;
		}
		else
		{
			memcpy(_pBuf + _nWriteOffset, buf, nTmp); //写到缓存结尾
			memcpy(_pBuf, buf, writeLen - nTmp);      //写剩余部分
			_nWriteOffset = writeLen - nTmp;
			assert(_nWriteOffset <= _nReadOffset);    //到这里读取后_nReadOffset必定大于等于_nWriteOffset)
		}
	}

	if(_nWriteOffset == _nReadOffset)
	{
		_bFlag = true;
	}

	return 0;
}
