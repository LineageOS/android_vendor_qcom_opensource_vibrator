/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#define LOG_TAG "android.hardware.vibrator@1.2-service.qti"

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>

#include "Vibrator.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::vibrator::V1_2::IVibrator;
using android::hardware::vibrator::V1_2::implementation::Vibrator;
using namespace android;

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_2 {
namespace implementation {


using Status = ::android::hardware::vibrator::V1_0::Status;

Vibrator::Vibrator(int vibraFd, bool supportGain, bool supportEffects) :
    mVibraFd(vibraFd), mSupportGain(supportGain),
    mSupportEffects(supportEffects) {
    mCurrAppId = -1;
    mCurrEffectId = -1;
    mCurrMagnitude = 0x7fff;
    mPlayLengthMs = 0;
}

/** Play vibration
 *
 *  @param timeoutMs: playing length, non-zero means playing; zero means stop playing.
 *
 *  If the request is playing with a predefined effect, the timeoutMs value is
 *  ignored, and the real playing length is required to be returned from the kernel
 *  driver for userspace service to wait until the vibration done.
 *
 *  The custom_data in periodic is reused for this usage. It's been defined with
 *  following format: <effect-ID, play-time-in-seconds, play-time-in-milliseconds>.
 *  The effect-ID is used for passing down the predefined effect to kernel driver,
 *  and play-time-xxx is used for return back the real playing length from kernel
 *  driver.
 */
Return<Status> Vibrator::play(__attribute__((unused)) uint32_t timeoutMs) {
    mVibraFd = 0;
    return Status::OK;

}

Return<Status> Vibrator::on(__attribute__((unused)) uint32_t timeoutMs) {
    return play(timeoutMs);
}

Return<Status> Vibrator::off() {
    return play(0);
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    return mSupportGain ? true : false;
}

Return<Status> Vibrator::setAmplitude(__attribute__((unused)) uint8_t amplitude) {
    return Status::OK;
}


using Effect_1_0 = ::android::hardware::vibrator::V1_0::Effect;
Return<void> Vibrator::perform(__attribute__((unused))Effect_1_0 effect,__attribute__((unused)) EffectStrength es, perform_cb _hidl_cb) {
    if (!mSupportEffects) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }

    _hidl_cb(Status::OK, mPlayLengthMs);
    return Void();
}

using Effect_1_1 = ::android::hardware::vibrator::V1_1::Effect_1_1;
Return<void> Vibrator::perform_1_1(__attribute__((unused)) Effect_1_1 effect,__attribute__((unused)) EffectStrength es, perform_1_1_cb _hidl_cb) {

    _hidl_cb(Status::OK, mPlayLengthMs);
    return Void();
}

using Effect_1_2 = ::android::hardware::vibrator::V1_2::Effect;
Return<void> Vibrator::perform_1_2(__attribute__((unused)) Effect_1_2 effect,__attribute__((unused))  EffectStrength es, perform_1_2_cb _hidl_cb) {
    _hidl_cb(Status::OK, mPlayLengthMs);
    return Void();
}
}  // namespace implementation
}  // namespace V1_2
}  // namespace vibrator
}  // namespace hardware
}  // namespace android



int main() {
    bool supportGain = false, supportEffects = false;
    int ret = -1;
    int vibraFd = -1;

    configureRpcThreadpool(1, true);

    sp<IVibrator> vibrator = new Vibrator(vibraFd, supportGain, supportEffects);
    ret =  vibrator->registerAsService();


    joinRpcThreadpool();
    return ret;
}
