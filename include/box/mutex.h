///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: mutex.h 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_MUTEX_H__
#define __BOX_MUTEX_H__

namespace box
{

///////////////////////////////////////////////////////////
// Mutex
///////////////////////////////////////////////////////////

class Mutex
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    struct MutexSeed *_seed;

};

///////////////////////////////////////////////////////////
// LockIt
///////////////////////////////////////////////////////////

class LockIt
{
public:
    inline LockIt(Mutex *lo)
        : _mutex(lo)
    { _mutex->lock(); }
    ~LockIt()
    { _mutex->unlock(); }

    inline Mutex * seed() const
    { return _mutex; }

private:
    Mutex *_mutex;

};

} // namespace box

#endif // __BOX_MUTEX_H__
