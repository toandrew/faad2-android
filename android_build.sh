#!/bin/sh

#git clone https://github.com/knik0/faad2

cp ./config.h ./faad2

ndk-build NDK_PROJECT_PATH=./build \
			NDK_LIBS_OUT=./jniLibs \
			NDK_APPLICATION_MK=./Application.mk \
			APP_BUILD_SCRIPT=./Android.mk \
			V=1
			
#cp -rf faad2/include ./include

cp -rf faad2/include lib/audioconverter/src/main/cpp/faad2
cp -rf build/obj/local/armeabi-v7a/libfaad2.a lib/audioconverter/src/main/cpp/faad2/armeabi-v7a/libfaad2.a
cp -rf build/obj/local/arm64-v8a/libfaad2.a lib/audioconverter/src/main/cpp/faad2/arm64-v8a/libfaad2.a
