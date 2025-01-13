
#pragma once

#if !defined(__VIDERELAB_COMMON_FRAME_H__)
#define __VIDERELAB_COMMON_FRAME_H__

#include <viderelab/common/exception.hpp>
#include <viderelab/common/image.hpp>

#include <optional>
#include <vector>

namespace viderelab {

class Frame
{
public:
    Frame(std::shared_ptr<image_t<uint8_t>> image) :
        m_image8u(image) {};
    Frame(std::shared_ptr<image_t<uint16_t>> image) :
        m_image16u(image) {};

    template<typename data_t>
    image_t<data_t> Image() const;

    inline
    void SetTime(const eTimeDomain domain, const time_t time) {
        if (domain >= eTimeDomain::Count) {
            throw Exception("Frame::SetTime: Invalid time domain: {}", to_string(domain));
        }

        m_times[static_cast<size_t>(domain)] = time;
    }
    inline
    time_t GetTime(const eTimeDomain domain) {
        if (domain >= eTimeDomain::Count) {
            throw Exception("Frame::GetTime: Invalid time domain: {}", to_string(domain));
        }

        return m_times[static_cast<size_t>(domain)];
    }

    template <typename data_t> inline
    std::optional<image_t<data_t>> Image() const {
        if constexpr (std::is_same_v<data_t, uint8_t>) {
            return m_image8u;
        } else if constexpr (std::is_same_v<data_t, uint16_t>) {
            return m_image16u;
        }
        return std::nullopt;
    }

private:
    std::array<time_t, static_cast<size_t>(eTimeDomain::Count)> m_times{};

    std::shared_ptr<image_t<uint8_t>> m_image8u;
    std::shared_ptr<image_t<uint16_t>> m_image16u;
};

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_FRAME_H__)
