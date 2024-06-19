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

    if (mAmplitudeControlEnabled)
        *_aidl_return |= IVibrator::CAP_AMPLITUDE_CONTROL;

    ALOGD("QTI Vibrator reporting capabilities: %d", *_aidl_return);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LedVibratorDevice::perform(Effect effect, EffectStrength es, const std::shared_ptr<IVibratorCallback>& callback, int32_t* _aidl_return) {
    int32_t playLengthMs;

    ALOGD("Vibrator perform effect %d", effect);

    playLengthMs = effectToMs(effect);
    if (!playLengthMs)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

    if (mAmplitudeControlEnabled) {
        if (es != EffectStrength::LIGHT && es != EffectStrength::MEDIUM && es != EffectStrength::STRONG)
            return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

        setAmplitude(strengthToAmplitude(es));
    }

    if (on(playLengthMs) != 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));

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

ndk::ScopedAStatus LedVibratorDevice::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {Effect::CLICK, Effect::DOUBLE_CLICK, Effect::TICK, Effect::THUD,
                     Effect::POP, Effect::HEAVY_CLICK, Effect::TEXTURE_TICK};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LedVibratorDevice::setAmplitude(float amplitude) {
    if (!mAmplitudeControlEnabled)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

    ALOGD("Vibrator set amplitude: %f", amplitude);

    if (amplitude <= 0.0f || amplitude > 1.0f)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));

    uint32_t mv = mMvMin + ((mMvMax - mMvMin) * amplitude);
    ALOGD("Vibrator set mv: %u", mv);

    char value[6];
    snprintf(value, sizeof(value), "%u", mv);
    if (write_value("/sys/class/leds/vibrator/vmax_mv", value) < 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));

    return ndk::ScopedAStatus::ok();
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
