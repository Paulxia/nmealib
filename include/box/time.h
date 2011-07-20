///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_TIME_H__
#define __BOX_TIME_H__

namespace box
{

///////////////////////////////////////////////////////////
// Time
///////////////////////////////////////////////////////////

class Time
{
public:
    Time();
    Time(const Time &src);

    inline int day() const { return _mday; }
    inline int weekDay() const { return _wday; }
    inline int month() const { return _mon; }
    inline int year() const { return _year; }
    inline int hour() const { return _hour; }
    inline int min() const { return _min; }
    inline int sec() const { return _sec; }
    inline int milliseconds() const { return _msec; }

    Time & operator = (const Time &src);

    static Time now();

private:
    int _mday;
    int _wday;
    int _mon;
    int _year;
    int _hour;
    int _min;
    int _sec;
    int _msec;

};

} // namespace box

#endif // __BOX_TIME_H__
