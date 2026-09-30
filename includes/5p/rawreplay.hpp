#pragma once

#include "5p/common.hpp"
#include "5p/pcapreader.hpp"

namespace rawreplay {

bool IsReplayableFrame(const pcpp::RawPacket& packet, pcpp::LinkLayerType deviceLinkType);
bool Replay(pcapreader::Reader& reader, const common::config& config);

}    // namespace rawreplay