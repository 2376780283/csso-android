#!/bin/bash

# --- 路径配置 ---
export NDK=$ANDROID_NDK_HOME
export HOST_OS="linux-arm64"
export TOOLCHAIN="${NDK}/toolchains/llvm/prebuilt/${HOST_OS}"
export PATH="${TOOLCHAIN}/bin:${PATH}"

# 你的 libvpx 安装路径 (请根据实际情况修改)
export VPX_PATH="/data/data/com.termux/files/home/csso-android/thirdparty/libvpx/android/arm64-v8a"
# 最终 FFmpeg 安装路径
export BASEDIR=$(pwd)
export PREFIX="${BASEDIR}/android/${ABI}"

# --- 工具链配置 ---
export TARGET_NAME="aarch64-linux-android"
export API=29
export CC="${TARGET_NAME}${API}-clang"
export CXX="${TARGET_NAME}${API}-clang++"
export AR=llvm-ar
export AS=llvm-as
export NM=llvm-nm
export RANLIB=llvm-ranlib
export STRIP=llvm-strip

# --- 编译参数 ---
# 1. 开启 PIC 解决之前的重定位报错
# 2. 开启 libvpx 用于 WebM
# 3. 开启所需的解复用器(demuxer)和解码器(decoder)
./configure \
  --prefix="${PREFIX}" \
  --target-os=android \
  --arch=aarch64 \
  --cpu=armv8-a \
  --enable-cross-compile \
  --sysroot="${TOOLCHAIN}/sysroot" \
  --enable-pic \
  --disable-shared \
  --enable-static \
  --disable-doc \
  --enable-mediacodec \
  --enable-pthreads \
  --enable-libvpx \
  --enable-libvorbis \
  --enable-libwebp \
  --disable-symver \
  --enable-decoder=bink \
  --enable-demuxer=bink \
  --disable-libdrm \
  --disable-vdpau \
  --disable-vaapi \
  --disable-programs \
  --enable-zlib \
  --enable-jni \
  --enable-mediacodec \
  --enable-hwaccels \
  --enable-parsers \
  --enable-protocols \
  --enable-filters \
  --enable-demuxer=matroska,webm,bink,mov,m4v,mp4 \
  --extra-cflags="-I${VPX_PATH}/include -fPIC -fstack-protector-strong -DANDROID" \
  --extra-ldflags="-L${VPX_PATH}/lib -Wl,--whole-archive -lvpx -Wl,--no-whole-archive -lm -lz -Wl,--hash-style=sysv"
  
  
  
  

# 开始编译
make clean
make -s -j$(nproc)
make install
