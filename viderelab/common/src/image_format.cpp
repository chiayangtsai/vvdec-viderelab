
#include <viderelab/common/inc/image_format.hpp>

#include <viderelab/common/exception.hpp>
#include <viderelab/common/templates.hpp>

#include <array>
#include <format>
#include <stdexcept>

namespace viderelab {

namespace {

size_t DivideImageSize(const size_t size, const size_t divisor)
{
    if (size % divisor) {
        throw Exception("DivideImageSize: the size {} is not divisible by the divisor {}", size, divisor);
    }

    return size / divisor;
}

size_t GetYUV420ImageSize(const IMAGE_FORMAT &imageFormat, const uint32_t log2align)
{
    const size_t pitchY{AlignValue<size_t>(imageFormat.dim.width, log2align)};

    return pitchY * imageFormat.dim.height * 3u / 2u;
}

size_t GetYUV420ImageStride(const IMAGE_FORMAT &imageFormat, const size_t imageSize)
{
    return DivideImageSize(imageSize * 2u, 3u * imageFormat.dim.height);
}

IMAGE_FORMAT GetYUV420PlaneFormat(const IMAGE_FORMAT &imageFormat, const size_t planeIdx)
{
    IMAGE_FORMAT planeFormat{imageFormat};
    if (0u == planeIdx) {
        planeFormat.colorFormat = eColorFormat::YUV400;
        return planeFormat;
    } else {
        planeFormat.colorFormat = eColorFormat::YUV400;
        planeFormat.dim /= 2u;
        return planeFormat;
    }
}

ptrdiff_t GetYUV420PlaneSize(const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx)
{
    return (imageStride * imageFormat.dim.height) / ((0u == planeIdx) ? (1u) : (4u));
}

ptrdiff_t GetYUV420PlaneStride(const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx)
{
    UNUSED(imageFormat);
    return (0u == planeIdx) ? imageStride : imageStride / 2u;
}

const std::array<IMAGE_DESC, 9u> Descriptions{{
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::I420,
        .numPlanes= 3u,
#else
        .colorFormat{eColorFormat::I420},
        .numPlanes{3u},
#endif
        .GetImageSize{GetYUV420ImageSize},
        .GetImageStride{GetYUV420ImageStride},
        .GetPlaneFormat{GetYUV420PlaneFormat},
        .GetPlaneSize{GetYUV420PlaneSize},
        .GetPlaneStride{GetYUV420PlaneStride}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::NV12,
        .numPlanes= 2u,
#else
        .colorFormat{eColorFormat::NV12},
        .numPlanes{2u},
#endif
        .GetImageSize{GetYUV420ImageSize},
        .GetImageStride{GetYUV420ImageStride},
        .GetPlaneFormat{[](const IMAGE_FORMAT &imageFormat, const size_t planeIdx) -> IMAGE_FORMAT {
            IMAGE_FORMAT planeFormat{imageFormat};
            if (0u == planeIdx) {
                planeFormat.colorFormat = eColorFormat::YUV400;
                return planeFormat;
            } else {
                planeFormat.colorFormat = eColorFormat::UV;
                planeFormat.dim /= 2u;
                return planeFormat;
            }
            }},
        .GetPlaneSize{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            return (imageStride * imageFormat.dim.height) / ((0u == planeIdx) ? (1u) : (2u));
            }},
        .GetPlaneStride{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(imageFormat, planeIdx);
            return imageStride;
            }}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::UYVY,
        .numPlanes= 1u,

#else
        .colorFormat{eColorFormat::UYVY},
        .numPlanes{1u},
#endif
        .GetImageSize{[](const IMAGE_FORMAT &imageFormat, const uint32_t log2align) -> size_t {
            return AlignValue<size_t>(imageFormat.dim.width * 2u, log2align) * imageFormat.dim.height;
            }},
        .GetImageStride{[](const IMAGE_FORMAT &imageFormat, const size_t imageSize) -> ptrdiff_t {
            return DivideImageSize(imageSize, imageFormat.dim.height * 2u);
            }},
        .GetPlaneFormat{[](const IMAGE_FORMAT &imageFormat, const size_t planeIdx) -> IMAGE_FORMAT {
            UNUSED(planeIdx);
            return imageFormat;
            }},
        .GetPlaneSize{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(planeIdx);
            return imageStride * imageFormat.dim.height;
            }},
        .GetPlaneStride{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(imageFormat, planeIdx);
            return imageStride;
            }}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::YUV400,
        .numPlanes= 1u,

