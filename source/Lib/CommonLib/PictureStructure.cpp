
#include "Picture.h"

namespace {

bool IsZeroCU(const vvdec::CodingUnit &cu)
{
    return (0 == cu.lwidth()) || (0 == cu.lheight());
}

void PrintCU(viderelab::json::Dict &prn, const vvdec::CodingUnit &cu)
{
    prn.insert("cu_id", cu.idx - 1);
    prn.insert("x", cu.lumaPos().x);
    prn.insert("y", cu.lumaPos().y);
    prn.insert("width", cu.lwidth());
    prn.insert("height", cu.lheight());

    prn.insert("cu_qp", (uint32_t) cu.qp);
    prn.insert("pred_mode", (cu.predMode() == vvdec::MODE_INTER) ? "inter" : "intra");

    if (cu.predMode() == vvdec::MODE_INTER) {
        std::cout << "inter cu\n";
    } else {
        prn.insert("num_pus", 1u);
        viderelab::json::Array prnPUs;
        prn.insert("pu-info", prnPUs);
        viderelab::json::Dict pu;
        prnPUs.push_back(pu);
        pu.insert("pu_id", 0u);
        pu.insert("x", cu.lumaPos().x);
        pu.insert("y", cu.lumaPos().y);
        pu.insert("width", cu.lwidth());
        pu.insert("height", cu.lheight());
        if (cu.predMode() == vvdec::MODE_INTRA) {
            pu.insert("pu_mode", static_cast<uint32_t>(cu.intraDir[vvdec::CHANNEL_TYPE_LUMA]));
        } else {
            pu.insert("pu_mode", "ibc");
        }
    }
}

void PrintCTU(viderelab::json::Dict &prn, const vvdec::CtuData& ctu)
{
    prn.insert("ctu_id", ctu.ctuIdx);

    const auto ctuSize{ctu.sps->getCTUSize()};
    prn.insert("x", ctu.colIdx * ctuSize);
    prn.insert("y", ctu.lineIdx * ctuSize);
    prn.insert("width", ctuSize);
    prn.insert("height", ctuSize);

    prn.insert("num_cus", ctu.numCUs);
    viderelab::json::Array prnCUs;
    prn.insert("cu-info", prnCUs);

    for (auto cu{ctu.firstCU}; cu != nullptr; cu = cu->next) {
        if (IsZeroCU(*cu)) {
            continue;
        }
        viderelab::json::Dict prnCU;
        prnCUs.push_back(prnCU);
        PrintCU(prnCU, *cu);
    }
}

void PrintPictureHeader(viderelab::json::Dict &prn, const vvdec::Picture& picture)
{
    prn.insert("num_slices", picture.slices.size());

    viderelab::json::Array prnSlices;
    prn.insert("slice-info", prnSlices);

    for (const auto &slice : picture.slices) {
        viderelab::json::Dict prnSlice;
        prnSlices.push_back(prnSlice);

        prnSlice.insert("slice_qp", slice->getSliceQp());
        const auto numCTUs{slice->getNumCtuInSlice()};
        prnSlice.insert("num_ctus", numCTUs);

        viderelab::json::Array prnCTUs;
        prnSlice.insert("ctu-info", prnCTUs);

        for (size_t ctuIdx{0u}; ctuIdx < numCTUs; ++ctuIdx) {
            viderelab::json::Dict prnCTU;
            prnCTUs.push_back(prnCTU);

            auto ctu_id{slice->getCtuAddrInSlice(ctuIdx)};
            const auto &ctu{picture.cs->getCtuData(ctu_id)};

            PrintCTU(prnCTU, ctu);
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
