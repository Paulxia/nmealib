///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_SECTION_H__
#define __BOX_SECTION_H__

#include "log.h"

namespace box
{

///////////////////////////////////////////////////////////
// Section
///////////////////////////////////////////////////////////

class Section : public Log
{
public:
    Section(const std::string &name, bool trace_bounds = default_bounds());
    Section(const std::wstring &name, bool trace_bounds = default_bounds());
    ~Section();

    static void default_bounds(bool state);
    static bool default_bounds();

protected:
    virtual void _dolog(const Sentence &data);

private:
    bool _bounds;
    std::string _name;
    static bool _default_bounds;

};

} // namespace box

#endif // __BOX_SECTION_H__
