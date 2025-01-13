
#pragma once

#if !defined(__VIDERELAB_COMMON_TYPES_H__)
#define __VIDERELAB_COMMON_TYPES_H__

#include <cstdint>
#include <cstddef>

namespace viderelab {

template <typename T>
struct Dimension {
    T width{0};
    T height{0};

    inline
    Dimension<T> &operator /= (const T divisor) {
        width /= divisor;
        height /= divisor;
        return *this;
    }

};
using dim_t = Dimension<std::uint32_t>;

enum class eTimeDomain : uint32_t {
    Default = 0u,

    Host = 1u,
    DTS = 2u,
    PTS = 3u,
    User = 4u,

    Count = 5u
};
using time_t = uint64_t;

std::string to_string(const eTimeDomain &timeDomain);

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_TYPES_H__)
