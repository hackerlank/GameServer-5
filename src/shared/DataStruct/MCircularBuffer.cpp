#include "MCircularBuffer.h"
#include "DynBufResizePolicy.h"
#include "BufferUtil.h"
#include <string.h>
#include "Platform/Define.h"

/**
 * @brief ���캯��
 */
MCircularBuffer::MCircularBuffer(std::size_t len)
	: m_head(0), m_tail(0), m_size(0), m_iCapacity(len)
{
	m_storage = new char[len];
}

MCircularBuffer::~MCircularBuffer()
{
	delete[] m_storage;
}

std::size_t MCircularBuffer::size()
{
	return m_size;
}

bool MCircularBuffer::full()
{
	return (m_iCapacity == m_size);
}

void MCircularBuffer::clear()
{
	m_head = 0;
	m_tail = 0;
	m_size = 0;
}

char* MCircularBuffer::getStorage()
{
	return m_storage;
}

bool MCircularBuffer::empty()
{
	return (m_size == 0);
}

void MCircularBuffer::linearize()
{
	if (isLinearized())
	{
		// ����memcpy()   ��source  ָ���������destָ���������count���ַ�������������ص���������ú�������Ϊ����memmove(), ����������ص�����ֵ����ȷ���С�memcpy��������Ҫ���Ƶ��ڴ����򲻴����ص����������ȷ������и��Ʋ����ĵ��ڴ�����û���κ��ص�������ֱ����memcpy������㲻�ܱ�֤�Ƿ����ص���Ϊ��ȷ�����Ƶ���ȷ�ԣ��������memmove��memcpy��Ч�ʻ��memmove��һЩ������������׵Ļ����Կ�һЩ���ߵ�ʵ�֣� ����memcpy()   ��source  ָ���������destָ���������count���ַ�������������ص���������ú�������Ϊ����memmove(), ����������ص�����ֵ����ȷ���С�	memcpy��������Ҫ���Ƶ��ڴ����򲻴����ص����������ȷ������и��Ʋ����ĵ��ڴ�����û���κ��ص�������ֱ����memcpy������㲻�ܱ�֤�Ƿ����ص���Ϊ��ȷ�����Ƶ���ȷ�ԣ��������memmove��memcpy��Ч�ʻ��memmove��һЩ
		std::memmove(m_storage, m_storage + m_head, m_size);
	}
	else
	{
		if (m_iCapacity - m_head == m_tail)
		{
			BufferUtil::memSwap(m_storage, m_storage + m_head, m_tail);
		}
		else if (m_iCapacity - m_head > m_tail)
		{
			BufferUtil::memSwap(m_storage, m_storage + m_head, m_size - m_head);
			std::memmove(m_storage + (m_size - m_tail), m_storage + m_head, m_tail);
		}
		else
		{
			BufferUtil::memSwap(m_storage, m_storage + (m_size - m_tail), m_tail);
			std::memmove(m_storage, m_storage + (m_tail - m_head), m_head);
		}
	}

	m_head = 0;
	m_tail = m_size;
}

bool MCircularBuffer::isLinearized()
{
	return m_head <= m_tail;
}

size_t MCircularBuffer::capacity()
{
	return m_iCapacity;
}

void MCircularBuffer::setCapacity(std::size_t newCapacity)
{
	if (newCapacity == capacity())
	{
		return;
	}
	if (newCapacity < size())       // ���ܷ���ȵ�ǰ�Ѿ�ռ�еĿռ仹С�Ŀռ�
	{
		return;
	}
	char* tmpbuff = new char[newCapacity];   // �����µĿռ�
	if (isLinearized()) // �������һ���ڴ�ռ�
	{
		std::memcpy(tmpbuff, m_storage + m_head, m_size);
	}
	else    // ����������ڴ�ռ�
	{
		std::memcpy(tmpbuff, m_storage + m_head, m_iCapacity - m_head);
		std::memcpy(tmpbuff + m_iCapacity - m_head, m_storage, m_tail);
	}

	m_head = 0;
	m_tail = m_size;
	m_iCapacity = newCapacity;

	delete[] m_storage;
	m_storage = tmpbuff;
}

/**
*@brief �ܷ����� num ���ȵ�����
*/
bool MCircularBuffer::canAddData(uint32 num)
{
	if (m_iCapacity - m_size > num)
	{
		return true;
	}

	return false;
}

