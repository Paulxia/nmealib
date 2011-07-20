#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <tchar.h>

#include "config.h"

enum LogLevel
{
    LevelTrace = 0,
    LevelWarning,
    LevelError
};

#ifdef __cplusplus
extern "C" {
#endif

BOOL log_init(const PTCHAR file_name);
void log_done();

void log_push(int level, const PTCHAR format, ...);
void log_args(int level, const PTCHAR format, va_list *args);

void log_trace(const PTCHAR format, ...);
void log_warning(const PTCHAR format, ...);
void log_error(const PTCHAR format, ...);
void log_none(const PTCHAR format, ...);

#ifdef __cplusplus
}
#endif

#if VGD_USE_LOG

# ifdef DEBUGMSG
# undef DEBUGMSG
# endif

# ifdef DEBUG_CHECK
# undef DEBUG_CHECK
# endif

# define LOGINIT                log_init(VGD_LOG_FILE)
# define LOGDONE                log_done()
# define DEBUGMSG(z, exp)       log_none exp
# define DEBUG_CHECK(cond, exp) if(!(cond)) {log_trace exp;}

#else

# define LOGINIT
# define LOGDONE

#endif // VGD_USE_LOG

#endif // __LOG_H__
