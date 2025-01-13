
#pragma once

#if !defined(__VIDERELAB_COMMON_IMAGE_FORMATS_H__)
#define __VIDERELAB_COMMON_IMAGE_FORMATS_H__

#include <viderelab/common/color_format.hpp>
#include <viderelab/common/types.hpp>

namespace viderelab {

struct IMAGE_FORMAT
{
    eColorFormat colorFormat{eColorFormat::NONE};
    dim_t dim{};
};

size_t GetImageSize(const IMAGE_FORMAT &imageFormat);
size_t GetImageSizeAligned(const IMAGE_FORMAT &imageFormat, const uint32_t align);

ptrdiff_t GetImageStride(const IMAGE_FORMAT &imageFormat, const size_t imageSize);

size_t GetPlaneCount(const IMAGE_FORMAT &imageFormat);

IMAGE_FORMAT GetPlaneFormat(const IMAGE_FORMAT &imageFormat, const size_t planeIdx);

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_IMAGE_FORMATS_H__)
