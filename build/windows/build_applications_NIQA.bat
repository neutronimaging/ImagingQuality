call set_vars.bat
pushd .

set REPOSPATH=%WORKSPACE%\ImagingQuality
set DEST=%WORKSPACE%\builds
set SPECSTR=-spec win32-msvc

mkdir %DEST%\build-NIQA
cd %DEST%\build-NIQA


%QTBINPATH%\qmake.exe -makefile ..\..\ImagingQuality\applications\NIQualityAssessment\NIQA\NIQA.pro -o Makefile
%QTBINPATH%\..\..\..\Tools\QtCreator\bin\jom.exe -f Makefile clean
%QTBINPATH%\..\..\..\Tools\QtCreator\bin\jom.exe -f Makefile mocables all
%QTBINPATH%\..\..\..\Tools\QtCreator\bin\jom.exe -f Makefile release
