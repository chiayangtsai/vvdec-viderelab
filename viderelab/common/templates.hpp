
#pragma once

#if !defined(__VIDERELAB_COMMON_TEMPLATES_H__)
#define __VIDERELAB_COMMON_TEMPLATES_H__

#include <cstdint>
#include <cstddef>

namespace viderelab {

template <typename ...Args>
void UNUSED(Args&&...) {}

template <typename T> inline
T AlignValue(const T value, const uint32_t log2alignment) {
    static_assert(std::is_integral_v<T> == true, "T must be an integral type");

    const auto alignment{1u << log2alignment};
    return (value + alignment - 1u) & ~(static_cast<T>(alignment - 1u));
}

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_TEMPLATES_H__)
