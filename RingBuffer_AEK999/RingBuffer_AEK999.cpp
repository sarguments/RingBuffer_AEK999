#include "stdafx.h"
#include "RingBuffer_AEK999.h"
#include "hoxy_Header.h"

///////////////////////////////////////////////////////////////////////////////////

// TODO : 계산 관련
//int remaining = (rear - front) + (-((int)(rear <= front)) & bufferSize);

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

		if (randSum == TEXT_SIZE)
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

			// Dequeue 하기 전에 Peek
			DreRing.Peek(PeekData, deqRand);
			DreRing.Dequeue(OutData, deqRand);

			// Peek 과 Dequeue 비교
			if (memcmp(OutData, PeekData, 300) != 0)
			{
				CCmdStart::CmdDebugText(L"MEMCMP", false);
				return -1;
			}

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

// 끊기지 않고 얻을 수 있는 사이즈
int CRingBuffer::GetNotBrokenGetSize(void)
{
	// 3가지 경우의 수

	// 1. rear == front
	if (_rear == _front)
	{
		return 0;
	}

	// 2. rear > front
	if (_rear > _front)
	{
		return _rear - _front;
	}

	// 3. rear < front
	return _size - _front;
}

// 끊기지 않고 넣을 수 있는 사이즈
int CRingBuffer::GetNotBrokenPutSize(void)
{
	// 6가지 경우의 수

	// 1. rear == front / rear가 끝
	if (_rear == _front && _rear == _size)
	{
		return 1;
	}
	// 2. rear == front / rear가 시작위치
	else if (_rear == _front && _rear == 0)
	{
		return _size - 1;
	}
	// 3. rear == front / rear가 중간
	else if (_rear == _front && _rear != 0) //
	{
		return _size - _rear;
	}
	// 4. rear > front / front가 처음
	else if (_rear > _front && _front == 0)
	{
		return _size - _rear - 1;
	}
	// 5. rear > front / 나머지 경우
	else if (_rear > _front) //
	{
		return _size - _rear;
	}

	// 6. rear < front / 모든 경우
	return _front - _rear - 1;
}

int CRingBuffer::Enqueue(char * chpData, int iSize)
{
	int toInputSize = iSize;
	int freeSize = GetFreeSize();

	// 넣으려는 사이즈가 여유공간보다 크면
	//여유공간만큼만 넣는다.
	if (iSize > freeSize)
	{
		toInputSize = freeSize;
	}

	int retval = 0;
	int notBrokenSize = GetNotBrokenPutSize();

	// 넣으려는 사이즈가 notBrokenSize보다 큰경우
	if (toInputSize > notBrokenSize)
	{
		char* toCpyPtr = GetRearBufferPtr();
		memcpy(toCpyPtr, chpData, notBrokenSize);

		// 나머지 복사해야할 사이즈
		int toRemainingSize = toInputSize - notBrokenSize;

		// 포인터 처음 위치로
		toCpyPtr = _buffer;
		memcpy(toCpyPtr, chpData + notBrokenSize, toRemainingSize);

		MoveRearPos(toInputSize);
	}
	else
	{
		// 그냥 넣는다
		char* toCpyPtr = GetRearBufferPtr();
		memcpy(toCpyPtr, chpData, toInputSize);

		MoveRearPos(toInputSize);
	}

	return retval;
}

int CRingBuffer::Dequeue(char * chpDest, int iSize)
{
	int toGetSize = iSize;
	int useSize = GetUseSize();
	if (toGetSize > useSize)
	{
		toGetSize = useSize;
	}

	int retval = 0;
	int notBrokenSize = GetNotBrokenGetSize();

	if (toGetSize > notBrokenSize)
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		int toRemainingSize = toGetSize - notBrokenSize;

		toCpyPtr = _buffer;
		memcpy(chpDest + notBrokenSize, toCpyPtr, toRemainingSize);

		MoveFrontPos(toGetSize);
	}
	else
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		MoveFrontPos(toGetSize);
	}

	return retval;
}

int CRingBuffer::Peek(char * chpDest, int iSize)
{
	int toGetSize = iSize;
	int useSize = GetUseSize();
	if (toGetSize > useSize)
	{
		toGetSize = useSize;
	}

	int notBrokenSize = GetNotBrokenGetSize();

	if (toGetSize > notBrokenSize)
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);

		int toRemainingSize = toGetSize - notBrokenSize;

		toCpyPtr = _buffer;
		memcpy(chpDest + notBrokenSize, toCpyPtr, toRemainingSize);
	}
	else
	{
		char* toCpyPtr = GetFrontBufferPtr();
		memcpy(chpDest, toCpyPtr, toGetSize);
	}

	return toGetSize;
}

int CRingBuffer::MoveFrontPos(int iSize)
{
	////////////////////////////////////////////////////////////
	// 사이즈를 지정하면 그만큼 front 이동(잘리는거 고려해서)
	////////////////////////////////////////////////////////////

	// 이동시키려는 사이즈가 경계를 넘어갈 경우
	if ((_front + iSize) > _size)
	{
		// 넣으려는 사이즈에서 안넘는 부분 뺀 값을 rear로
		_front = iSize - (_size - _front);
	}
	else
	{
		_front += iSize;

		if (_front == _size)
		{
			_front = 0;
		}
	}

	// TODO : 리턴값 의미 있나?
	return iSize;
}

int CRingBuffer::MoveRearPos(int iSize)
{
	////////////////////////////////////////////////////////////
	// 사이즈를 지정하면 그만큼 rear 이동(잘리는거 고려해서)
	////////////////////////////////////////////////////////////

	// 이동시키려는 사이즈가 경계를 넘어갈 경우
	if ((_rear + iSize) > _size)
	{
		// 넣으려는 사이즈에서 안넘는 부분 뺀 값을 rear로
		_rear = iSize - (_size - _rear);
	}
	else
	{
		_rear += iSize;

		if (_rear == _size)
		{
			_rear = 0;
		}
	}

	// TODO : 리턴값 의미 있나?
	return iSize;
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