///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: mutex.cpp 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#include "mutex.h"
#include "config.h"

#ifdef BOX_WIN
#   pragma warning(disable: 4201)
#   include <windows.h>
#   pragma warning(default: 4201)
#else
#   include <pthread.h>
#endif

namespace box
{

///////////////////////////////////////////////////////////
// MutexSeed
///////////////////////////////////////////////////////////

struct MutexSeed
{
#ifdef BOX_WIN
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif
};

///////////////////////////////////////////////////////////
// Mutex
///////////////////////////////////////////////////////////

Mutex::Mutex()
{
    _seed = new MutexSeed;
#ifdef BOX_WIN
    InitializeCriticalSection(&(_seed->cs));
#else
    pthread_mutex_init(&(_seed->mutex), 0);
#endif
}

//=========================================================
Mutex::~Mutex()
{
#ifdef BOX_WIN
    DeleteCriticalSection(&(_seed->cs));
#else
    pthread_mutex_destroy(&(_seed->mutex));
#endif
}

//=========================================================
void Mutex::lock()
{
#ifdef BOX_WIN
    EnterCriticalSection(&(_seed->cs));
#else
    pthread_mutex_lock(&(_seed->mutex));
#endif
}

//=========================================================
void Mutex::unlock()
{
#ifdef BOX_WIN
    LeaveCriticalSection(&(_seed->cs));
#else
    pthread_mutex_unlock(&(_seed->mutex));
#endif
}

} // namespace box
