DIRECTORY=$WORKSPACE/deployed
QTPATH=$QTBINPATH/..
DEST="$DIRECTORY/NIQA.app"
REPOSPATH=$WORKSPACE
ARCH=`uname -m`

GITVER=`git rev-parse --short HEAD`

if [ ! -d "$DIRECTORY" ]; then
  mkdir $DIRECTORY
fi

cp -r $REPOSPATH/install/applications/NIQA.app $DIRECTORY

pushd .
CPCMD="cp"

cd $DEST/Contents
echo "This is where it lands..."
pwd
if [ ! -d "./Frameworks" ]; then
 mkdir ./Frameworks
fi

`$CPCMD $REPOSPATH/install/lib/libImagingAlgorithms.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libImagingQAAlgorithms.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libQtAddons.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libkipl.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libModuleConfig.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libReaderConfig.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libReaderGUI.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libQtModuleConfigure.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/install/lib/libQtImaging.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/ExternalDependencies/macos/$ARCH/lib/libNeXus.1.0.0.dylib $DEST/Contents/Frameworks`
`$CPCMD $REPOSPATH/ExternalDependencies/macos/$ARCH/lib/libNeXusCPP.1.0.0.dylib $DEST/Contents/Frameworks`
`$CPCMD /opt/local/lib/libhdf5.*.dylib $DEST/Contents/Frameworks`
`$CPCMD /opt/local/lib/libhdf5.*.dylib $DEST/Contents/Frameworks`
`$CPCMD /opt/local/lib/libhdf5.*.dylib $DEST/Contents/Frameworks`
#`$CPCMD $REPOSPATH/imagingsuite/external/mac/lib/libsz.2.dylib $DEST/Contents/Frameworks`

