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

#define LOG_TAG "vendor.qti.vibrator@1.2-impl"

#include <hardware/hardware.h>
#include <hardware/vibrator.h>
#include <dirent.h>
#include <inttypes.h>
#include <linux/input.h>
#include <log/log.h>
#include <string.h>
#include <sys/ioctl.h>

#include "Vibrator_1_2.h"

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_2 {
namespace implementation {

using Status = ::android::hardware::vibrator::V1_0::Status;
using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;

#define STRONG_MAGNITUDE        0x7fff
#define MEDIUM_MAGNITUDE        0x5fff
#define LIGHT_MAGNITUDE         0x3fff
#define INVALID_VALUE           -1

#define test_bit(bit, array)    ((array)[(bit)/8] & (1<<((bit)%8)))

InputFFDevice::InputFFDevice()
{
    DIR *dp;
    struct dirent *dir;
    uint8_t ffBitmask[FF_CNT / 8];
    char devicename[PATH_MAX];
    const char *INPUT_DIR = "/dev/input/";
    int fd, ret;

    mVibraFd = INVALID_VALUE;
    mSupportGain = false;
    mSupportEffects = false;
    mCurrAppId = INVALID_VALUE;
    mCurrMagnitude = 0x7fff;

    dp = opendir(INPUT_DIR);
    if (!dp) {
        ALOGE("open %s failed, errno = %d", INPUT_DIR, errno);
        return;
    }

    memset(ffBitmask, 0, sizeof(ffBitmask));
    while ((dir = readdir(dp)) != NULL){
        if (dir->d_name[0] == '.' &&
            (dir->d_name[1] == '\0' ||
             (dir->d_name[1] == '.' && dir->d_name[2] == '\0')))
            continue;

        snprintf(devicename, PATH_MAX, "%s%s", INPUT_DIR, dir->d_name);
        fd = TEMP_FAILURE_RETRY(open(devicename, O_RDWR));
        if (fd < 0) {
            ALOGE("open %s failed, errno = %d", devicename, errno);
            continue;
        }

        ret = TEMP_FAILURE_RETRY(ioctl(fd, EVIOCGBIT(EV_FF, sizeof(ffBitmask)), ffBitmask));
        if (ret == -1) {
            ALOGE("ioctl failed, errno = %d", errno);
            close(fd);
            continue;
        }

        if (test_bit(FF_CONSTANT, ffBitmask) ||
                test_bit(FF_PERIODIC, ffBitmask)) {
            mVibraFd = fd;
            if (test_bit(FF_CUSTOM, ffBitmask))
                mSupportEffects = true;
            if (test_bit(FF_GAIN, ffBitmask))
                mSupportGain = true;
            break;
        }

        close(fd);
    }

    closedir(dp);
}

/** Play vibration
 *
 *  @param effectId:  ID of the predefined effect will be played. If effectId is valid
 *                    (non-negative value), the timeoutMs value will be ignored, and the
 *                    real playing length will be set in param@playLengtMs and returned
 *                    to VibratorService. If effectId is invalid, value in param@timeoutMs
 *                    will be used as the play length for playing a constant effect.
 *  @param timeoutMs: playing length, non-zero means playing, zero means stop playing.
 *  @param playLengthMs: the playing length in ms unit which will be returned to
 *                    VibratorService if the request is playing a predefined effect.
 *                    The custom_data in periodic is reused for returning the playLengthMs
 *                    from kernel space to userspace if the pattern is defined in kernel
 *                    driver. It's been defined with following format:
 *                       <effect-ID, play-time-in-seconds, play-time-in-milliseconds>.
 *                    The effect-ID is used for passing down the predefined effect to
 *                    kernel driver, and the rest two parameters are used for returning
 *                    back the real playing length from kernel driver.
 */
Return<Status> InputFFDevice::play(int effectId, uint32_t timeoutMs, long *playLengthMs) {
    struct ff_effect effect;
    struct input_event play;
    #define CUSTOM_DATA_LEN    3
    int16_t data[CUSTOM_DATA_LEN] = {0, 0, 0};
    int ret;

    /* For QMAA compliance, return OK even if vibrator device doesn't exist */
    if (mVibraFd == INVALID_VALUE) {
        if (playLengthMs != NULL)
            *playLengthMs = 0;
        return Status::OK;
    }

    if (timeoutMs != 0) {
        if (mCurrAppId != INVALID_VALUE) {
            ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCRMFF, mCurrAppId));
            if (ret == -1) {
                ALOGE("ioctl EVIOCRMFF failed, errno = %d", -errno);
                goto errout;
            }
            mCurrAppId = INVALID_VALUE;
        }

        memset(&effect, 0, sizeof(effect));
        if (effectId != INVALID_VALUE) {
            data[0] = effectId;
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_CUSTOM;
            effect.u.periodic.magnitude = mCurrMagnitude;
            effect.u.periodic.custom_data = data;
            effect.u.periodic.custom_len = sizeof(int16_t) * CUSTOM_DATA_LEN;
        } else {
            effect.type = FF_CONSTANT;
            effect.u.constant.level = mCurrMagnitude;
            effect.replay.length = timeoutMs;
        }

        effect.id = mCurrAppId;
        effect.replay.delay = 0;

        ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCSFF, &effect));
        if (ret == -1) {
            ALOGE("ioctl EVIOCSFF failed, errno = %d", -errno);
            goto errout;
        }

        mCurrAppId = effect.id;
        if (effectId != INVALID_VALUE && playLengthMs != NULL)
            *playLengthMs = data[1] * 1000 + data[2];

        play.value = 1;
        play.type = EV_FF;
        play.code = mCurrAppId;
        play.time.tv_sec = 0;
        play.time.tv_usec = 0;
        ret = TEMP_FAILURE_RETRY(write(mVibraFd, (const void*)&play, sizeof(play)));
        if (ret == -1) {
            ALOGE("write failed, errno = %d", -errno);
            ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCRMFF, mCurrAppId));
            if (ret == -1)
                ALOGE("ioctl EVIOCRMFF failed, errno = %d", -errno);
            goto errout;
        }
    } else if (mCurrAppId != INVALID_VALUE) {
        ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCRMFF, mCurrAppId));
        if (ret == -1) {
            ALOGE("ioctl EVIOCRMFF failed, errno = %d", -errno);
            goto errout;
        }
        mCurrAppId = INVALID_VALUE;
    }
    return Status::OK;

