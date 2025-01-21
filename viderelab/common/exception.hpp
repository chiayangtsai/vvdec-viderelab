
#pragma once

#if !defined(__VIDERELAB_COMMON_EXCEPTION_H__)
#define __VIDERELAB_COMMON_EXCEPTION_H__

#include <viderelab/common/platform.hpp>

#include <format>
#if defined(__cpp_lib_stacktrace)
#include <stacktrace>
#endif // defined(__cpp_lib_stacktrace)
#include <string>
#include <string_view>
#include <utility>

namespace viderelab {

class Exception
{
public:
    template <typename ...Args>
    Exception(std::string_view msg, Args&&... args) :
        m_msg(FormatExceptionMessage(msg, std::forward<Args>(args)...)) {}

    virtual ~Exception() {}

    auto what() const -> auto {
        return m_msg;
    }

    auto where() const -> auto {
        return m_stacktrace;
    }

private:

    template <typename ...Args> static
    std::string FormatExceptionMessage(std::string_view msg, Args&&... args)
    {
        try {
            return std::vformat(msg, std::make_format_args(args...));
        } catch (const std::exception& e) {
            return std::format("FormatExceptionMessage: {}\nException message: {}.", e.what(), msg);
        }
    }

    const std::string m_msg{};
#if defined(__cpp_lib_stacktrace)
    const std::stacktrace m_stacktrace{std::stacktrace::current()};
#else
    static inline const std::string m_stacktrace{"std::stacktrace requires C++23"};
#endif // defined(__cpp_lib_stacktrace)
};

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_EXCEPTION_H__)
