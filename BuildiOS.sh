INSTALL_ROOT=`pwd`

cd xcode/Stormancer
echo "Building device lib..."
xcodebuild -scheme Stormancer -destination generic/platform=iOS install DSTROOT="$INSTALL_ROOT/output/libs/ios-device"
echo "Building simulator lib..."
xcodebuild -scheme Stormancer -sdk iphonesimulator install DSTROOT="$INSTALL_ROOT/output/libs/ios-sim"

echo "Merging device and simulator libs..."
cd "$INSTALL_ROOT"
lipo -create output/libs/ios-device/*.a output/libs/ios-sim/*.a -output output/libs/libStormancer_Release_iOS.a
rm -rf output/libs/ios-device output/libs/ios-sim

echo "Done."