#else
        .colorFormat{eColorFormat::YUV400},
        .numPlanes{1u},
#endif
        .GetImageSize{[](const IMAGE_FORMAT &imageFormat, const uint32_t log2align) -> size_t {
            return AlignValue<size_t>(imageFormat.dim.width, log2align) * imageFormat.dim.height;
            }},
        .GetImageStride{[](const IMAGE_FORMAT &imageFormat, const size_t imageSize) -> ptrdiff_t {
            return DivideImageSize(imageSize, imageFormat.dim.height);
            }},
        .GetPlaneFormat{[](const IMAGE_FORMAT &imageFormat, const size_t planeIdx) -> IMAGE_FORMAT {
            UNUSED(planeIdx);
            return imageFormat;
            }},
        .GetPlaneSize{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(planeIdx);
            return imageStride * imageFormat.dim.height;
            }},
        .GetPlaneStride{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(imageFormat, planeIdx);
            return imageStride;
            }}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::YUV420,
        .numPlanes= 3u,

#else
        .colorFormat{eColorFormat::YUV420},
        .numPlanes{3u},
#endif
        .GetImageSize{GetYUV420ImageSize},
        .GetImageStride{GetYUV420ImageStride},
        .GetPlaneFormat{GetYUV420PlaneFormat},
        .GetPlaneSize{GetYUV420PlaneSize},
        .GetPlaneStride{GetYUV420PlaneStride}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::YUV422,
        .numPlanes=3u,

#else
        .colorFormat{eColorFormat::YUV422},
        .numPlanes{3u},
#endif
        .GetImageSize{[](const IMAGE_FORMAT &imageFormat, const uint32_t log2align) -> size_t {
            const size_t pitchY{AlignValue<size_t>(imageFormat.dim.width, log2align)};
            return pitchY * imageFormat.dim.height + pitchY * imageFormat.dim.height / 2u;
            }},
        .GetImageStride{[](const IMAGE_FORMAT &imageFormat, const size_t imageSize) -> ptrdiff_t {
            return DivideImageSize(imageSize, 2u * imageFormat.dim.height);
            }},
        .GetPlaneFormat{[](const IMAGE_FORMAT &imageFormat, const size_t planeIdx) -> IMAGE_FORMAT {
            IMAGE_FORMAT planeFormat{imageFormat};
            if (0u == planeIdx) {
                planeFormat.colorFormat = eColorFormat::YUV400;
                return planeFormat;
            } else {
                planeFormat.colorFormat = eColorFormat::YUV400;
                planeFormat.dim.width /= 2u;
                return planeFormat;
            }
            }},
        .GetPlaneSize{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            return (imageStride * imageFormat.dim.height) / ((0u == planeIdx) ? (1u) : (2u));
            }},
        .GetPlaneStride{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(imageFormat);
            return imageStride / ((0u == planeIdx) ? 1u : 2u);
            }}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::YUV444,
        .numPlanes= 3u,
#else
        .colorFormat{eColorFormat::YUV444},
        .numPlanes{3u},
