///////////////////////////////////////////////////////////
//
// BOX - program logging library
//
// Author: Tim (spaceflush@mail.ru)
//
// Permission to use, copy, modify, and distribute this
// software for any purpose with or without fee is hereby
// granted
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY
// OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
// RIGHTS. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// $Id: context.h 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_CONTEXT_H__
#define __BOX_CONTEXT_H__

#include <map>

namespace box
{

class LoggerAgent;

///////////////////////////////////////////////////////////
// Params
///////////////////////////////////////////////////////////

class Params
{
public:
    Params();

    void addParam(
        const std::string &key,
        const std::string &value
        );
    void clear();

    bool hasKey(const std::string &key) const;
    std::string getParam(const std::string &key) const;

private:
    typedef std::map<std::string, std::string> param_map_t;
    param_map_t _params;

};

///////////////////////////////////////////////////////////
// Context
///////////////////////////////////////////////////////////

class Context
{
public:
    static bool init(const char *file_name);
    static bool init(const wchar_t *file_name);
    static void reg(const std::string &name, LoggerAgent *agent);

private:
    Context();
    ~Context();

    static Context * _self();
    static void _create(
        const std::string &name,
        const Params &p, int index
        );

private:
    typedef std::map<std::string, LoggerAgent *> agents_map_t;
    agents_map_t _agents;

};

} // namespace box

#endif // __BOX_CONTEXT_H__
