
#pragma once

#if !defined(__VIDERELAB_COMMON_IMAGE_H__)
#define __VIDERELAB_COMMON_IMAGE_H__

#include <viderelab/common/image_format.hpp>

#include <array>
#include <memory>
#include <span>

namespace viderelab {

template <typename T>
class plane_t
{
public:
    plane_t(const IMAGE_FORMAT format, std::span<T> plane, const ptrdiff_t stride);

    inline
    auto Format() const {
        return m_format;
    }

    inline
    std::span<T> Data() const {
        return m_plane;
    }

    inline
    ptrdiff_t Stride() const {
        return m_stride;
    }

private:
    IMAGE_FORMAT m_format{}; // format of the plane
    std::span<T> m_plane{};  // data of the plane
    ptrdiff_t m_stride{0}; // stride of the plane
};
using Plane = plane_t<uint8_t>;

template <typename T>
class image_t
{
public:
    image_t(const IMAGE_FORMAT format, std::span<T> data, const ptrdiff_t stride) :
        m_format{format},
        m_data{data},
        m_stride{stride} {}
    image_t(const IMAGE_FORMAT format, std::unique_ptr<T []> buffer, const size_t size) :
        image_t(format, std::span<T>(buffer.get(), size), GetImageStride(format, size)) {
        m_buffer = std::move(buffer);
    }

    inline
    auto Format() const {
        return m_format;
    }

    inline
    size_t NumPlanes() const {
        return GetPlaneCount(m_format);
    }

    inline
    plane_t<T> Plane(const size_t planeIdx) const;

private:

    IMAGE_FORMAT m_format{}; // format of the image
    std::span<T> m_data{}; // image planes
    ptrdiff_t m_stride{0}; // stride of the image

    std::unique_ptr<T []> m_buffer{}; // buffer for the image data
};
using Image = image_t<uint8_t>;

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_IMAGE_H__)