errout:
    mCurrAppId = INVALID_VALUE;
    return Status::UNSUPPORTED_OPERATION;
}

Return<Status> InputFFDevice::on(uint32_t timeoutMs) {
    return play(INVALID_VALUE, timeoutMs, NULL);
}

Return<Status> InputFFDevice::off() {
    return play(INVALID_VALUE, 0, NULL);
}

Return<bool> InputFFDevice::supportsAmplitudeControl() {
    return mSupportGain ? true : false;
}

Return<Status> InputFFDevice::setAmplitude(uint8_t amplitude) {
    int tmp, ret;
    struct input_event ie;

    if (!mSupportGain)
        return Status::UNSUPPORTED_OPERATION;

    if (amplitude == 0)
        return Status::BAD_VALUE;

    /* For QMAA compliance, return OK even if vibrator device doesn't exist */
    if (mVibraFd == INVALID_VALUE)
        return Status::OK;

    tmp = amplitude * (STRONG_MAGNITUDE - LIGHT_MAGNITUDE) / 255;
    tmp += LIGHT_MAGNITUDE;
    ie.type = EV_FF;
    ie.code = FF_GAIN;
    ie.value = tmp;

    ret = TEMP_FAILURE_RETRY(write(mVibraFd, &ie, sizeof(ie)));
    if (ret == -1) {
        ALOGE("write FF_GAIN failed, errno = %d", -errno);
        return Status::UNSUPPORTED_OPERATION;
    }

    mCurrMagnitude = tmp;
    return Status::OK;
}

Return<Status> InputFFDevice::playEffect(int effectId, EffectStrength es, long *playLengthMs) {
    if (!mSupportEffects)
        return Status::UNSUPPORTED_OPERATION;

    switch (es) {
    case EffectStrength::LIGHT:
        mCurrMagnitude = LIGHT_MAGNITUDE;
        break;
    case EffectStrength::MEDIUM:
        mCurrMagnitude = MEDIUM_MAGNITUDE;
        break;
    case EffectStrength::STRONG:
        mCurrMagnitude = STRONG_MAGNITUDE;
        break;
    default:
        break;
    }

    return play(effectId, INVALID_VALUE, playLengthMs);
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
}  // namespace implementation
}  // namespace V1_2
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
