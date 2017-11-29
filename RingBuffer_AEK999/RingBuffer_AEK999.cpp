#include "stdafx.h"
#include "RingBuffer_AEK999.h"
#include "hoxy_Header.h"

CRingBuffer::CRingBuffer(void)
	: _size(e_Size::DEFAULT_SIZE), _rear(0), _front(0)
{
	_buffer = (char*)malloc(_size);
}

CRingBuffer::CRingBuffer(int iBufferSize)
	: _size(iBufferSize), _rear(0), _front(0)
{
	_buffer = (char*)malloc(_size);
}

int main()
{
	CCmdStart MyCmdStart;
	CCmdStart::CmdDebugText(L"MAIN START", true);

	return 0;
}