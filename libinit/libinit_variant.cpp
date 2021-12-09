/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/properties.h>
#include <filesystem>
#include <libinit_utils.h>

#include <libinit_variant.h>

using android::base::GetProperty;
using std::filesystem::directory_iterator;
using std::filesystem::filesystem_error;

#define HWC_PROP "ro.boot.hwc"
#define SKU_PROP "ro.boot.product.hardware.sku"

void search_variant(const std::vector<variant_info_t> variants) {
    std::string hwc_value = GetProperty(HWC_PROP, "");
    std::string sku_value = GetProperty(SKU_PROP, "");

    for (const auto& variant : variants) {
        if ((variant.hwc_value == "" || variant.hwc_value == hwc_value) &&
            (variant.sku_value == "" || variant.sku_value == sku_value)) {
            set_variant_props(variant);
            break;
        }
    }
}

void set_variant_props(const variant_info_t variant) {
    set_ro_build_prop("brand", variant.brand, true);
    set_ro_build_prop("device", variant.device, true);
    set_ro_build_prop("marketname", variant.marketname, true);
    set_ro_build_prop("model", variant.model, true);

    set_ro_build_prop("fingerprint", variant.build_fingerprint);
    property_override("ro.bootimage.build.fingerprint", variant.build_fingerprint);

    property_override("ro.build.description", fingerprint_to_description(variant.build_fingerprint));

    if (variant.nfc)
        property_override(SKU_PROP, "nfc");

    property_override("ro.arch", variant.device);
    property_override("ro.boot.product.vendor.sku", variant.device);

    int i = 0;
    try {
        for (const auto& acdb : directory_iterator("/vendor/etc/acdbdata/" + variant.device))
            property_override("persist.vendor.audio.calfile" + std::to_string(i++), acdb.path());
    } catch (const filesystem_error&) {
        // Ignore
    }
}
