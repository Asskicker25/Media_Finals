#pragma once

template <typename T, size_t Size>
class RingBuffer
{
public:
	void Write(const T* input, size_t count)
	{
		// TODO: Implement this
	}

	void Read(T* output, size_t count)
	{
		// TODO: Implement this
	}

private:
	T m_Data[Size];
	const size_t m_Size = Size;
	size_t m_Head = 0;
	size_t m_Tail = 0;
};