#pragma once
#include "pch.h"
#include <atomic>
#include <list>
#include "HuxlThread.h"


template<class T>
class CEdoyunQueue
{//线程安全的队列，利用IOCP实现
public:
	typedef struct IocpParam {
		size_t nOperator;//操作
		T Data;//数据
		_beginthread_proc_type cbFunc;//回调
		HANDLE hEvent;//pop操作需要的
		IocpParam(int op, const T& data, HANDLE hEve = NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//Post Parameter 用于投递信息的结构体
	enum {
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};
protected:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock;//队列正在析构，担心post后还有人push或pop
public:
	CEdoyunQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL) {
			m_hThread = (HANDLE)_beginthread(&CEdoyunQueue<T>::threadEntry, 0, this);
		}
	};
	virtual ~CEdoyunQueue() {
		if (m_lock)return;
		m_lock = true;
		PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		if (m_hCompeletionPort) {
			HANDLE hTemp = m_hCompeletionPort;
			m_hCompeletionPort = NULL;
			//::CloseHandle(hTemp);
		}
	}
	bool PushBack(const T& data) {
		IocpParam* pParam = new IocpParam(EQPush, data);
		if (m_lock) {
			delete pParam;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;//post成功时由线程函数delete
		//printf("push back done %d %08p\r\n", ret,(void*)pParam);
		return ret;
	}
	//virtual bool PopFront(T& data) {
	//	if (m_lock)return false;
	//	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//	IocpParam Param(EQPop, data, hEvent);
	//	bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
	//	if (ret == false) {//与PostQueuedCompletionStatus间隙越小越好
	//		CloseHandle(hEvent);
	//		return false;
	//	}
	//	ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
	//	if (ret) {
	//		data = Param.Data;
	//	}
	//	return ret;
	//}
	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam Param(EQSize, T(), hEvent);
		if (m_lock) {//与PostQueuedCompletionStatus间隙越小越好
			if (hEvent)CloseHandle(hEvent);
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
		if (ret) {
			return Param.nOperator;
		}
		return -1;
	}
	bool Clear() {
		if (m_lock)return false;
		IocpParam* pParam = new IocpParam(EQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;//post成功时由线程函数delete
		//printf("Clear %08p\r\n", (void*)pParam);
		return ret;
	}
	

protected:
	static void threadEntry(void* arg) {
		CEdoyunQueue<T>* thiz = (CEdoyunQueue<T>*)arg;
		thiz->threadMain();
		//TODO:函数没实现
		_endthread();
	}
	virtual void DealParam(PPARAM* pParam) {
		switch (pParam->nOperator) {
		case EQPush:
			m_lstData.push_back(pParam->Data);
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		case EQPop:
			if (m_lstData.size() > 0) {
				pParam->Data = m_lstData.front();
				//pParam->Data = std::move(m_lstData.front());
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			break;
		case EQSize:
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			break;
		case EQClear:
			m_lstData.clear();
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}
	virtual void threadMain() {
		DWORD dwTransferred = 0;
		PPARAM* pParam = NULL;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* pOverlapped = NULL;
		while (GetQueuedCompletionStatus(m_hCompeletionPort, &dwTransferred,
			&CompletionKey, &pOverlapped, INFINITE)) {
			if ((dwTransferred == 0) || (CompletionKey == NULL)) {//post的参数
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		//防御性编程,防止post后有残余数据，如果没有数据就超时，进不来
		while (GetQueuedCompletionStatus(m_hCompeletionPort, &dwTransferred,
			&CompletionKey, &pOverlapped, 0)) {
			if ((dwTransferred == 0) || (CompletionKey == NULL)) {//post的参数
				printf("thread is prepare to exit!\r\n");
				continue;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		CloseHandle(m_hCompeletionPort);
	}
};


template<class T>
class HuxlSendQueue :public CEdoyunQueue<T>,public ThreadFuncBase
{
public:
	HuxlSendQueue() = default;
	typedef int (ThreadFuncBase::*HXLCALLBACK)(T& data);
	HuxlSendQueue(ThreadFuncBase*obj, HXLCALLBACK callback)
		:CEdoyunQueue<T>() ,m_base((ThreadFuncBase*)obj),m_callback(callback)
	{
		m_thread.Start();
		m_thread.UpdateWorker(::ThreadWorker(this, (FUNCTYPE)&HuxlSendQueue<T>::threadTick));
	}
	virtual ~HuxlSendQueue(){
		m_thread.Stop();
		m_base = NULL;
		m_callback = NULL;
	}
protected:
	virtual bool PopFront(T& data) { return false; };
	bool PopFront()
	{
		typename CEdoyunQueue<T>::IocpParam* Param=new typename CEdoyunQueue<T>::IocpParam(CEdoyunQueue<T>::EQPop, T());
		if (CEdoyunQueue<T>::m_lock) {
			delete Param;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(CEdoyunQueue<T>::m_hCompeletionPort, sizeof(*Param), (ULONG_PTR)&Param, NULL);
		if (ret == false) {//与PostQueuedCompletionStatus间隙越小越好
			delete Param;
			return false;
		}
		return ret;
	}

	int threadTick() {
		//WARN 
		/*if (WaitForSingleObject(CEdoyunQueue<T>::m_hThread, 0) != WAIT_TIMEOUT)return 0;*/
		if (WaitForSingleObject(CEdoyunQueue<T>::m_hThread, INFINITE)) return 0;
		if (CEdoyunQueue<T>::m_lstData.size() > 0)
		{
			PopFront();
		}
		return 0;
	}

	virtual void DealParam(typename CEdoyunQueue<T>::PPARAM* pParam) {
		switch (pParam->nOperator) {
		case CEdoyunQueue<T>::EQPush:
			CEdoyunQueue<T>::m_lstData.push_back(pParam->Data);
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		case CEdoyunQueue<T>::EQPop:
			if (CEdoyunQueue<T>::m_lstData.size() > 0) {
				pParam->Data = CEdoyunQueue<T>::m_lstData.front();
				if((m_base->*m_callback)(pParam->Data)==0)
				CEdoyunQueue<T>::m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			delete pParam;
			break;
		case CEdoyunQueue<T>::EQSize:
			pParam->nOperator = CEdoyunQueue<T>::m_lstData.size();
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			break;
		case CEdoyunQueue<T>::EQClear:
			CEdoyunQueue<T>::m_lstData.clear();
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}
private:
	ThreadFuncBase* m_base;
	HXLCALLBACK m_callback;
	HuxlThread m_thread;
};

typedef HuxlSendQueue< std::vector<char> >::HXLCALLBACK SENDCALLBACK;
