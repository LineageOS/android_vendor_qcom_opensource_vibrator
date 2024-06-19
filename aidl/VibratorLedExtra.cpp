/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2024, The LineageOS Project
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
 *
 * Changes from Qualcomm Innovation Center are provided under the following license:
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#define LOG_TAG "vendor.qti.vibrator.led.extra"
#include <log/log.h>

#include <stdint.h>
#include <stdio.h>

#include "include/Vibrator.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

int32_t LedVibratorDevice::effectToMs(Effect effect) {
    // Imported from hardware/interfaces/vibrator/1.3/example/Vibrator.cpp
    switch (effect) {
        case Effect::CLICK:
            return 10;
        case Effect::DOUBLE_CLICK:
            return 15;
        case Effect::TICK:
        case Effect::TEXTURE_TICK:
            return 5;
        case Effect::THUD:
            return 5;
        case Effect::POP:
            return 5;
        case Effect::HEAVY_CLICK:
            return 10;
        default:
            return 0;
    }
}

ndk::ScopedAStatus LedVibratorDevice::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {Effect::CLICK, Effect::DOUBLE_CLICK, Effect::TICK,        Effect::THUD,
                     Effect::POP,   Effect::HEAVY_CLICK,  Effect::TEXTURE_TICK};
    return ndk::ScopedAStatus::ok();
}

float LedVibratorDevice::strengthToAmplitude(EffectStrength strength) {
    switch (strength) {
        case EffectStrength::LIGHT:
            return 0.3f;
        case EffectStrength::MEDIUM:
            return 0.6f;
        case EffectStrength::STRONG:
            return 0.9f;
        default:
            return 0.5f;
    }
}

ndk::ScopedAStatus LedVibratorDevice::getCapabilities(int32_t* _aidl_return) {
    *_aidl_return = IVibrator::CAP_ON_CALLBACK | IVibrator::CAP_PERFORM_CALLBACK;

    if (mAmplitudeControlEnabled) *_aidl_return |= IVibrator::CAP_AMPLITUDE_CONTROL;

    ALOGD("QTI Vibrator reporting capabilities: %d", *_aidl_return);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LedVibratorDevice::perform(Effect effect, EffectStrength es,
                                              const std::shared_ptr<IVibratorCallback>& callback,
                                              int32_t* _aidl_return) {
    int32_t playLengthMs;

    ALOGD("Vibrator perform effect %d", effect);

    playLengthMs = effectToMs(effect);
    if (!playLengthMs) {
        ALOGE("No corresponding play duration");
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }

    if (mAmplitudeControlEnabled) {
        if (es != EffectStrength::LIGHT && es != EffectStrength::MEDIUM &&
            es != EffectStrength::STRONG) {
            ALOGE("Invalid EffectStrength");
            return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
        }

        if (!setAmplitude(strengthToAmplitude(es)).isOk()) ALOGE("Failed to set amplitude");
    }

    if (on(playLengthMs) != 0) {
        ALOGE("Failed to turn on vibrator");
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    }

    if (callback != nullptr) {
        std::thread([=] {
            ALOGD("Starting perform on another thread");
            usleep(playLengthMs * 1000);
            ALOGD("Notifying perform complete");
            callback->onComplete();
        }).detach();
    }

    *_aidl_return = playLengthMs;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LedVibratorDevice::setAmplitude(float amplitude) {
    if (!mAmplitudeControlEnabled)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

    ALOGD("Vibrator set amplitude: %f", amplitude);

    if (amplitude <= 0.0f || amplitude > 1.0f) {
        ALOGE("Invalid amplitude");
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }

    uint32_t mv = mMvMin + ((mMvMax - mMvMin) * amplitude);
    ALOGD("Vibrator set mv: %u", mv);

    char value[6];
    snprintf(value, sizeof(value), "%u", mv);
    if (write_value("/sys/class/leds/vibrator/vmax_mv", value) < 0) {
        ALOGE("Failed to write %s to vmax_mv", value);
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
