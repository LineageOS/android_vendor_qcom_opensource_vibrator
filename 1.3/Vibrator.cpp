/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "vendor.qti.vibrator@1.3-impl"

#include <hardware/hardware.h>
#include <hardware/vibrator.h>
#include <inttypes.h>
#include <log/log.h>

#include "Vibrator_1_3.h"

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

using android::hardware::vibrator::V1_0::Status;
using android::hardware::vibrator::V1_0::EffectStrength;
using android::hardware::vibrator::V1_3::Effect;

/**
 * Any targets using V1.3 vibrator interface will support
 * external control by default
 */
Return<bool> Vibrator::supportsExternalControl() {
    return true;
}

/**
 * The existing hardware module would cease current vibration
 * and switch to the external control vibration automatically,
 * so nothing need to be done in sw, hence return OK directly.
 */
Return<Status> Vibrator::setExternalControl(__attribute__((unused))bool enable) {
    return Status::OK;
}

using Effect_1_0 = ::android::hardware::vibrator::V1_0::Effect;
Return<void> Vibrator::perform(Effect_1_0 effect, EffectStrength es, perform_cb _hidl_cb) {
    Status status;
    int effectId = static_cast<int16_t>(effect);
    long playLengthMs;

    if (effectId < (static_cast<int16_t>(Effect_1_0::CLICK)) ||
            effectId > (static_cast<int16_t>(Effect_1_0::DOUBLE_CLICK))) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = ff.playEffect(effectId, es, &playLengthMs);
    if (status == Status::OK)
        _hidl_cb(Status::OK, playLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}

using Effect_1_1 = ::android::hardware::vibrator::V1_1::Effect_1_1;
Return<void> Vibrator::perform_1_1(Effect_1_1 effect, EffectStrength es, perform_cb _hidl_cb) {
    Status status;
    int effectId = static_cast<int16_t>(effect);
    long playLengthMs;

    if (effectId < (static_cast<int16_t>(Effect_1_1::CLICK)) ||
            effectId > (static_cast<int16_t>(Effect_1_1::TICK))) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = ff.playEffect(effectId, es, &playLengthMs);
    if (status == Status::OK)
        _hidl_cb(Status::OK, playLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}

using Effect_1_2 = ::android::hardware::vibrator::V1_2::Effect;
Return<void> Vibrator::perform_1_2(Effect_1_2 effect, EffectStrength es, perform_cb _hidl_cb) {
    Status status;
    int effectId = static_cast<int16_t>(effect);
    long playLengthMs;

    if (effectId < (static_cast<int16_t>(Effect_1_2::CLICK)) ||
            effectId > (static_cast<int16_t>(Effect_1_2::RINGTONE_15))) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = ff.playEffect(effectId, es, &playLengthMs);
    if (status == Status::OK)
        _hidl_cb(Status::OK, playLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}

using Effect_1_3 = ::android::hardware::vibrator::V1_3::Effect;
Return<void> Vibrator::perform_1_3(Effect_1_3 effect, EffectStrength es, perform_1_3_cb _hidl_cb) {
    Status status;
    int effectId = static_cast<int16_t>(effect);
    long playLengthMs;

    if (effectId < (static_cast<int16_t>(Effect_1_3::CLICK)) ||
           effectId > (static_cast<int16_t>(Effect_1_3::TEXTURE_TICK))) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = ff.playEffect(effectId, es, &playLengthMs);
    if (status == Status::OK)
        _hidl_cb(Status::OK, playLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}
}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
