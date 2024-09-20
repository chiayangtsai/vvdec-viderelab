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

#include <format>

namespace vvdec
{

namespace JSON {

constexpr size_t TAB_INC{2u};

class Item
{
protected:
    Item(std::ostream& s, const size_t tab) :
        m_s(s),
        m_tab(tab) {}

    inline
    std::ostream& s() {
        return m_s;
    }

    inline
    size_t Tab() const {
        return m_tab;
    }

    inline
    size_t NumValues() const {
        return m_numValues;
    }

    inline
    void IncNumValues() {
        ++m_numValues;
    }

private:
    std::ostream& m_s;
    const size_t m_tab{};
    size_t m_numValues{0u};
};

class Dict : public Item
{
protected:
    class Array;

public:
    Dict(std::ostream& s) :
        Dict(s, TAB_INC) {}

    ~Dict()
    {
        PrintTab(Item::Tab() - TAB_INC, eAction::CLOSING);
        Item::s() << "}";
    }

    void PrintInt(const std::string& name, const int64_t i)
    {
        PrintTab(Item::Tab());
        Item::s() << PrintName(name) << " : " << i;
    }

    void PrintString(const std::string& name, const std::string& value)
    {
        PrintTab(Item::Tab());
        Item::s() << PrintName(name) << " : \"" << value << "\"";
    }

    Dict StartDict(const std::string& name)
    {
        PrintTab(Item::Tab());
        Item::s() << PrintName(name) << " : ";
        return Dict(Item::s(), Item::Tab() + TAB_INC);
    }

    Array StartArray(const std::string& name)
    {
        PrintTab(Item::Tab());
        Item::s() << PrintName(name) << " : ";
        return Array(Item::s(), Item::Tab() + TAB_INC);
    }

protected:
    enum class eAction : uint32_t
    {
        NONE,
        CLOSING
    };

    class Array : public Item
    {
    public:
        ~Array()
        {
            PrintTab(Item::Tab() - TAB_INC, eAction::CLOSING);
            Item::s() << "]";
        }

        Dict StartDict()
        {
            PrintTab(Item::Tab());
            return Dict(Item::s(), Item::Tab() + TAB_INC);
        }

    protected:
        friend class Dict;

        Array(std::ostream& s, const size_t tab) :
            Item(s, tab)
        {
            Item::s() << "[";
        }

    private:

        void PrintTab(const size_t tab, const eAction action = eAction::NONE)
        {
            if ((eAction::NONE == action) && (Item::NumValues())) {
                Item::s() << ",";
            }
            Item::s() << std::endl;
            for (size_t i{0u}; i < tab; ++i) {
                Item::s() << " ";
            } 

            Item::IncNumValues();
        }
    };

    friend class Array;

    Dict(std::ostream& s, const size_t tab) :
        Item(s, tab)
    {
        Item::s() << "{";
    }

private:
    void PrintTab(const size_t tab, const eAction action = eAction::NONE)
    {
        if ((eAction::NONE == action) && (Item::NumValues())) {
            Item::s() << ",";
        }
        Item::s() << std::endl;
        for (size_t i{0u}; i < tab; ++i) {
            Item::s() << " ";
        } 

        Item::IncNumValues();
    }

    std::string PrintName(const std::string& name)
    {
        return "\"" + name + "\"";
    }
};

} // namespace JSON

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

void PrintPictureProperties(JSON::Dict &prn, const vvdecFrame& frame, const Picture& picture)
{
    {
        auto prnDim{prn.StartDict("dimension")};
        prnDim.PrintInt("width", picture.cs->pcv->lumaWidth);
        prnDim.PrintInt("height", picture.cs->pcv->lumaHeight);
    }
    prn.PrintString("chromaFormat", ChromaFormatToString(picture.cs->pcv->chrFormat));
    prn.PrintInt("bitDepth", picture.cs->sps->getBitDepth());
}

void PrintCU(JSON::Dict &prn, const CodingUnit& cu)
{
    prn.PrintInt("cuIdx", cu.idx);
    {
        auto prnPos{prn.StartDict("position")};
        prnPos.PrintInt("x", cu.lumaPos().x);
        prnPos.PrintInt("y", cu.lumaPos().y);
    }
    {
        auto prnSize{prn.StartDict("size")};
        prnSize.PrintInt("width", cu.lwidth());
        prnSize.PrintInt("height", cu.lheight());
    }
    prn.PrintString("channelType", (cu.chType() == CHANNEL_TYPE_LUMA) ? "luma" : "chroma");
    prn.PrintString("predMode", (cu.predMode() == MODE_INTRA) ? "intra" : "inter");
    if (cu.predMode() == MODE_INTRA) {
        if (cu.chType() == CHANNEL_TYPE_LUMA) {
            prn.PrintString("intraMode", IntraPredModeToString(cu.intraDir[0]));
        } else {
            prn.PrintString("intraMode", IntraPredModeToString(cu.intraDir[1]));
        }
    }
}

void PrintCTU(JSON::Dict &prn, const CtuData& ctu)
{
    prn.PrintInt("ctuIdx", ctu.ctuIdx);
    prn.PrintInt("colIdx", ctu.colIdx);
    prn.PrintInt("lineIdx", ctu.lineIdx);
    prn.PrintInt("numCUs", ctu.numCUs);
    prn.PrintInt("numTUs", ctu.numTUs);

    auto prnCUs{prn.StartArray("CUs")};
    auto cu{ctu.firstCU};
    for (size_t i{0u}; i < ctu.numCUs; ++i) {
        auto prnCU{prnCUs.StartDict()};
        PrintCU(prnCU, *cu);
        cu = cu->next;
    }
}

void PrintPicture(JSON::Dict &prn, const vvdecFrame& frame, const Picture& picture)
{
    prn.PrintInt("index", frame.sequenceNumber);
    {
        auto prnProps{prn.StartDict("properties")};
        PrintPictureProperties(prnProps, frame, picture);
    }
    const size_t sizeInCTUs{picture.cs->pcv->sizeInCtus};
    prn.PrintInt("sizeInCTUs", sizeInCTUs);

    auto prnCTUs{prn.StartArray("CTUs")};

    for (size_t i{0u}; i < 2/*sizeInCTUs*/; ++i) {
        auto prnCTU{prnCTUs.StartDict()};
        const auto ctu{picture.cs->getCtuData((int)i)};

        PrintCTU(prnCTU, ctu);
    }

} // void PrintPicture(JSON::Dict &prn, const vvdecFrame& frame, const Picture& picture)

} // namespace

int VVDecImpl::printPicStructure(std::ostream& s, const vvdecFrame* frame) const
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

  {
    JSON::Dict prn(s);
    auto prnFrames{prn.StartArray("frames")};
    auto prnFrame{prnFrames.StartDict()};

    PrintPicture(prnFrame, *frame, *picture);
  }

  /*
  s << "Picture structure for frame " << frame->sequenceNumber << std::endl;
  const size_t numCtu{picture->cs->pcv->sizeInCtus};
  s << "Number of Ctu's: " << numCtu << std::endl;

  for (size_t i{0u}; i < numCtu; ++i)
  {
    const auto ctu{picture->cs->getCtuData(i)};

    s << "Ctu #" << ctu.ctuIdx << "x: " << ctu.colIdx << " y: " << ctu.lineIdx << std::endl;
  }*/

  return VVDEC_OK;
}

}   // namespace vvdec
