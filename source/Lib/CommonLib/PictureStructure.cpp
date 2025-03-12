
#include "Picture.h"

namespace {

bool IsZeroCU(const vvdec::CodingUnit &cu)
{
    return (0 == cu.lwidth()) || (0 == cu.lheight());
}

void PrintCU(viderelab::json::Array &GUs, const vvdec::CodingUnit &cu, const vvdec::Picture& picture)
{
    const auto lumaSize{picture.lumaSize()};
    if ((cu.lumaPos().x >= lumaSize.width) || (cu.lumaPos().y >= lumaSize.height)) {
        return;
    }

    const auto guPicOffsetX{cu.lumaPos().x / 4u};
    const auto guPicOffsetY{cu.lumaPos().y / 4u};
    const auto predMode{(cu.predMode() == vvdec::MODE_INTER) ? "inter" :
        ((cu.predMode() == vvdec::MODE_INTRA) ? "intra" : "ibc")};
    const auto puMode{[&](){
        if (cu.predMode() == vvdec::MODE_INTRA) {
            return std::to_string(cu.intraDir[vvdec::CHANNEL_TYPE_LUMA]);
        } else if (cu.predMode() == vvdec::MODE_IBC) {
            return std::string("ibc");
        } else {
            return std::string("inter");
        }
    }()};

    for (size_t guY{0u}; guY < cu.lumaSize().height / 4u; ++guY) {
        for (size_t guX{0u}; guX < cu.lumaSize().width / 4u; ++guX) {
            viderelab::json::Dict gu;
            GUs.push_back(gu);

            gu.insert("gu_x_idx_pic", guX + guPicOffsetX);
            gu.insert("gu_y_idx_pic", guY + guPicOffsetY);
            gu.insert("gu_x_idx_pu", guX);
            gu.insert("gu_y_idx_pu", guY);
            gu.insert("num_gu_pu_width", cu.lumaSize().width / 4u);
            gu.insert("num_gu_pu_height", cu.lumaSize().height / 4u);
            gu.insert("qp", cu.qp);
            gu.insert("pred_mode", predMode);
            gu.insert("pu_mode", puMode);
        }
    }
}

void PrintCTU(viderelab::json::Array &GUs, const vvdec::CtuData& ctu, const vvdec::Picture& picture)
{
    for (auto cu{ctu.firstCU}; cu != nullptr; cu = cu->next) {
        if (IsZeroCU(*cu)) {
            continue;
        }
        PrintCU(GUs, *cu, picture);
    }
}

void PrintPictureHeader(viderelab::json::Dict &prn, const vvdec::Picture& picture)
{
    viderelab::json::Array GUs;
    prn.insert("gu-info", GUs);

    for (const auto &slice : picture.slices) {
        const auto numCTUs{slice->getNumCtuInSlice()};

        for (size_t ctuIdx{0u}; ctuIdx < numCTUs; ++ctuIdx) {

            auto ctu_id{slice->getCtuAddrInSlice(ctuIdx)};
            const auto &ctu{picture.cs->getCtuData(ctu_id)};

            PrintCTU(GUs, ctu, picture);
        }
    }
}

} // namespace

namespace vvdec
{

void Picture::printStructure(viderelab::json::Dict& prn, Picture& pic)
{
    prn.insert("poc", pic.getPOC());
    prn.insert("width", pic.lumaSize().width);
    prn.insert("height", pic.lumaSize().height);

    PrintPictureHeader(prn, pic);

} // void Picture::printStructure() const

} // namespace vvdec
