Promys.app: libs XCode/build/Release/Promys.app
	cp -a XCode/build/Release/Promys.app .

XCode/build/Release/Promys.app:
	xcodebuild -project XCode/Promys.xcodeproj -configuration Release
	
libs:
	cp ../Common/x264/libx264.*.dylib .
	cp ../Common/ffmpeg/libswscale/libswscale.*.dylib .
	cp ../Common/ffmpeg/libavutil/libavutil.*.dylib .
	install_name_tool -id @rpath/libswscale.5.dylib ./libswscale.5.dylib
	install_name_tool -id @rpath/libx264.155.dylib ./libx264.155.dylib
	install_name_tool -id @rpath/libavutil.56.dylib ./libavutil.56.dylib
	install_name_tool -change /usr/local/lib/libavutil.56.dylib @rpath/libavutil.56.dylib libswscale.5.dylib

clean:
	-rm -f *.dylib
	-rm -rf Promys.app XCode/build
