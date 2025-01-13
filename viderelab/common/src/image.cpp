
#include <viderelab/common/image.hpp>

#include <viderelab/common/inc/image_format.hpp>

namespace viderelab {

template <typename T>
plane_t<T> image_t<T>::Plane(const size_t planeIdx) const
{
    auto desc{GetImageDesc(m_format.colorFormat)};

    auto planeFormat{desc.GetPlaneFormat(m_format, planeIdx)};


    auto stride{desc.GetPlaneStride(m_format, m_stride, planeIdx)};
    auto plane{m_data};
    for (size_t i{0u}; i < planeIdx; ++i) {
        plane = plane.subspan(desc.GetPlaneSize(m_format, m_stride, i));
    }
    plane = plane.subspan(0, desc.GetPlaneSize(m_format, m_stride, planeIdx));

    return plane_t<T>{planeFormat, plane, stride};

} // plane_t<T> image_t<T>::Plane(const size_t planeIdx) const

template <>
plane_t<uint8_t> image_t<uint8_t>::Plane(const size_t planeIdx) const;
template <>
plane_t<uint16_t> image_t<uint16_t>::Plane(const size_t planeIdx) const;

} // namespace viderelab
