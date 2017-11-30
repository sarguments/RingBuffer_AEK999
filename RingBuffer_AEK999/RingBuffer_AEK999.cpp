#include "stdafx.h"
#include "RingBuffer_AEK999.h"
#include "hoxy_Header.h"

///////////////////////////////////////////////////////////////////////////////////

#define TEXT_SIZE 81

int main()
{
	CCmdStart MyCmdStart;
	CCmdStart::CmdDebugText(L"MAIN START", true);

	CRingBuffer DreRing;

	// 고정된 81자의 (콘솔화면 너비와 똑같은 문자열)  문자열에서 앞 부터  랜덤한 사이즈 만큼씩 큐에 넣고,  
	//큐에서 임의의 길이로 뽑아서 이를 출력하는 테스트.

	char* temp = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
	char testText[TEXT_SIZE];
	memcpy_s(testText, TEXT_SIZE, temp, TEXT_SIZE);

	int randSum = 0;

	while (1)
	{
		// 인큐한 포인터 이후부터 다시 인큐(랜드범위조절)
		int newRandNum = rand();
		newRandNum %= (TEXT_SIZE + 1 - randSum);
		DreRing.Enqueue(testText + randSum, newRandNum);

		randSum += newRandNum;

		if (randSum == 81)
		{
			randSum = 0;
		}

		// 사이즈구해서 그 범위안에서 디큐 루프
		int	qSize = DreRing.GetUseSize();
		while (qSize)
		{
			char OutData[300] = {};
			char PeekData[300] = {};

			int deqRand = rand() % (qSize + 1);

			DreRing.Dequeue(OutData, deqRand);

			std::cout << OutData;

			qSize = DreRing.GetUseSize();
		}
	}

	/*
	while (1)
	{
		int InputNum = 0;
		char inputText[10] = {};
		char getText[10] = {};

		wcout << L"Enqueue : ";
		std::cin >> inputText;

		DreRing.Enqueue(inputText, strlen(inputText));

		std::cout << "inputText : " << inputText << endl;
		wcout << L"UseSize : " << DreRing.GetUseSize() << endl;
		wcout << L"FreeSize : " << DreRing.GetFreeSize() << endl;

		wcout << L"------------------------------------------------------" << endl;

		InputNum = 0;
		wcout << L"Dequeue : ";
		std::cin >> InputNum;

		DreRing.Dequeue(getText, InputNum);

		std::cout << "GetText : " << getText << endl;
		wcout << L"UseSize : " << DreRing.GetUseSize() << endl;
		wcout << L"FreeSize : " << DreRing.GetFreeSize() << endl;

		wcout << L"------------------------------------------------------" << endl;
	}
	*/

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

// TODO : 위에서 사이즈 고려해서 밑에서는 그냥 넣을 수 있게?
// TODO : TEST_SIZE, 크기 빈칸 포함?
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
	if (_rear == _front)
	{
		return 0;
	}

	if (_rear > _front)
	{
		return _rear - _front;
	}

	return _size - _front;
}

int CRingBuffer::GetNotBrokenPutSize(void)
{
	// 6가지 경우의 수
	if (_rear == _front && _rear == _size)
	{
		return 1;
	}
	else if (_rear == _front && _rear == 0)
	{
		return _size - 1;
	}
	else if (_rear == _front && _rear != 0)
	{
		return _size - _rear;
	}
	else if (_rear > _front && _front == 0)
	{
		return _size - _rear - 1;
	}
	else if (_rear > _front)
	{
		return _size - _rear;
	}

	return _front - _rear - 1;
}

int CRingBuffer::Enqueue(char * chpData, int iSize)
{
	int toInputSize = iSize;
	int freeSize = GetFreeSize();
	if (iSize > freeSize)
	{
		toInputSize = freeSize;
	}

	int retval = 0;
	int notBrokenSize = GetNotBrokenPutSize();

	if (toInputSize > notBrokenSize)
	{
		char* toCpyPtr = GetRearBufferPtr();
		memcpy(toCpyPtr, chpData, notBrokenSize);

		retval = MoveRearPos(notBrokenSize);

		int toRemainingSize = toInputSize - notBrokenSize;

		toCpyPtr = GetRearBufferPtr();
		memcpy(toCpyPtr, chpData + retval, toRemainingSize);

		retval += MoveRearPos(toRemainingSize);
	}
	else
	{
		char* toCpyPtr = GetRearBufferPtr();
		memcpy(toCpyPtr, chpData, toInputSize);

		retval = MoveRearPos(toInputSize);
	}

	return retval;
}

int CRingBuffer::Dequeue(char * chpDest, int iSize)
{
	int toGetSize = iSize;
	int useSize = GetUseSize();
	if (iSize > useSize)
	{
		toGetSize = useSize;
	}

	int retval = 0;
	int notBrokenSize = GetNotBrokenGetSize();

	if (toGetSize > notBrokenSize)
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		// TODO : 리턴값은 어떻게?, peek하고 다름
		retval = MoveFrontPos(notBrokenSize);

		int toRemainingSize = toGetSize - notBrokenSize;

		toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest + retval, toCpyPtr, toRemainingSize);

		retval += MoveFrontPos(toRemainingSize);
	}
	else
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		retval = MoveFrontPos(toGetSize);
	}

	return retval;
}

int CRingBuffer::Peek(char * chpDest, int iSize)
{
	int toGetSize = iSize;
	int useSize = GetUseSize();
	if (iSize > useSize)
	{
		toGetSize = useSize;
	}

	//int retval = 0;
	int notBrokenSize = GetNotBrokenGetSize();

	if (toGetSize > notBrokenSize)
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		//retval = MoveFrontPos(notBrokenSize);

		int toRemainingSize = toGetSize - notBrokenSize;

		toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest + notBrokenSize, toCpyPtr, toRemainingSize);

		//retval += MoveFrontPos(toRemainingSize);
	}
	else
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		//retval = MoveFrontPos(toGetSize);
	}

	return toGetSize;
}

int CRingBuffer::MoveFrontPos(int iSize)
{
	_front += iSize;

	// _front가 사이즈와 같으면 0으로
	if (_front == _size)
	{
		_front = 0;
	}
	// TODO : 디버깅용
	else if (_front > _size || 0 > _size)
	{
		CCmdStart::CmdDebugText(L"ERROR!!!!", false);
		return -1;
	}
	return iSize;

	/*
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
	*/
}

int CRingBuffer::MoveRearPos(int iSize)
{
	_rear += iSize;

	// _rear가 사이즈와 같으면 0으로
	if (_rear == _size)
	{
		_rear = 0;
	
	}
	// TODO : 디버깅용
	else if (_front > _size || 0 > _size)
	{
		CCmdStart::CmdDebugText(L"ERROR!!!!", false);
		return -1;
	}
	return iSize;
	
	/*
	if (_rear == (_front - 1))
	{
		// 쓸 공간이 없다.
		return 0;
	}

	int toMoveSize = iSize;

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
	*/
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
