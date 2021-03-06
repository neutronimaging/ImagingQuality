#!/bin/bash
if [ `uname` == 'Linux' ]; then
    SPECSTR="-spec linux-g++"
else
    SPECSTR="-spec macx-clang CONFIG+=x86_64"
fi

REPOSPATH=$WORKSPACE/ImagingQuality

DEST=$WORKSPACE/builds

mkdir -p $DEST/build-NIQA
cd $DEST/build-NIQA

$QTBINPATH/qmake -makefile -r $SPECSTR -o Makefile ../../ImagingQuality/applications/NIQualityAssessment/NIQA/NIQA.pro
make -f Makefile clean
make -f Makefile mocables all
make -f Makefile

echo "Build tests"
if [ -e "$REPOSPATH/applications/NIQualityAssessment/UnitTests" ]; then
for f in `ls $REPOSPATH/applications/NIQualityAssessment/UnitTests`
do
	echo "$REPOSPATH/applications/NIQualityAssessment//UnitTests/$f/$f.pro"
	if [ -e "$REPOSPATH/applications/NIQualityAssessment//UnitTests/$f/$f.pro" ]
	then 
		mkdir -p $DEST/build-$f
		cd $DEST/build-$f

                $QTBINPATH/qmake -makefile -r $SPECSTR -o Makefile ../../imagingsuite/applications/NIQualityAssessment//UnitTests/$f/$f.pro
        make -f Makefile clean
        make -f Makefile mocables all
        make -f Makefile
	fi

done

echo "Tests built"
fi
