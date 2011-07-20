///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#include "section.h"
#include "sentence.h"

namespace box
{

bool Section::_default_bounds = true;

Section::Section(const std::string &name, bool trace_bounds)
: Log(false)
, _bounds(trace_bounds)
, _name(name)
{
    if(_bounds)
        log(LevelTrace, "+");
}

Section::Section(const std::wstring &name, bool trace_bounds)
: Log(false)
, _bounds(trace_bounds)
, _name(Sentence::wctomb(name))
{
    if(_bounds)
        log(LevelTrace, "+");
}

Section::~Section()
{
    if(_bounds)
        log(LevelTrace, "-");
}

void Section::_dolog(const Sentence &data)
{
    Sentence sen(data);
    sen.prefix += " " + _name;
    Log::specific()->log(sen);
}

void Section::default_bounds(bool state)
{
    _default_bounds = state;
}

bool Section::default_bounds()
{
    return _default_bounds;
}

} // namespace box
