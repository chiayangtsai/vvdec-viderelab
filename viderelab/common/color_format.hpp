
#pragma once

#if !defined(__VIDERELAB_COMMON_COLOR_FORMATS_H__)
#define __VIDERELAB_COMMON_COLOR_FORMATS_H__

#include <cstdint>
#include <string>

namespace viderelab {

constexpr uint32_t fourcc(const char (&str)[5])
{
    return (uint32_t) str[0] + ((uint32_t) str[1] << 8u) + ((uint32_t) str[2] << 16u) + ((uint32_t) str[3] << 24u);
}

template <const size_t N>
consteval uint32_t const32u(const char (&str)[N])
{
    static_assert(N <= 5, "String is too long for 32-bit integer");

    uint32_t result = 0;
    for (size_t i = 0; str[i] != '\0'; ++i)
    {
        result = result + ((uint32_t) str[i] << (i * 8u));
    }
    return result;
}

template <const size_t N>
consteval uint64_t const64u(const char (&str)[N])
{
    static_assert(N <= 9, "String is too long for 64-bit integer");

    uint64_t result = 0;
    for (size_t i = 0; str[i] != '\0'; ++i)
    {
        result = result + ((uint64_t) str[i] << (i * 8u));
    }
    return result;
}

enum class eColorFormat : uint32_t
{
    NONE = 0,

    I420 = fourcc("I420"),
    NV12 = fourcc("NV12"),
    UYVY = fourcc("UYVY"),
    UV = fourcc("UVUV"), // NV12 second plane
    YUV400 = fourcc("Y400"),
    YUV420 = fourcc("Y420"),
    YUV422 = fourcc("Y422"),
    YUV444 = fourcc("Y444"),
    YUY2 = fourcc("YUY2"),
    YV12 = fourcc("YV12")
};

const std::string to_string(const eColorFormat format);

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_COLOR_FORMATS_H__)
