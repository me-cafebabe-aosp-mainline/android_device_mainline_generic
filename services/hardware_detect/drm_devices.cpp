/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include "drm_devices.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

namespace fs = std::filesystem;
using namespace android::base;

namespace hardware_detect {

namespace {

constexpr char kBootEnumDrmInReverseProp[] = "ro.boot.enum_drm_in_reverse";
constexpr char kBootPreferDrmCardNameProp[] = "ro.boot.prefer_drm_card_name";
constexpr char kBootPreferDrmRenderNameProp[] = "ro.boot.prefer_drm_render_name";

const std::set<std::string> kDrmSysfbNames = {
        "efidrm",
        "ofdrm",
        "simpledrm",
        "vesadrm",
};

bool IsDrmCardWithConnectorConnected(const DrmDevice& dev) {
    drmModeRes* resources = dev.drm_mode_res;
    int fd = dev.fd.get();
    bool result = false;
    const bool was_master = drmIsMaster(fd) == 1;
    const bool can_force_probe = was_master || drmSetMaster(fd) == 0;

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector* conn = can_force_probe
                                         ? drmModeGetConnector(fd, resources->connectors[i])
                                         : drmModeGetConnectorCurrent(fd, resources->connectors[i]);

        if (!conn) continue;

        const bool connected =
                conn->connection == DRM_MODE_CONNECTED ||
                (dev.name == "hyperv_drm" && conn->connection == DRM_MODE_UNKNOWNCONNECTION);
        if (connected && conn->count_modes > 0) {
            result = true;
        }

        drmModeFreeConnector(conn);

        if (result) break;
    }

    if (can_force_probe && !was_master) drmDropMaster(fd);

    return result;
}

std::tuple<int, uint32_t, std::string> DrmNodeSortKey(const fs::directory_entry& entry) {
    const std::string name = entry.path().filename();
    const bool is_render = StartsWith(name, "renderD");
    const std::string_view suffix =
            is_render ? std::string_view(name).substr(7) : std::string_view(name).substr(4);
    uint32_t index = UINT32_MAX;
    const auto [end, error] = std::from_chars(suffix.data(), suffix.data() + suffix.size(), index);
    if (error != std::errc() || end != suffix.data() + suffix.size()) index = UINT32_MAX;
    return {is_render ? 1 : 0, index, name};
}

}  // namespace

void CleanupDrmDevice(DrmDevice& dev) {
    if (dev.drm_device) drmFreeDevice(&dev.drm_device);
    if (dev.drm_mode_res) {
        drmModeFreeResources(dev.drm_mode_res);
        dev.drm_mode_res = nullptr;
    }
    dev.fd.reset();
}

