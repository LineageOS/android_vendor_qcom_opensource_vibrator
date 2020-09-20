LOCAL_PATH := $(call my-dir)
ifeq ($(TARGET_USES_QMAA),true)
    PRODUCT_SOONG_NAMESPACESa += ${LOCAL_PATH}/qmaa
endif

