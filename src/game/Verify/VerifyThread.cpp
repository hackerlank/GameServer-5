#include "VerifyThread.h"
#include "Database/DatabaseEnv.h"
#include "Socket.h"
#include "MClientThreadSafeData.h"
#include "MNetClientBuffer.h"
#include "WorldSocket.h"

VerifyThread::VerifyThread() :
	m_Connections(0), m_exitFlag(false)
{

}

VerifyThread::~VerifyThread()
{
	Stop();
	Wait();
}

void VerifyThread::Stop()
{
	Wait();
}

void VerifyThread::Start()
{
	m_thread.reset(new boost::thread(boost::bind(&VerifyThread::svc, this)));
}

void VerifyThread::Wait()
{
	m_exitFlag = true;
	if (m_thread.get())
	{
		m_thread->join();
		m_thread.reset();
	}
}

void VerifyThread::AddSocket(const SocketPtr& sock)
{
	++m_Connections;

	boost::lock_guard<boost::mutex> lock(m_SocketsLock);
	m_Sockets.insert(sock);
}

void VerifyThread::RemoveSocket(const SocketPtr& sock)
{
	--m_Connections;

	boost::lock_guard<boost::mutex> lock(m_SocketsLock);
	m_Sockets.erase(sock);
}

void VerifyThread::svc()
{
	while (!m_exitFlag)
	{
		if (m_Sockets.size())
		{
			SocketSet removeList;
			// ������Ϣ
			SocketSet::iterator itBegin = m_Sockets.begin();
			SocketSet::iterator itEnd = m_Sockets.end();
			for (; itBegin != itEnd; ++itBegin)
			{
				MByteBuffer* pMsgBA;
				pMsgBA = (*itBegin)->getNetClientBuffer()->getMsg();
				while (pMsgBA)
				{
					((WorldSocket*)(itBegin->get()))->addSession();
					// Test ���յ���һ����Ϣ�ͽ��볡��
					removeList.insert(*itBegin);
					//RemoveSocket(*itBegin);
					break;
					//pMsgBA = (*itBegin)->getNetClientBuffer()->getMsg();
				}
			}

			if (removeList.size())
			{
				itBegin = removeList.begin();
				itEnd = removeList.end();

				for (; itBegin != itEnd; ++itBegin)
				{
					RemoveSocket(*itBegin);
				}

				removeList.clear();
			}
		}

		MaNGOS::Thread::Sleep(1000);
	}
}