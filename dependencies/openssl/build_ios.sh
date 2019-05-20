#!/bin/sh

OPENSSL_DIR=`pwd`

build_one_arch()
{
  ARCH=$1
  DIR="$OPENSSL_DIR/build/$ARCH"

  case $ARCH in
    "armv7")
      TARGET=ios-cross
      SDK=iPhoneOS
      ;;
    "arm64")
      TARGET=ios64-cross
      SDK=iPhoneOS
      ;;
    "i386")
      TARGET=darwin-i386-cc
      SDK=iPhoneSimulator
      ;;
    "x86_64")
      TARGET=darwin64-x86_64-cc
      SDK=iPhoneSimulator
      ;;
    *)
      echo "Unknown architecture: $ARCH"
      exit 1
      ;;
  esac

  export CC=clang;
  export CROSS_TOP=/Applications/Xcode.app/Contents/Developer/Platforms/$SDK.platform/Developer
  export CROSS_SDK=$SDK.sdk
  export PATH="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin:$PATH"

  # openssl has no preset for simulator build, we need to add some flags for it.
  if [ $SDK == "iPhoneSimulator" ]
  then
    CFLAGS="-mios-version-min=6.0.0 -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK}"
    export CC="clang ${CFLAGS}"
  fi

  mkdir -p "$DIR"
  cd "$DIR"

  if [ ! -f "Makefile" ]
  then
    "$OPENSSL_DIR"/Configure no-shared no-dso no-hw no-engine $TARGET >configure.log 2>&1
  fi
  make >make.log 2>&1

  cp -pf "$DIR/include/openssl/opensslconf.h" "$FINAL_DIR/include/openssl/opensslconf_$ARCH.h"
  cp "$DIR/libcrypto.a" "$FINAL_DIR/lib/libcrypto_$ARCH.a"
  cp "$DIR/libssl.a" "$FINAL_DIR/lib/libssl_$ARCH.a"

  cd "$OPENSSL_DIR"
}

FINAL_DIR="$OPENSSL_DIR/build/final"

# Xcode should export ACTION before running the script
if [ "$ACTION" == "clean" ]
then
  rm -rf build
  exit 0
fi

mkdir -p "$FINAL_DIR/include/openssl/"
mkdir -p "$FINAL_DIR/lib"

for ARCH in $@
do
  echo "Building $ARCH..."
  build_one_arch $ARCH
done

echo "Copying headers..."
rsync -ru opensslconf.h include/openssl/* "$FINAL_DIR/include/openssl/"

echo "Creating universal libraries..."
lipo -create "$FINAL_DIR/lib/libssl_"* -output "$FINAL_DIR/lib/libssl.a"
lipo -create "$FINAL_DIR/lib/libcrypto_"* -output "$FINAL_DIR/lib/libcrypto.a"

rm -f "$FINAL_DIR/lib/libssl_"* "$FINAL_DIR/lib/libcrypto_"*
