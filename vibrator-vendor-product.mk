ifeq ($(call is-board-platform-in-list, msmnile kona lahaina), true)

QTI_VIBRATOR_HAL_SERVICE := \
      vendor.qti.hardware.vibrator@1.2-service

ifeq ($(call is-board-platform-in-list, lahaina), true)
QTI_VIBRATOR_HAL_SERVICE := \
      vendor.qti.hardware.vibrator@1.3-service
endif

ifeq ($(TARGET_USES_QMAA),true)
ifeq ($(TARGET_USES_QMAA_OVERRIDE_VIBRATOR),true)
QTI_VIBRATOR_HAL_SERVICE := \
      vendor.qti.hardware.vibrator.qmaa@1.2-service
endif
endif

PRODUCT_PACKAGES += $(QTI_VIBRATOR_HAL_SERVICE)

PRODUCT_COPY_FILES += \
      vendor/qcom/opensource/vibrator/excluded-input-devices.xml:vendor/etc/excluded-input-devices.xml
endif