std::pair<std::optional<DrmDevice>, std::optional<DrmDevice>> GetDrmCardRenderDevicePair(void) {
    std::vector<fs::directory_entry> all_entries;
    constexpr char kDriDir[] = "/dev/dri";
    if (!std::filesystem::is_directory(kDriDir)) {
        LOG(ERROR) << kDriDir << " does not exist";
        return {};
    }
    for (const auto& entry : fs::directory_iterator(kDriDir)) {
        if (!entry.is_other()) continue;
        std::string name = entry.path().filename();
        if (StartsWith(name, "card") || StartsWith(name, "renderD")) {
            all_entries.push_back(entry);
        }
    }
    const bool reverse = GetBoolProperty(kBootEnumDrmInReverseProp, false);
    std::sort(all_entries.begin(), all_entries.end(),
              [reverse](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return reverse ? DrmNodeSortKey(a) > DrmNodeSortKey(b)
                                 : DrmNodeSortKey(a) < DrmNodeSortKey(b);
              });

    std::vector<DrmDevice> all_cards, all_renders;
    for (const auto& entry : all_entries) {
        DrmDevice dev;
        dev.is_render = StartsWith(std::string(entry.path().filename()), "renderD");
        dev.path = entry.path();

        dev.fd.reset(open(dev.path.c_str(), dev.is_render ? O_RDONLY : O_RDWR));
        if (!dev.fd.ok()) {
            LOG(ERROR) << "Unable to open DRM device " << dev.path;
            continue;
        }

        drmVersionPtr version = drmGetVersion(dev.fd.get());
        if (!version) {
            LOG(ERROR) << "Failed to get DRM version for " << dev.path;
            CleanupDrmDevice(dev);
            continue;
        }
        dev.name = std::string(version->name, version->name_len);
        LOG(INFO) << "DRM device " << dev.path << " is " << dev.name;
        drmFreeVersion(version);

        if (drmGetDevice(dev.fd.get(), &dev.drm_device)) {
            LOG(ERROR) << "Failed to get DRM device information for " << dev.path;
            CleanupDrmDevice(dev);
            continue;
        }

        if (!dev.is_render) {
            dev.drm_mode_res = drmModeGetResources(dev.fd.get());
            if (!dev.drm_mode_res) {
                LOG(ERROR) << "Failed to get DRM mode resources for " << dev.path;
                CleanupDrmDevice(dev);
                continue;
            }

            dev.have_connector_connected = IsDrmCardWithConnectorConnected(dev);
            if (!dev.have_connector_connected) {
                LOG(INFO) << "DRM card " << dev.path << " have no connector connected";
            }

            if (kDrmSysfbNames.find(dev.name) != kDrmSysfbNames.end()) {
                dev.is_sysfb = true;
            }

            all_cards.push_back(std::move(dev));
        } else {
            all_renders.push_back(std::move(dev));
        }
    }

    if (all_cards.empty()) {
        LOG(ERROR) << "No DRM card found";
        return {};
    }

    if (all_renders.empty()) {
        LOG(INFO) << "No DRM render found";
    } else {
        for (const auto& render : all_renders) {
            for (auto& card : all_cards) {
                if (drmDevicesEqual(render.drm_device, card.drm_device)) {
                    LOG(INFO) << "DRM card " << card.path << " have render " << render.path;
                    card.card_have_render_path = render.path;
                }
            }
        }
    }

    std::string result_card_path, result_render_path;

    for (const auto& card : all_cards) {
        if (card.is_sysfb) continue;
        if (!card.have_connector_connected) continue;
        if (card.card_have_render_path.empty()) continue;
        LOG(INFO) << "Select DRM devices using strategy 1";
        result_card_path = card.path;
        result_render_path = card.card_have_render_path;
        goto out;
    }
    for (const auto& card : all_cards) {
        if (card.is_sysfb) continue;
        if (!card.have_connector_connected) continue;
        LOG(INFO) << "Select DRM devices using strategy 2";
        result_card_path = card.path;
        goto out;
    }
    for (const auto& card : all_cards) {
        if (card.is_sysfb) continue;
        LOG(INFO) << "Select DRM devices using strategy 3";
        result_card_path = card.path;
        goto out;
    }
    LOG(INFO) << "Select DRM devices using fallback strategy";
    result_card_path = all_cards.begin()->path;

out:
    auto user_preferred_card_name = GetProperty(kBootPreferDrmCardNameProp, "");
    if (!user_preferred_card_name.empty()) {
        for (const auto& card : all_cards) {
            if (card.name != user_preferred_card_name) continue;
            LOG(INFO) << "Override DRM card selection from " << result_card_path;
            result_card_path = card.path;
            if (!card.card_have_render_path.empty()) {
                LOG(INFO) << "Override DRM render selection from " << result_render_path;
                result_render_path = card.card_have_render_path;
            }
            break;
        }
    }
    auto user_preferred_render_name = GetProperty(kBootPreferDrmRenderNameProp, "");
    if (!user_preferred_render_name.empty()) {
        for (const auto& render : all_renders) {
            if (render.name != user_preferred_render_name) continue;
            LOG(INFO) << "Override DRM render selection from " << result_render_path;
            result_render_path = render.path;
            break;
        }
    }

    LOG(INFO) << "Selected DRM card " << result_card_path << " and DRM render "
              << (result_render_path.empty() ? "(none)" : result_render_path);
    std::optional<DrmDevice> result_card, result_render;
    for (auto& card : all_cards) {
        if (card.path == result_card_path) {
            result_card.emplace(std::move(card));
            continue;
        }
        CleanupDrmDevice(card);
        fs::rename(card.path, card.path + "_orig");
        fs::create_symlink("/dev/null", card.path);
    }
    for (auto& render : all_renders) {
        if (render.path == result_render_path) {
            result_render.emplace(std::move(render));
            continue;
        }
        CleanupDrmDevice(render);
        fs::rename(render.path, render.path + "_orig");
        fs::create_symlink("/dev/null", render.path);
    }
    return {std::move(result_card), std::move(result_render)};
}

}  // namespace hardware_detect
