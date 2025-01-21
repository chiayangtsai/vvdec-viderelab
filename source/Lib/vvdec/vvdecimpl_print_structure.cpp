/* -----------------------------------------------------------------------------
The copyright in this software is being made available under the Clear BSD
License, included below. No patent rights, trademark rights and/or
other Intellectual Property Rights other than the copyrights concerning
the Software are granted under this license.

The Clear BSD License

Copyright (c) 2018-2024, Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V. & The VVdeC Authors.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted (subject to the limitations in the disclaimer below) provided that
the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

     * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     * Neither the name of the copyright holder nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.


------------------------------------------------------------------------------------------- */

#include <string>

#if defined( __linux__ )
#include <malloc.h>
#endif

#include "vvdecimpl.h"
#include "vvdec/version.h"
#include "DecoderLib/NALread.h"
#include "CommonLib/CommonDef.h"
#include "CommonLib/x86/CommonDefX86.h"
#include "CommonLib/arm/CommonDefARM.h"

#if ENABLE_FILM_GRAIN
#  include "vvdec/sei.h"
#  include "FilmGrain/FilmGrain.h"
#endif   // ENABLE_FILM_GRAIN

#include <viderelab/common/platform.hpp>

#include <format>

namespace vvdec
{

namespace {

std::string ChromaFormatToString(const ChromaFormat chromaFormat)
{
    switch (chromaFormat)
    {
    case CHROMA_400: return "400";
    case CHROMA_420: return "420";
    case CHROMA_422: return "422";
    case CHROMA_444: return "444";
    default: return "UNKNOWN";
    }
}

std::string IntraPredModeToString(const int8_t intraPredMode)
{
    switch (intraPredMode)
    {
        case PLANAR_IDX: return "PLANAR";
        case DC_IDX: return "DC";
        case HOR_IDX: return "HORIZONTAL";
        case DIA_IDX: return "DIAGONAL";
        case VER_IDX: return "VERTICAL";
        case VDIA_IDX: return "VDIAGONAL";
        default: return std::format("{}", intraPredMode);
    }
}

void PrintPictureProperties(viderelab::json::Dict &prn, const vvdecFrame& frame, const Picture& picture)
{
    {
        auto prnDim{prn.StartDict("dimension")};
        prnDim.AddValue("width", picture.cs->pcv->lumaWidth);
        prnDim.AddValue("height", picture.cs->pcv->lumaHeight);
    }
    prn.AddValue("chromaFormat", ChromaFormatToString(picture.cs->pcv->chrFormat));
    prn.AddValue("bitDepth", picture.cs->sps->getBitDepth());
}

void PrintTU(viderelab::json::Dict &prn, const TransformUnit &tu)
{
}

void PrintCU(viderelab::json::Dict &prn, const CodingUnit &cu)
{
    prn.AddValue("cu_id", cu.idx);
    {
        auto prnPos{prn.StartDict("cu_pos")};
        prnPos.AddValue("x", cu.lumaPos().x);
        prnPos.AddValue("y", cu.lumaPos().y);
    }

    {
        auto prnSize{prn.StartDict("cu_size")};
        prnSize.AddValue("width", cu.lwidth());
        prnSize.AddValue("height", cu.lheight());
    }

    prn.AddValue("cu_qp", (uint32_t) cu.qp);

    {
        auto prnPredMode{prn.StartDict("pred_mode")};
        if (cu.predMode() == MODE_INTRA) {
            auto prnIntraPredMode{prnPredMode.StartDict(" intra_pred_mode")};
            if (cu.chType() == CHANNEL_TYPE_LUMA) {
                prnIntraPredMode.AddValue(" intra_luma_pred_mode", IntraPredModeToString(cu.intraDir[0]));
            } else {
                prnIntraPredMode.AddValue(" intra_chroma_pred_mode", IntraPredModeToString(cu.intraDir[0]));
            }
        } else {
            throw std::runtime_error("Inter prediction mode not implemented");
        }
    }
}

void PrintCTU(viderelab::json::Dict &prn, const CtuData& ctu)
{
    prn.AddValue("ctu_id", ctu.ctuIdx);

    const auto ctuSize{ctu.sps->getCTUSize()};
    {
        auto prnCTUPos{prn.StartDict("ctu_pos")};
        prnCTUPos.AddValue("x", ctu.colIdx * ctuSize);
        prnCTUPos.AddValue("y", ctu.lineIdx * ctuSize);
    }

    {
        auto prnCTUSize{prn.StartDict("ctu_size")};
        prnCTUSize.AddValue("width", ctuSize);
        prnCTUSize.AddValue("height", ctuSize);
    }

    //prn.AddValue("ctu_qp",
    //prn.AddValue("cu_split_flags",

    auto prnPartModes{prn.StartDict("cu_part_modes")};
    prnPartModes.AddValue("numCUs", ctu.numCUs);
    auto prnCUs{prnPartModes.StartArray("CUs")};

    for (auto cu{ctu.firstCU}; cu != nullptr; cu = cu->next) {
        auto prnCU{prnCUs.StartDict()};
        PrintCU(prnCU, *cu);
    }
}

void PrintPictureHeader(viderelab::json::Dict &prn, const vvdecFrame& frame, const Picture& picture)
{
    prn.AddValue("numSlices", picture.slices.size());

    auto prnSlices{prn.StartArray("slices")};
    for (const auto &slice : picture.slices) {
        auto prnSlice{prnSlices.StartDict()};

        auto prnSliceHeader{prnSlice.StartDict("slice_header")};
        auto prnSliceSegmentLayerRbsp{prnSliceHeader.StartDict("slice_segment_layer_rbsp")};

        const auto numCTUs{slice->getNumCtuInSlice()};
        prnSliceSegmentLayerRbsp.AddValue("numCTUs", numCTUs);

        auto prnCTUs{prnSliceSegmentLayerRbsp.StartArray("CTUs")};
        for (size_t ctuIdx{0u}; ctuIdx < numCTUs; ++ctuIdx) {
            auto prnCTU{prnCTUs.StartDict()};
            auto ctu_id{slice->getCtuAddrInSlice(ctuIdx)};
            const auto &ctu{picture.cs->getCtuData(ctu_id)};

            PrintCTU(prnCTU, ctu);
        }
    }
}

void PrintPicture(viderelab::json::Dict &prn, const vvdecFrame& frame, const Picture& picture)
{
    prn.AddValue("frame_index", frame.sequenceNumber);
    {
        auto prnProps{prn.StartDict("properties")};
        PrintPictureProperties(prnProps, frame, picture);
    }

    auto prnPictureHeader{prn.StartDict("picture_header")};
    PrintPictureHeader(prnPictureHeader, frame, picture);

} // void PrintPicture(viderelab::json::Dict &prn, const vvdecFrame& frame, const Picture& picture)

} // namespace

int VVDecImpl::printPicStructure(viderelab::json::Dict &prnFrame, const vvdecFrame* frame) const
{
  if( !m_bInitialized )      { return VVDEC_ERR_INITIALIZE; }

  if( nullptr == frame )
  {
    m_cErrorString = "printPicStructure: frame is null\n";
    return VVDEC_ERR_PARAMETER;
  }

  Picture* picture{nullptr};
  for( auto& entry: m_rcFrameList )
  {
    if( frame == &std::get<vvdecFrame>( entry ) )
    {
      picture = std::get<Picture*>( entry );
      break;
    }
  }

  if( picture == nullptr )
  {
    msg(VERBOSE, "findFrameSei: cannot find pictue in internal list.\n");
    return VVDEC_ERR_PARAMETER;
  }

  PrintPicture(prnFrame, *frame, *picture);

  return VVDEC_OK;
}

}   // namespace vvdec