rm -f ./MacOS/*.dylib

cd Frameworks
rm -f *.1.0.dylib
rm -f *.1.dylib

if [ -e "/opt/local/lib/libzstd.1.dylib" ]; then
	`$CPCMD /opt/local/lib/libzstd.1.dylib $DEST/Contents/Frameworks`
fi

for f in `ls *.1.0.0.dylib`; do
	ln -s $f "`basename $f .1.0.0.dylib`.1.0.dylib"
	ln -s $f "`basename $f .1.0.0.dylib`.1.dylib"
	ln -s $f "`basename $f .1.0.0.dylib`.dylib"
done
cd ..



sed -i.bak s+com.yourcompany+ch.psi.www+g $DEST/Contents/Info.plist
echo "copy plugins"
pwd
if [ ! -d "./PlugIns" ]; then
 mkdir ./PlugIns
fi

if [ ! -d "./PlugIns/platforms" ]; then
 mkdir ./PlugIns/platforms
fi

if [ ! -f "./PlugIns/platforms/libqcocoa.dylib" ]; then 
	cp $QTPATH/plugins/platforms/libqcocoa.dylib $DEST/Contents/PlugIns/platforms/
fi

if [ ! -d "./PlugIns/printsupport" ]; then
 mkdir ./PlugIns/printsupport
fi

if [ ! -f "./PlugIns/printsupport/libcocoaprintersupport.dylib" ]; then 
	cp $QTPATH/plugins/printsupport/libcocoaprintersupport.dylib $DEST/Contents/PlugIns/printsupport/
fi

if [ ! -d "./PlugIns/accessible" ]; then
 mkdir ./PlugIns/accessible
fi

if [ ! -f "./PlugIns/accessible/libqtaccessiblewidgets.dylib" ]; then 
	if [ -f "$QTPATH/plugins/accessible/libqtaccessiblewidgets.dylib" ]; then 
		cp $QTPATH/plugins/accessible/libqtaccessiblewidgets.dylib $DEST/Contents/PlugIns/accessible/
	fi
fi

pwd
ls PlugIns

cd $QTPATH/bin/
echo "Do deploy..."
./macdeployqt $DEST #-dmg

cd $DEST/Contents/MacOS
# NIQA
install_name_tool -add_rpath @executable_path/../Frameworks NIQA
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib NIQA
# install_name_tool -change libQtAddons.1.dylib @executable_path/../Frameworks/libQtAddons.1.dylib NIQA
# install_name_tool -change libQtImaging.1.dylib @executable_path/../Frameworks/libQtImaging.1.dylib NIQA
# install_name_tool -change libImagingAlgorithms.1.dylib @executable_path/../Frameworks/libImagingAlgorithms.1.dylib NIQA
# install_name_tool -change libImagingQAAlgorithms.1.dylib @executable_path/../Frameworks/libImagingQAAlgorithms.1.dylib NIQA
# install_name_tool -change libReaderConfig.1.dylib @executable_path/../Frameworks/libReaderConfig.1.dylib NIQA
# install_name_tool -change libReaderGUI.1.dylib @executable_path/../Frameworks/libReaderGUI.1.dylib NIQA

# cd ../Frameworks

# # QtAddons
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib libQtAddons.1.0.0.dylib

# # QtImaging
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib libQtImaging.1.0.0.dylib
# install_name_tool -change libQtAddons.1.dylib @executable_path/../Frameworks/libQtAddons.1.dylib libQtImaging.1.0.0.dylib
# install_name_tool -change libReaderConfig.1.dylib @executable_path/../Frameworks/libReaderConfig.1.dylib libQtImaging.1.0.0.dylib
# install_name_tool -change libReaderGUI.1.dylib @executable_path/../Frameworks/libReaderConfig.1.dylib libQtImaging.1.0.0.dylib

# # ImagingAlgorithms
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib libImagingAlgorithms.1.0.0.dylib

# # ImagingQAAlgorithms
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib libImagingQAAlgorithms.1.0.0.dylib
# install_name_tool -change libImagingAlgorithms.1.dylib @executable_path/../Frameworks/libImagingAlgorithms.1.dylib libImagingQAAlgorithms.1.0.0.dylib

# # Reader config
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib libReaderConfig.1.0.0.dylib
# install_name_tool -change libImagingAlgorithms.1.dylib @executable_path/../Frameworks/libImagingAlgorithms.1.dylib libReaderConfig.1.0.0.dylib

# # reader GUI
# install_name_tool -change libkipl.1.dylib @executable_path/../Frameworks/libkipl.1.dylib libReaderGUI.1.0.0.dylib
# install_name_tool -change libReaderConfig.1.dylib @executable_path/../Frameworks/libReaderConfig.1.dylib libReaderGUI.1.0.0.dylib
# install_name_tool -change libQtAddons.1.dylib @executable_path/../Frameworks/libQtAddons.1.dylib libReaderGUI.1.0.0.dylib

# #nexus_related
# install_name_tool -change libNeXus.1.dylib @executable_path/../Frameworks/libNeXus.1.dylib libkipl.1.0.0.dylib
# install_name_tool -change libNeXusCPP.1.dylib @executable_path/../Frameworks/libNeXusCPP.1.dylib libkipl.1.0.0.dylib
# install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib libNexus.1.dylib
# install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib libNexusCPP.1.dylib
# install_name_tool -change libNeXus.1.dylib @executable_path/../Frameworks/libNeXus.1.dylib libNexusCPP.1.dylib

# install_name_tool -change libNeXus.1.dylib @executable_path/../Frameworks/libNeXus.1.dylib libQtImaging.1.0.0.dylib
# install_name_tool -change libNeXusCPP.1.dylib @executable_path/../Frameworks/libNeXusCPP.1.dylib libQtImaging.1.0.0.dylib
# install_name_tool -change libImagingAlgorithms.1.dylib @executable_path/../Frameworks/libImagingAlgorithms.1.dylib libQtImaging.1.0.0.dylib

# install_name_tool -change /usr/local/Cellar/hdf5/1.8.16_1/lib/libhdf5.10.dylib @executable_path/../Frameworks/libhdf5.10.dylib libhdf5_cpp.11.dylib
# install_name_tool -change /usr/local/Cellar/hdf5/1.8.16_1/lib/libhdf5.10.dylib @executable_path/../Frameworks/libhdf5.10.dylib libhdf5_hl.10.dylib

# install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_cpp.11.dylib @executable_path/../Frameworks/libhdf5_cpp.11.dylib libNeXus.1.0.0.dylib
# install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5.10.dylib @executable_path/../Frameworks/libhdf5.10.dylib libNeXus.1.0.0.dylib
# install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_hl.10.dylib @executable_path/../Frameworks/libhdf5_hl.10.dylib libNeXus.1.0.0.dylib
# install_name_tool -change  /usr/local/opt/szip/lib/libsz.2.dylib @executable_path/../Frameworks/libsz.2.dylib libNeXus.1.0.0.dylib

# install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_cpp.11.dylib @executable_path/../Frameworks/libhdf5_cpp.11.dylib libNeXusCPP.1.0.0.dylib
# install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5.10.dylib @executable_path/../Frameworks/libhdf5.10.dylib libNeXusCPP.1.0.0.dylib
# install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_hl.10.dylib @executable_path/../Frameworks/libhdf5_hl.10.dylib libNeXusCPP.1.0.0.dylib
# install_name_tool -change  /usr/local/opt/szip/lib/libsz.2.dylib @executable_path/../Frameworks/libsz.2.dylib libNeXusCPP.1.0.0.dylib

# install_name_tool -change /usr/local/opt/hdf5/lib/libhdf5.10.dylib libhdf5.10.dylib libhdf5.10.dylib
# install_name_tool -change /usr/local/opt/szip/lib/libsz.2.dylib @executable_path/../Frameworks/libsz.2.dylib libhdf5.10.dylib
# install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib libhdf5.10.dylib

# install_name_tool -change /usr/local/opt/hdf5/lib/libhdf5_cpp.11.dylib libhdf5_cpp.11.dylib libhdf5_cpp.11.dylib
# install_name_tool -change /usr/local/opt/szip/lib/libsz.2.dylib @executable_path/../Frameworks/libsz.2.dylib libhdf5_cpp.11.dylib
# install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib libhdf5_cpp.11.dylib

# install_name_tool -change /usr/local/opt/hdf5/lib/libhdf5_hl.10.dylib  libhdf5_hl.10.dylib libhdf5_hl.10.dylib
# install_name_tool -change /usr/local/opt/szip/lib/libsz.2.dylib @executable_path/../Frameworks/libsz.2.dylib libhdf5_hl.10.dylib
# install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib libhdf5_hl.10.dylib

rm -rf /tmp/NIQA

if [ ! -d "/tmp/NIQA" ]; then
  mkdir /tmp/NIQA
fi

if [ ! -e "/tmp/NIQA/Applications" ]; then
	ln -s /Applications /tmp/NIQA
fi

cp -r $DEST /tmp/NIQA

hdiutil create -volname NIQA -srcfolder /tmp/NIQA -ov -format UDZO $DIRECTORY/NIQA_build-$GITVER-`date +%Y%m%d`.dmg