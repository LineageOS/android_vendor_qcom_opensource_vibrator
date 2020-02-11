ifeq ($(call is-board-platform-in-list, msmnile kona lahaina), true)

ENABLE_VIBRATOR := true

ifeq ($(TARGET_USES_QMAA),true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_VIBRATOR),true)
ENABLE_VIBRATOR := false
endif
endif


ifeq ($(ENABLE_VIBRATOR),true)
PRODUCT_PACKAGES += \
      vendor.qti.hardware.vibrator@1.2-service
else
PRODUCT_PACKAGES += \
      vendor.qti.hardware.vibrator.qmaa@1.2-service
endif
endif
