LOCAL_PATH := $(call my-dir)
ifeq ($(TARGET_USES_QMAA),true)
    PRODUCT_SOONG_NAMESPACES += ${LOCAL_PATH}/qmaa
endif

