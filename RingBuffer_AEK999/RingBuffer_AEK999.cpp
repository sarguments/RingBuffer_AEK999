#include "stdafx.h"
#include "RingBuffer_AEK999.h"
#include "hoxy_Header.h"

// TODO : TEST_SIZE
CRingBuffer::CRingBuffer(void)
	: _size(e_Size::TEST_SIZE), _rear(0), _front(0)
{
	Initial(_size);
}

CRingBuffer::CRingBuffer(int iBufferSize)
	: _size(iBufferSize), _rear(0), _front(0)
{
	Initial(_size);
}

int CRingBuffer::GetBufferSize(void)
{
	return _size;
}

int CRingBuffer::GetUseSize(void)
{
	// rear 가 front보다 크면
	if (_rear > _front)
	{
		return _rear - _front;
	}
	else if (_rear < _front)
	{
		return _rear + (_size - _front);
	}
	else
	{
		// 같으면
		return 0;
	}
}

int CRingBuffer::GetFreeSize(void)
{
	return _size - GetUseSize() - 1;
}

int CRingBuffer::GetNotBrokenGetSize(void)
{
	// TODO : 다른 경우도 고려
	return _size - _front;
}

int CRingBuffer::GetNotBrokenPutSize(void)
{
	return _size - _rear;
}

int CRingBuffer::Enqueue(char * chpData, int iSize)
{
	int retval = MoveRearPos(iSize);

	return retval;
}

int CRingBuffer::Dequeue(char * chpDest, int iSize)
{
	int retval = MoveFrontPos(iSize);

	return retval;
}

///////////////////////////////////////////////////////////////////////////////////

int main()
{
	CCmdStart MyCmdStart;
	CCmdStart::CmdDebugText(L"MAIN START", true);

	CRingBuffer DreRing;

	while (1)
	{
		int InputNum = 0;
		wcout << L"Enqueue : ";
		std::cin >> InputNum;

		DreRing.Enqueue((char*)nullptr, InputNum);

		wcout << L"BufferSize : " << DreRing.GetBufferSize() << endl;
		wcout << L"UseSize : " << DreRing.GetUseSize() << endl;
		wcout << L"FreeSize : " << DreRing.GetFreeSize() << endl;

		wcout << L"------------------------------------------------------" << endl;

		InputNum = 0;
		wcout << L"Dequeue : ";
		std::cin >> InputNum;

		DreRing.Dequeue((char*)nullptr, InputNum);

		wcout << L"UseSize : " << DreRing.GetUseSize() << endl;
		wcout << L"FreeSize : " << DreRing.GetFreeSize() << endl;

		wcout << L"------------------------------------------------------" << endl;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

int CRingBuffer::MoveFrontPos(int iSize)
{
	if (_rear == _front)
	{
		// 읽을게 없다.
		return 0;
	}

	int toMoveSize = iSize;

	// 읽을게 모자랄 경우 있는 만큼 읽는다
	if (iSize > GetUseSize())
	{
		toMoveSize = GetUseSize();
	}

	// rear가 front보다 작을 경우
	if (_rear < _front)
	{
		// 1. 안끊기고 움직일수 있는 크기를 구하고
		int notBrokenToMove = GetNotBrokenGetSize();
			
		// 읽을 사이즈가 1. 이하인 경우
		if (iSize <= notBrokenToMove)
		{
			// 이동시키고 끝
			_front += toMoveSize;

			// TODO : 테스트 해봐야.. 이게 음..
			// _front가 사이즈이상이면 0으로
			if (_front >= _size)
			{
				_front = 0;
			}

			return toMoveSize;
		}

		// iSize 에서 1. 을 뺀만큼 front 대입
		_front = toMoveSize - notBrokenToMove;
		return notBrokenToMove + _front;
	}

	//  rear가 front보다 클 경우
	// 그냥 더한다.
	_front += toMoveSize;
	return toMoveSize;
}

int CRingBuffer::MoveRearPos(int iSize)
{
	if (_rear == (_front - 1))
	{
		// 쓸 공간이 없다.
		return 0;
	}

	int toMoveSize = iSize;

	// TODO : 함수호출 줄이기
	// TODO : 쓸 공간이 모자랄 경우 쓸수 있는 만큼 <- 이 예외처리 여기서도 하는게 맞나?
	if (iSize > GetFreeSize())
	{
		toMoveSize = GetFreeSize();
	}

	// rear가 front보다 클 경우
	if (_rear > _front)
	{
		// 1. 안끊기고 움직일수 있는 크기를 구하고
		int notBrokenToMove = GetNotBrokenPutSize();

		// 쓸 사이즈가 1. 이하인 경우
		if (iSize <= notBrokenToMove)
		{
			// 이동시키고 끝
			_rear += toMoveSize;

			// _rear가 사이즈이상이면 0으로
			if (_rear >= _size)
			{
				_rear = 0;
			}

			return toMoveSize;
		}

		// iSize 에서 1. 을 뺀만큼 rear 대입
		_rear = toMoveSize - notBrokenToMove; 
		return notBrokenToMove + _rear;
	}

	//  rear가 front보다 작을 경우
	// 그냥 더한다.
	_rear += toMoveSize;
	return toMoveSize;
}

void CRingBuffer::ClearBuffer(void)
{
	_front = 0;
	_rear = 0;
}

char * CRingBuffer::GetBufferPtr(void)
{
	return _buffer;
}

char * CRingBuffer::GetFrontBufferPtr(void)
{
	return _buffer + _front;
}

char * CRingBuffer::GetRearBufferPtr(void)
{
	return _buffer + _rear;
}

void CRingBuffer::Initial(int iBufferSize)
{
	_buffer = (char*)malloc(iBufferSize);
}
