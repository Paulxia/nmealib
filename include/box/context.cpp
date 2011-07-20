///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: context.cpp 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#include "context.h"
#include "logger.h"
#include "sentence.h"
#include "config.h"

#include <fstream>
#include <sstream>

namespace box
{

///////////////////////////////////////////////////////////
// Params
///////////////////////////////////////////////////////////

Params::Params()
{
}

//=========================================================
void Params::addParam(
    const std::string &key,
    const std::string &value
    )
{
    _params[key] = value;
}

//=========================================================
void Params::clear()
{
    _params.clear();
}

//=========================================================
bool Params::hasKey(const std::string &key) const
{
    return _params.end() != _params.find(key);
}

//=========================================================
std::string Params::getParam(const std::string &key) const
{
    param_map_t::const_iterator it = _params.find(key);
    return ((it != _params.end())?it->second:std::string());
}

///////////////////////////////////////////////////////////
// Context
///////////////////////////////////////////////////////////

namespace tools
{
    std::string trimLeft(const std::string &str)
    {
        std::string resv;
        for(std::string::size_type i = 0; i < str.length(); ++i)
        {
            if(str[i] != ' ' && str[i] != '\t')
            {
                resv = str.substr(i, str.length() - i);
                break;
            }
        }
        return resv;
    }

    std::string trimRight(const std::string &str)
    {
        std::string resv;
        for(std::string::size_type i = str.length() - 1; i > 0 ; --i)
        {
            if(str[i] != ' ' && str[i] != '\t')
            {
                resv = str.substr(0, i + 1);
                break;
            }
        }
        return resv;
    }

    std::string trim(const std::string &str)
    {
        return trimLeft(trimRight(str));
    }

    std::string parseSqBr(const char *str)
    {
        bool hasBeg = false, hasEnd = false;
        std::string resv;

        for(; *str; ++str)
        {
            if(!hasBeg && *str == '[')
                hasBeg = true;
            else if(hasBeg && *str == ']')
            {
                hasEnd = true;
                break;
            }
            else if(hasBeg)
                resv += *str;
        }

        if(!hasEnd)
            resv.clear();

        return resv;
    }

    void parseParam(const std::string &str, const std::string &agent, Params &p)
    {
        std::string::size_type qp = str.find('=');
        if(qp != str.npos)
        {
            std::string key, val;
            key = trim(str.substr(0, qp));
            val = trimLeft(str.substr(qp + 1));
            if(!key.empty())
                p.addParam(agent + '.' + key, val);
        }
    }
}

//=========================================================
Context::Context()
{
}

//=========================================================
Context::~Context()
{
    agents_map_t::iterator it = _agents.begin();
    for(; it != _agents.end(); ++it)
        delete (*it).second;
}

//=========================================================
bool Context::init(const wchar_t *file_name)
{
    return init(Sentence::wctomb(file_name).c_str());
}

//=========================================================
bool Context::init(const char *file_name)
{
    static bool reg_def = false;

    if(!reg_def)
    {
        reg_def = true;
        reg("StreamLogger", new TLoggerAgent<StreamLogger>);
        reg("FileLogger", new TLoggerAgent<FileLogger>);
    }

    Params params;
    bool resv = true;
    std::string line, agent_name;
    std::ifstream fs(file_name);
    int log_index = 0;

    if(!fs.is_open())
        return resv;

    while(!fs.eof())
    {
        char buff[BOX_SPRINTF_BUFF], *pbuff = &buff[0];

        fs.getline(pbuff, BOX_SPRINTF_BUFF - 1);
        line = tools::trimLeft(pbuff);

        if(line.empty())
            continue;
        else if(line[0] == '#')
            continue;
        else if(line[0] == '[')
        {
            if(!agent_name.empty())
                _create(agent_name, params, log_index++);
            agent_name = tools::parseSqBr(pbuff);
        }
        else if(!agent_name.empty())
        {
            std::ostringstream ss;
            ss << agent_name << '_' << log_index;
            tools::parseParam(line, ss.str(), params);
        }
    }

    if(!agent_name.empty())
        _create(agent_name, params, log_index++);

    fs.close();

    return resv;
}

//=========================================================
void Context::reg(const std::string &name, LoggerAgent *agent)
{
    agents_map_t::const_iterator it = _self()->_agents.find(name);
    if(it == _self()->_agents.end())
        _self()->_agents[name] = agent;
    else
        delete agent;
}

//=========================================================
Context * Context::_self()
{
    static Context self;
    return &self;
}

//=========================================================
void Context::_create(
    const std::string &name,
    const Params &p, int index
    )
{
    std::ostringstream ss;
    ss << name << '_' << index;

    agents_map_t::iterator it = _self()->_agents.find(name);
    if(it != _self()->_agents.end())
        it->second->create(ss.str(), p);
}

} // namespace box
