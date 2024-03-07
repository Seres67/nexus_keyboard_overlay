#include "Shared.h"
#include "nexus/Nexus.h"
#include <utils.h>
namespace Log
{
void info(const char *message) { APIDefs->Log(ELogLevel_INFO, message); }
void debug(const char *message) { APIDefs->Log(ELogLevel_DEBUG, message); }
} // namespace Log
