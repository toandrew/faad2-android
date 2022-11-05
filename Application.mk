#APP_STL := gnustl_shared
APP_STL := c++_shared

APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -std=c++11

APP_CPPFLAGS += -DANDROID
APP_PLATFORM := android-16

APP_ABI := armeabi-v7a arm64-v8a
