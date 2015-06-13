APP_BUILD_SCRIPT := Android.mk
$(info $(APP_BUILD_SCRIPT))

NDK_TOOLCHAIN_VERSION := 4.8
APP_ABI := armeabi armeabi-v7a
APP_PLATFORM := android-21
APP_STL := gnustl_static
APP_CPPFLAGS += -std=c++11 -fexceptions -frtti
LOCAL_C_INCLUDES += ${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/include
TARGET_CPU_ABI := armeabi-v7a 
TARGET_CPU_ABI2 := armeabi 