#endif
        .GetImageSize{[](const IMAGE_FORMAT &imageFormat, const uint32_t log2align) -> size_t {
            return AlignValue<size_t>(imageFormat.dim.width, log2align) * imageFormat.dim.height * 3u;
            }},
        .GetImageStride{[](const IMAGE_FORMAT &imageFormat, const size_t imageSize) -> ptrdiff_t {
            return DivideImageSize(imageSize, 3u * imageFormat.dim.height);
            }},
        .GetPlaneFormat{[](const IMAGE_FORMAT &imageFormat, const size_t planeIdx) -> IMAGE_FORMAT {
            UNUSED(planeIdx);
            IMAGE_FORMAT planeFormat{imageFormat};
            planeFormat.colorFormat = eColorFormat::YUV400;
            return planeFormat;
            }},
        .GetPlaneSize{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(planeIdx);
            return imageStride * imageFormat.dim.height;
            }},
        .GetPlaneStride{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(imageFormat, planeIdx);
            return imageStride;
            }}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::YUY2,
        .numPlanes= 1u,
#else
        .colorFormat{eColorFormat::YUY2},
        .numPlanes{1u},
#endif
        .GetImageSize{[](const IMAGE_FORMAT &imageFormat, const uint32_t log2align) -> size_t {
            return AlignValue<size_t>(imageFormat.dim.width * 2u, log2align) * imageFormat.dim.height;
            }},
        .GetImageStride{[](const IMAGE_FORMAT &imageFormat, const size_t imageSize) -> ptrdiff_t {
            return DivideImageSize(imageSize, imageFormat.dim.height * 2u);
            }},
        .GetPlaneFormat{[](const IMAGE_FORMAT &imageFormat, const size_t planeIdx) -> IMAGE_FORMAT {
            UNUSED(planeIdx);
            return imageFormat;
            }},
        .GetPlaneSize{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(planeIdx);
            return imageStride * imageFormat.dim.height;
            }},
        .GetPlaneStride{[](const IMAGE_FORMAT &imageFormat, const ptrdiff_t imageStride, const size_t planeIdx) -> ptrdiff_t {
            UNUSED(imageFormat, planeIdx);
            return imageStride;
            }}
    },
    {
#if MACOS_C20_WORKAROUND
        .colorFormat= eColorFormat::YV12,
        .numPlanes= 3u,
#else
        .colorFormat{eColorFormat::YV12},
        .numPlanes{3u},
#endif
        .GetImageSize{GetYUV420ImageSize},
        .GetImageStride{GetYUV420ImageStride},
        .GetPlaneFormat{GetYUV420PlaneFormat},
        .GetPlaneSize{GetYUV420PlaneSize},
        .GetPlaneStride{GetYUV420PlaneStride}
    }
}};

} // namespace

const IMAGE_DESC &GetImageDesc(const eColorFormat &colorFormat)
{
    for (const auto &desc : Descriptions) {
        if (desc.colorFormat == colorFormat) {
            return desc;
        }
    }

    throw Exception("GetImageDesc: the color format {} is not supported", to_string(colorFormat));

} // const IMAGE_DESC &GetImageDesc(const eColorFormat &colorFormat)

size_t GetImageSize(const IMAGE_FORMAT &imageFormat)
{
    return GetImageSizeAligned(imageFormat, 1u);
}

size_t GetImageSizeAligned(const IMAGE_FORMAT &imageFormat, const uint32_t log2align)
{
    const auto &desc{GetImageDesc(imageFormat.colorFormat)};

    return desc.GetImageSize(imageFormat, log2align);
}

ptrdiff_t GetImageStride(const IMAGE_FORMAT &imageFormat, const size_t imageSize)
{
    const auto &desc{GetImageDesc(imageFormat.colorFormat)};

    return desc.GetImageStride(imageFormat, imageSize);
}

size_t GetPlaneCount(const IMAGE_FORMAT &imageFormat)
{
    return GetImageDesc(imageFormat.colorFormat).numPlanes;
}

IMAGE_FORMAT GetPlaneFormat(const IMAGE_FORMAT &imageFormat, const size_t planeIdx)
{
    const auto &desc{GetImageDesc(imageFormat.colorFormat)};

    if (planeIdx >= desc.numPlanes) {
        throw Exception("GetPlaneFormat: the plane index {} is out of range for format {}", planeIdx,
            to_string(imageFormat.colorFormat));
    }

    return desc.GetPlaneFormat(imageFormat, planeIdx);
}

} // namespace viderelab