/**
* @brief �ڻ�����β������
*/
void MCircularBuffer::pushBack(char* pItem, std::size_t startPos, std::size_t len)
{
	if (!canAddData(len)) // �洢�ռ����Ҫ��ʵ���������ٶ� 1
	{
		uint32 closeSize = DynBufResizePolicy::getCloseSize(len + size(), capacity());
		setCapacity(closeSize);
	}

	if (isLinearized())
	{
		if (len <= (m_iCapacity - m_tail))
		{
			std::memcpy(m_storage + m_tail, pItem + startPos, len);
		}
		else
		{
			std::memcpy(m_storage + m_tail, pItem + startPos, m_iCapacity - m_tail);
			std::memcpy(m_storage + 0, pItem + m_iCapacity - m_tail, len - (m_iCapacity - m_tail));
		}
	}
	else
	{
		std::memcpy(m_storage + m_tail, pItem + startPos, len);
	}

	m_tail += len;
	m_tail %= m_iCapacity;

	m_size += len;
}

/**
* @brief �ڻ�����ͷ������
*/
void MCircularBuffer::pushFront(char* pItem, std::size_t startPos, std::size_t len)
{
	if (!canAddData(len)) // �洢�ռ����Ҫ��ʵ���������ٶ� 1
	{
		uint32 closeSize = DynBufResizePolicy::getCloseSize(len + size(), capacity());
		setCapacity(closeSize);
	}

	if (isLinearized())
	{
		if (len <= m_head)
		{
			std::memcpy(m_storage + m_head - len, pItem + startPos, len);
		}
		else
		{
			std::memcpy(m_storage + 0, pItem + startPos + len - m_head, m_head);
			std::memcpy(m_storage + m_iCapacity - (len - m_head), pItem + 0, len - m_head);
		}
	}
	else
	{
		std::memcpy(m_storage + m_head - len, pItem + startPos + 0, len);
	}

	if (len <= m_head)
	{
		m_head -= (uint32)len;
	}
	else
	{
		m_head = m_iCapacity - ((uint32)len - m_head);
	}
	m_size += (uint32)len;
}

/**
* @brief ��ȡ������β��������ɾ��
*/
bool MCircularBuffer::popBack(char* pItem, std::size_t startPos, std::size_t len)
{
	if (back(pItem, startPos, len))
	{
		popBackLenNoData(len);
		return true;
	}

	return false;
}

/**
* @brief ��ȡ������β�������ǲ�ɾ��
*/
bool MCircularBuffer::back(char* pItem, std::size_t startPos, std::size_t len)
{
	if (len <= size())
	{
		return false;
	}

	if (isLinearized())
	{
		std::memcpy(pItem + startPos, m_storage + m_tail - len, len);
	}
	else
	{
		if (len <= m_tail)
		{
			std::memcpy(pItem + startPos, m_storage, len);
		}
		else
		{
			std::memcpy(pItem + startPos, m_storage + m_iCapacity - (len - m_tail), len - m_tail);
			std::memcpy(pItem + startPos + len - m_tail, m_storage + 0, m_tail);
		}
	}

	return true;
}

void MCircularBuffer::popBackLenNoData(std::size_t len)
{
	m_tail -= len;
	m_tail += m_iCapacity;
	m_tail %= m_iCapacity;
	m_size -= len;
}

/**
* @brief ��ȡ������ͷ��������ɾ��
*/
bool MCircularBuffer::popFront(char* pItem, std::size_t startPos, std::size_t len)
{
	if (front(pItem, startPos, len))
	{
		popFrontLenNoData(len);
		return true;
	}

	return false;
}

/**
* @brief ��ȡ������ͷ�������ǲ�ɾ��
*/
bool MCircularBuffer::front(char* pItem, std::size_t startPos, std::size_t len)
{
	if (len <= size())
	{
		return false;
	}

	if (isLinearized())
	{
		std::memcpy(pItem + startPos, m_storage + m_head, len);
	}
	else
	{
		if (len <= (m_iCapacity - m_head))
		{
			std::memcpy(pItem + startPos, m_storage + m_head, len);
		}
		else
		{
			std::memcpy(pItem + startPos, m_storage + m_head, m_iCapacity - m_head);
			std::memcpy(pItem + startPos + m_iCapacity - m_head, m_storage + 0, len - (m_iCapacity - m_head));
		}
	}

	return true;
}

void MCircularBuffer::popFrontLenNoData(std::size_t len)
{
	m_head += len;
	m_head %= m_iCapacity;
	m_size -= len;
}