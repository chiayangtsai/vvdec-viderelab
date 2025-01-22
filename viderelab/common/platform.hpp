
#pragma once

#if !defined(__VIDERELAB_COMMON_PLATFORM_H__)
#define __VIDERELAB_COMMON_PLATFORM_H__

#if defined(__APPLE__)

#include <string>

namespace std {

inline
string format(string format, ...)
{
    return string("CAN'T FORMAT STRINGS ON APPLE: ") + format;
}

inline
string vformat(string format, ...)
{
    return string("CAN'T FORMAT STRINGS ON APPLE: ") + format;
}

} // namespace std

#endif // defined(__APPLE__)

#endif // !defined(__VIDERELAB_COMMON_PLATFORM_H__)
