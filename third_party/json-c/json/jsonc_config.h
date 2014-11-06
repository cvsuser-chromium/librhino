/* json_config.h.  Generated from json_config.h.in by configure.  */

#if defined(OS_WIN32)
#include "jsonc_config_win32.h"
#elif defined(OS_LINUX)
#include "jsonc_config_linux.h"
#elif defined(ANDROID)
#include "jsonc_config_linux.h"
#else
#program error
#endif
