LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL_mixer

LOCAL_CFLAGS := -I$(LOCAL_PATH) -I$(LOCAL_PATH)/.. -I$(LOCAL_PATH)/../SDL/include -I$(LOCAL_PATH)/../mad  -I$(LOCAL_PATH)/../mikmod \
					-DWAV_MUSIC -DOGG_USE_TREMOR -DOGG_MUSIC -DMP3_MAD_MUSIC -DMOD_MUSIC -DMOD_DINAMIC

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))
#LOCAL_SRC_FILES += $(notdir/mikmod $(wildcard $(LOCAL_PATH)/mikmod/*.c))

LOCAL_SHARED_LIBRARIES := SDL
LOCAL_STATIC_LIBRARIES := tremor mad mikmod

include $(BUILD_SHARED_LIBRARY)

