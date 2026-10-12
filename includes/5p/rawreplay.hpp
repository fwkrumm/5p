#pragma once

#include "5p/common.hpp"
#include "5p/pcapreader.hpp"

namespace rawreplay {

// Return whether the link type supports raw Ethernet replay.
bool IsReplayableInterface(pcpp::LinkLayerType linkType);
// Log openable Ethernet interfaces without sending packets.
void ListRawInterfaces();
bool IsReplayableFrame(const pcpp::RawPacket& packet, pcpp::LinkLayerType deviceLinkType);
bool Replay(pcapreader::Reader& reader, const common::config& config);

}    // namespace rawreplay