#!/bin/bash
# Builds a signed (debug key) APK: build-android/bloodrush.apk
# Requires ANDROID_HOME with platforms + build-tools, and an NDK
# (ANDROID_NDK, ANDROID_NDK_HOME or ANDROID_NDK_LATEST_HOME).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/build-android"
ABI="${ABI:-arm64-v8a}"

NDK="${ANDROID_NDK:-${ANDROID_NDK_HOME:-${ANDROID_NDK_LATEST_HOME:-}}}"
[ -n "$NDK" ] || { echo "error: no NDK found (set ANDROID_NDK)"; exit 1; }
BT="$ANDROID_HOME/build-tools/$(ls "$ANDROID_HOME/build-tools" | sort -V | tail -1)"
PLAT="$ANDROID_HOME/platforms/$(ls "$ANDROID_HOME/platforms" | sort -V | tail -1)"
echo "NDK: $NDK"
echo "build-tools: $BT"
echo "platform: $PLAT"

cmake -S "$ROOT" -B "$OUT" \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$OUT" -j"$(nproc)"

# --- package ---
STAGE="$OUT/apk"
rm -rf "$STAGE"
mkdir -p "$STAGE/lib/$ABI"
cp "$OUT/libbloodrush.so" "$STAGE/lib/$ABI/"

"$BT/aapt" package -f \
    -M "$ROOT/android/AndroidManifest.xml" \
    -S "$ROOT/android/res" \
    -I "$PLAT/android.jar" \
    -F "$OUT/bloodrush.unaligned.apk"
(cd "$STAGE" && "$BT/aapt" add "$OUT/bloodrush.unaligned.apk" "lib/$ABI/libbloodrush.so")

"$BT/zipalign" -f 4 "$OUT/bloodrush.unaligned.apk" "$OUT/bloodrush.unsigned.apk"

KS="$OUT/debug.keystore"
if [ ! -f "$KS" ]; then
    keytool -genkeypair -keystore "$KS" -storepass android -keypass android \
        -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US"
fi
"$BT/apksigner" sign --ks "$KS" --ks-pass pass:android --key-pass pass:android \
    --out "$OUT/bloodrush.apk" "$OUT/bloodrush.unsigned.apk"

echo "APK ready: $OUT/bloodrush.apk"
