
#include <viderelab/common/color_format.hpp>

namespace viderelab {

const std::string to_string(const eColorFormat format)
{
    switch (format)
    {
        case eColorFormat::NONE: return "NONE";
        case eColorFormat::I420: return "I420";
        case eColorFormat::NV12: return "NV12";
        case eColorFormat::UYVY: return "UYVY";
        case eColorFormat::YUV400: return "YUV400";
        case eColorFormat::YUV420: return "YUV420";
        case eColorFormat::YUV422: return "YUV422";
        case eColorFormat::YUV444: return "YUV444";
        case eColorFormat::YUY2: return "YUY2";
        case eColorFormat::YV12: return "YV12";
        default: return "UNKNOWN";
    }
}

} // namespace viderelab
