
#include <viderelab/demuxers/yuv_demuxer.hpp>

#include <format>

namespace viderelab
{

std::unique_ptr<YUVDemuxer> YUVDemuxer::Create(const Parameters& params, std::unique_ptr<std::istream> file)
{
    if ((eColorFormat::NONE == params.format.colorFormat) ||
        (0u == params.format.dim.width) ||
        (0u == params.format.dim.height)) {
        throw Exception("YUVDemuxer::Create: invalid format: color format {}, width {}, height {}",
            to_string(params.format.colorFormat), params.format.dim.width, params.format.dim.height);
    }
    if (!file) {
        return nullptr;
    }

    return std::unique_ptr<YUVDemuxer>(new (std::nothrow) YUVDemuxer(params, std::move(file)));
}

YUVDemuxer::YUVDemuxer(const YUVDemuxer::Parameters& params, std::unique_ptr<std::istream> file) :
    m_format{params.format},
    m_frameSize{GetImageSize(params.format)},
    m_file{std::move(file)}
{
}

std::shared_ptr<Frame> YUVDemuxer::ReadFrame()
{
    auto planes{std::make_unique<uint8_t []>(m_frameSize)};

    m_file->read(reinterpret_cast<char*>(planes.get()), m_frameSize);
    if (!m_file) {
        return nullptr;
    }

    auto img{std::make_shared<Image>(m_format, std::move(planes), m_frameSize)};

    return std::make_shared<Frame>(img);
}

} // namespace viderelab
