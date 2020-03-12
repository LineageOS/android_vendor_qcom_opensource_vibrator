/*
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
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

#ifndef ANDROID_HARDWARE_VIBRATOR_V1_2_VIBRATOR_H
#define ANDROID_HARDWARE_VIBRATOR_V1_2_VIBRATOR_H

#include <android/hardware/vibrator/1.2/IVibrator.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_2 {
namespace implementation {

using Status = ::android::hardware::vibrator::V1_0::Status;
using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;

class InputFFDevice {
public:
    InputFFDevice();
    Return<Status> playEffect(int effectId, EffectStrength es, long *playLengthMs);

    Return<Status> on(uint32_t timeoutMs);
    Return<Status> off();
    Return<bool> supportsAmplitudeControl();
    Return<Status> setAmplitude(uint8_t amplitude);
private:
    Return<Status> play(int effectId, uint32_t timeoutMs, long *playLengthMs);
    int mVibraFd;
    int16_t mCurrAppId;
    int16_t mCurrMagnitude;
    bool mSupportGain;
    bool mSupportEffects;
};

class Vibrator : public IVibrator {
public:
    class InputFFDevice ff;
    Return<Status> on(uint32_t timeoutMs) {return ff.on(timeoutMs);};
    Return<Status> off() {return ff.off();};
    Return<bool> supportsAmplitudeControl() {return ff.supportsAmplitudeControl();};
    Return<Status> setAmplitude(uint8_t amplitude) {return ff.setAmplitude(amplitude);};

    Return<void> perform(::android::hardware::vibrator::V1_0::Effect effect, EffectStrength strength, perform_cb _hidl_cb) override;
    Return<void> perform_1_1(::android::hardware::vibrator::V1_1::Effect_1_1 effect, EffectStrength strength, perform_cb _hidl_cb) override;
    Return<void> perform_1_2(::android::hardware::vibrator::V1_2::Effect effect, EffectStrength strength, perform_cb _hidl_cb) override;
};

}  // namespace implementation
}  // namespace V1_2
}  // namespace vibrator
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_VIBRATOR_V1_2_VIBRATOR_H
