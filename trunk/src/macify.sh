#!/bin/sh
APP_DIR=palx.app
CONTENTS_DIR=$APP_DIR/Contents
MACOS_DIR=$CONTENTS_DIR/MacOS
DYLIB_DIR=$CONTENTS_DIR/dylib
RES_DIR=$CONTENTS_DIR/Resources
LIBDIR=/opt/local/lib

rm -rf $APP_DIR

mkdir -p $CONTENTS_DIR
cp Info.plist $CONTENTS_DIR

mkdir -p $RES_DIR
cp palx.icns $RES_DIR

mkdir -p $MACOS_DIR
cp palx $MACOS_DIR
install_name_tool -change $LIBDIR/libbinio.1.dylib @loader_path/../dylib/libbinio.1.dylib $MACOS_DIR/palx
install_name_tool -change $LIBDIR/libadplug-2.1.0.dylib @loader_path/../dylib/libadplug-2.1.0.dylib $MACOS_DIR/palx
install_name_tool -change $LIBDIR/liballeg-4.2.2.dylib @loader_path/../dylib/liballeg-4.2.2.dylib $MACOS_DIR/palx
install_name_tool -change $LIBDIR/libfreetype.6.dylib @loader_path/../dylib/libfreetype.6.dylib $MACOS_DIR/palx
install_name_tool -change $LIBDIR/libiconv.2.dylib @loader_path/../dylib/libiconv.2.dylib $MACOS_DIR/palx
install_name_tool -change $LIBDIR/libz.1.2.3.dylib @loader_path/../dylib/libz.1.2.3.dylib $MACOS_DIR/palx

mkdir -p $DYLIB_DIR
cp $LIBDIR/libadplug-2.1.0.dylib $DYLIB_DIR
install_name_tool -change $LIBDIR/libbinio.1.dylib @loader_path/../dylib/libbinio.1.dylib $DYLIB_DIR/libadplug-2.1.0.dylib
cp $LIBDIR/libbinio.1.dylib $DYLIB_DIR
cp $LIBDIR/liballeg-4.2.2.dylib $DYLIB_DIR
cp $LIBDIR/libfreetype.6.dylib $DYLIB_DIR
cp $LIBDIR/libiconv.2.dylib $DYLIB_DIR
cp $LIBDIR/libz.1.2.3.dylib $DYLIB_DIR
