///////////////////////////////////////////////////////////
//
// Project: XTL
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: mkspec.h 27 2007-04-04 19:33:20Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __XTL_THREAD_H
#define __XTL_THREAD_H

#include "prepare.h"

#ifdef __XTL_USE_PRAGMAONE
#   pragma once 
#endif

#include "root.h"

#ifdef __XTL_USE_WINAPI
#  include <windows.h>
#else
#  include <pthread.h>
#endif 

///////////////////////////////////////////////////////////
// lockit
///////////////////////////////////////////////////////////

template <class _LockTy>
class lockit : public noncopyable
{
public:
    inline lockit(_LockTy *lock) : _lock(lock)
    { _lock->lock(); }
    ~lockit()
    { _lock->unlock(); }
    _LockTy * object() const
    { return _lock; }
private:
    _LockTy *_lock;
};

///////////////////////////////////////////////////////////
// basic_mutex
///////////////////////////////////////////////////////////

#ifdef __XTL_USE_WINAPI

template<class _ChTy>
class basic_mutex : public noncopyable
{
protected:
    CRITICAL_SECTION seed;
public:
    mutex() throw(basic_system_error<_ChTy>)
    {
        try { ::InitializeCriticalSection(&seed); }
        catch(...) { throw basic_system_error<_ChTy>(); }
    }
    ~mutex()
    { ::DeleteCriticalSection(&seed); }
    void lock()
    { ::EnterCriticalSection(&seed); }
    void unlock()
    { ::LeaveCriticalSection(&seed); }
};

//#else /* __XTL_USE_WINAPI */

protected:
    pthread_mutex_t seed;
public:
    mutex()
    { ::pthread_mutex_init(&seed, 0); }
    ~mutex()
    { ::pthread_mutex_destroy(&seed); }
    void lock()
    { ::pthread_mutex_lock(&seed); }
    void unlock()
    { ::pthread_mutex_unlock(&seed); }
};

#endif /* __XTL_USE_WINAPI */

///////////////////////////////////////////////////////////
// trigger
///////////////////////////////////////////////////////////

class trigger : public noncopyable
{
#ifdef __XTL_USE_WINAPI
protected:
    HANDLE seed;
public:
    inline trigger(
        bool autoreset = true,
        bool state = false)
    {
        seed = ::CreateEvent
    }
    inline ~trigger()
    {
        ::CloseHandle(seed);
    }
    inline bool wait(long timeout = -1)
    {
        return (WAIT_OBJECT_0 ==
            ::WaitForSingleObject(seed, (timeout<0)?INFINITE:timeout));
    }
    inline void signal()
    {
        ::SetEvent(seed);
    }
    inline void reset()
    {
        ::ResetEvent(seed);
    }
#else
#endif
};

///////////////////////////////////////////////////////////
// basic_tread
///////////////////////////////////////////////////////////

__XTL_BEGIN_NAMESPACE

template<class _ChTy>
class basic_tread : 


__XTL_END_NAMESPACE

#endif /* __XTL_THREAD_H */
