#pragma once

#include "nexus/Nexus.h"

#include <imgui.h>
#include <string>

namespace wap {

class DatAssetIconService {
public:
    static void Initialize(AddonAPI_t* api, const std::string& addonDir);
    static void Shutdown();
    static ImTextureID Request(int assetId);
    static void ProcessDownloads();

private:
    static ImTextureID LoadFromDisk(int assetId);
    static void QueueDownload(int assetId);
};

}  // namespace wap
