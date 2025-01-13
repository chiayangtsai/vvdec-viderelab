
#pragma once

#if !defined(__VIDERELAB_DEMUXERS_YUV_DEMUXER_H__)
#define __VIDERELAB_DEMUXERS_YUV_DEMUXER_H__

#include <viderelab/common/frame.hpp>

#include <istream>
#include <memory>

namespace viderelab {

class YUVDemuxer
{
public:
    struct Parameters
    {
        // when format is not set, then frames may have arbitrary format/size
        IMAGE_FORMAT format;
    };

    static
    std::unique_ptr<YUVDemuxer> Create(const Parameters& params, std::unique_ptr<std::istream> file);

    virtual ~YUVDemuxer() {};

    std::shared_ptr<Frame> ReadFrame();

private:
    YUVDemuxer(const Parameters& params, std::unique_ptr<std::istream> file);

    const IMAGE_FORMAT m_format{};
    const size_t m_frameSize{};
    std::unique_ptr<std::istream> m_file;
};

} // namespace viderelab

#endif // !defined(__VIDERELAB_DEMUXERS_YUV_DEMUXER_H__)
