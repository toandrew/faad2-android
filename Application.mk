APP_STL := gnustl_shared

APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -std=c++11

APP_CPPFLAGS += -DANDROID
APP_PLATFORM := android-9

APP_ABI := armeabi armeabi-v7a x86 x86_64 arm64-v8a
