///////////////////////////////////////////////////////////
//
// Project: NmeaSDK
// Sub project: NMEA service
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __NMEASDK_SERVICE_GENERATOR_H__
#define __NMEASDK_SERVICE_GENERATOR_H__

#include "params.h"

namespace service
{
    ///////////////////////////////////////////////////////
    // Generator
    ///////////////////////////////////////////////////////

    class Generator
    {
    public:
        virtual ~Generator();

        void params(const Params &p);
        Params params() const;

        xtl::string generate();
        void reset();

    protected:
        Params _params;

        virtual xtl::string _generate() = 0;
        virtual void _reset() {}
        virtual void _updateparams() {}

        Generator();
        Generator(const Generator &) {}
        Generator & operator = (const Generator &) { return *this; }

    };

    ///////////////////////////////////////////////////////
    // GeneratorAgent
    ///////////////////////////////////////////////////////

    class GeneratorAgent
    {
    public:
        virtual ~GeneratorAgent();
        virtual Generator * create() = 0;
        virtual xtl::tstring name() const = 0;
        virtual xtl::tstring description() const = 0;

    protected:
        GeneratorAgent();
    };

    ///////////////////////////////////////////////////////
    // TGeneratorAgent
    ///////////////////////////////////////////////////////

    template<class _GenTy>
    class TGeneratorAgent
    {
    public:
        TGeneratorAgent(
            const xtl::tstring &name, 
            const xtl::tstring &description =
            _T("Sorry, no description for this generator.")
            )
            : _name(name)
            , _description(description)
        {}
        Generator * create() { return new _GenTy(); }
        virtual xtl::tstring name() const { return _name; }
        virtual xtl::tstring description() const { return _description; }

    private:
        xtl::tstring _name;
        xtl::tstring _description;

    };

} // namespace service

#endif // __NMEASDK_SERVICE_GENERATOR_H__
