#ifndef MACROS
#define MACROS

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>
#include <sstream>

template <typename ...Args>
void Trace(Args&& ...args)
{
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args)) << '\n';

    OutputDebugString(stream.str().c_str());
}

#endif //MACROS