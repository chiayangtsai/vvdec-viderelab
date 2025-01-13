
#pragma once

#if !defined(__VIDERELAB_COMMON_COLOR_FORMATS_H__)
#define __VIDERELAB_COMMON_COLOR_FORMATS_H__

#include <cstdint>
#include <string>

namespace viderelab {

enum class eColorFormat : uint32_t
{
    NONE = 0,

    I420 = 'I420',
    NV12 = 'NV12',
    UYVY = 'UYVY',
    UV = 'UVUV', // NV12 second plane
    YUV400 = 'Y400',
    YUV420 = 'Y420',
    YUV422 = 'Y422',
    YUV444 = 'Y444',
    YUY2 = 'YUY2',
    YV12 = 'YV12'
};

const std::string to_string(const eColorFormat format);

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_COLOR_FORMATS_H__)
