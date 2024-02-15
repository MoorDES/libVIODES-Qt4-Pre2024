REM ================================== copy all viodiag libs


REM ========= copy qt libs
copy /Y %FAUDES_QT%\lib\QtCore4.dll release
copy /Y %FAUDES_QT%\lib\QtSvg4.dll release
copy /Y %FAUDES_QT%\lib\QtGui4.dll release
copy /Y %FAUDES_QT%\lib\QtXml4.dll release
copy /Y %FAUDES_MIN%\bin\mingwm10.dll release
copy /Y %FAUDES_PTHREAD%\lib\pthreadGC2.dll release
mkdir release\plugins
mkdir release\plugins\imageformats
copy /Y %FAUDES_QT%\plugins\imageformats\qjpeg4.dll release\plugins\imageformats
copy /Y %FAUDES_QT%\plugins\imageformats\qsvg4.dll release\plugins\imageformats
mkdir release\plugins\viotypes
copy /Y ..\libfaudes\libfaudes.dll  release
copy /Y ..\libfaudes\include\libfaudes.rti  release
copy /Y ..\viodes.dll release
copy /Y ..\viogen.dll release\plugins\viotypes
copy /Y ..\viohio.dll release\plugins\viotypes
copy /Y ..\viomtc.dll release\plugins\viotypes
copy /Y ..\viosim.dll release\plugins\viotypes
copy /Y ..\viodiag.dll release\plugins\viotypes
copy /Y ..\violua.dll release\plugins\viotypes
copy /Y data\vioconfig.txt release


