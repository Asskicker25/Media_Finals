#pragma once

#include <iostream>

template <typename T, int Size>
class RingBuffer
{
public:
	void Write(const T* input, int count)
	{
		for (int i = 0; i < count; i++)
		{
			m_Data[m_Head] = input[i];
			//std::cout << "Data : " << input[i] << std::endl;
			AdvanceHead();
		}
	}

	void Read(T* output, int count)
	{
		for (int i = 0; i < count; i++)
		{
			output[i] = m_Data[m_Tail];
			AdvanceTail();
		}
	}

	int GetSize()
	{
		return Size;
	}

private:

	T m_Data[Size];
	const int m_Size = Size;
	int m_Head = 0;
	int m_Tail = 0;

	void AdvanceHead()
	{
		m_Head = (m_Head + 1) % m_Size;
		if (m_Head == m_Tail)
		{
			AdvanceTail();
		}
	}

	void AdvanceTail()
	{
		m_Tail = (m_Tail + 1) % m_Size;
	}
};