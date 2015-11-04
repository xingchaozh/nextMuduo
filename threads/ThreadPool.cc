#include "ThreadPool.h"
#include "Log.h"

namespace nextMuduo {
ThreadPool::ThreadPool(int maxThreadPoolSize)
    :_aborted(false),
    _maxThreadPoolSize(maxThreadPoolSize)
{
}

ThreadPool::~ThreadPool()
{
    LOG_DEBUG << std::endl;
    abort();
}

void ThreadPool::start(int threadNum)
{
    if(!isAborted() && _threads.empty()) {
        _threads.reserve(threadNum);
        appendThreads(threadNum);
    }
}

void ThreadPool::appendThreads(int threadNum)
{
    MutexLockGuard lock(_mutex);
    if(isAborted()) {
        return;
    }
    if ((threadNum + (int)_threads.size()) > _maxThreadPoolSize) {
        threadNum = _maxThreadPoolSize - (int)_threads.size();
    }
    for(int i = 0; i < threadNum; i++) {
        shared_ptr<Thread> thread;
        thread.reset(new Thread(std::bind(&ThreadPool::runInThread, this)));
        _threads.push_back(thread);
        thread->start();
    }
}

void ThreadPool::addTask(Task& task)
{
    if(isAborted()) {
        return;
    }
    _tasks.put(task);
}

void ThreadPool::runInThread()
{
    while(!isAborted()) {
        _tasks.take().performTask();
    }
}

void ThreadPool::abort()
{
    MutexLockGuard lock(_mutex);
    if(isAborted()) {
        return;
    }
    _aborted = true;
    for(int i = 0; i < (int)_threads.size(); i++) {
        addAbortTask();
    }
}

void ThreadPool::addAbortTask()
{
    Task task;
    _tasks.put(task);
}
}
