
#pragma once

#if !defined(__VIDERELAB_COMMON_INC_IMAGE_FORMATS_H__)
#define __VIDERELAB_COMMON_INC_IMAGE_FORMATS_H__

#include <viderelab/common/image_format.hpp>

#include <functional>

namespace viderelab {

struct IMAGE_DESC
{
    eColorFormat colorFormat;
    size_t numPlanes;

    std::function<size_t(const IMAGE_FORMAT &, const uint32_t log2align)> GetImageSize;
    std::function<ptrdiff_t(const IMAGE_FORMAT &, const size_t imageSize)> GetImageStride;

    std::function<IMAGE_FORMAT(const IMAGE_FORMAT &, const size_t planeIdx)> GetPlaneFormat;
    std::function<size_t(const IMAGE_FORMAT &, const ptrdiff_t stride, const size_t planeIdx)> GetPlaneSize;
    std::function<ptrdiff_t(const IMAGE_FORMAT &, const ptrdiff_t stride, const size_t planeIdx)> GetPlaneStride;
};

const IMAGE_DESC &GetImageDesc(const eColorFormat &colorFormat);

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_INC_IMAGE_FORMATS_H__)
