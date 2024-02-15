=============================================================================================
=============================================================================================

THIS IS NOT AN ORIGINAL QT DISTRIBUTION  --- IT IS NOT MEANT FOR GENERAL USE

=============================================================================================
=============================================================================================

This archieve is based on the open-source distribution of Qt 4.8.7 provided by The Qt Company 
under terms of the LGPL license. Since we distribute Qt together with DESTool we are meant to 
provide the sources and to document how the binaries have been obtained.

We have applied a number of minor tweaks to link against LSB library stubs and to gain some 
cross-distribution binary compatibility for DESTool. To simplify deployment, we also added 
some fixes that target MS Visual C++ compilers. However, be warned: the tweaks are **very** 
pragmatic and it is not expected that this varaint of Qt will be good for anything else then 
to deploy DESTool. 

Allthough we are obliged to redistribute the patched sources by the applicable LGPL license, 
and allthough you are permitted to use them by the terms of that license, you should carefully 
inspect the committed changes to evaluate whether they meet your purposes.


=============================================================================================

*** Linux/LSB

Versions: LSB 4.0, Qt 4.8.7, GCC 4.7.2, LSB-SDK 4.1.7, all on Debian 7.4, built 2016/02

*** process

   start with plain qt source "qt-everywhere-opensource-4.8.7.tgz"
   mark all changes with "lsb/tm", try to be careful not to affect other architectures 
   never ever configure/build the source tree directly --- allways do this in a copy

*** LSB install: use install skript for complete install, move to /usr/opt/lsb; it is
   in general a good idea to use a recent LSB-SDK; the packages provided by most 
   Linux distributions will most likely not do the job.

*** set environment to locate lsb compilers 

  export PATH=$PATH:/opt/lsb/bin

*** fix "mkspecs/linux-lsb-g++" to explicitly set lsb version 4.0 

  (alternatively, we could have our own specs "mkspecs/linux-lsb-g++-faudes")

  set explicit target version to lsb 4.0 

  QMAKE_CC = lsbcc --lsb-target-version=4.0    
  QMAKE_CXX = lsbc++ --lsb-target-version=4.0    

  set fallback dynamic loader for target systems without lsb support

  QMAKE_LINK = lsbc++ --lsb-target-version=4.0  --lsb-besteffort
  QMAKE_LINK_SHLIB  = lsbc++ --lsb-target-version=4.0  --lsb-besteffort

  for the besteffort option to compile, the linker needs to find besteffor.o provided by LSB;
  pargmatically

  cp /opt/lsb/lib64-4.0/besteffort.o QTSOURCEPATH/lib/

  explicitly link to dynamic libs to make the LSB linker figure relevant stubs

  QMAKE_LIBS = -Wl,-as-needed -lrt -lz -lm -ldl -lX11 -lXrender -lXext -lfontconfig -lfreetype
 
  explicitly direct to 64-bit library stubs in version 4.0 (could also set a link in /opt/lsb)

  QMAKE_LIBDIR = /opt/lsb/lib64-4.0 
  QMAKE_LIBDIR_X11 = /opt/lsb/lib64-4.0 
  QMAKE_LIBDIR_OPENGL = /opt/lsb/lib64-4.0

  QMAKE_CFLAGS = -m64
  QMAKE_LFLAGS = -m64

  follow up in "mkspecs/features_qt_functions.prf"
  
  QMAKE_LFLAGS *= -L/opt/lsb/lib64-4.0    


*** configure options

  - use "-qt-libpng" to bypass missing "pngcpnf.h" in lsb system png
  - don't say anything on "sse", let configure figure it (will turn it all on)
  - configure as open source since we dont have a commercial license for 4.8.7 (will be prompted)
  - use "-no-cups", "-no-opengl" since we dont need it
  - use "-no-javascript-jit", "-no-openssl" since they wont compile
  - command line given below

*** qglobal.h --- may want to fix the QT_LINUXBASE macro (?)

*** freetype fix: 

  LSB 4.0 includes xrender, freetype and fontconfig - fine; however, Qt wont compile with 
  LSB system freetype for missinterpreting its version/capabilities; in fact, to my best
  knowledge freetype 2.1.10 does *not* provide FT_GlyphSlot_Embolden. I therefore manually
  disable access to that features

  file "src/gui/text/qfontengine_ft.cpp" line 100ff

  /* FreeType 2.1.10 starts to provide FT_GlyphSlot_Embolden */
  #if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20110
  #define Q_FT_GLYPHSLOT_EMBOLDEN(slot)   FT_GlyphSlot_Embolden(slot)
  #else
  #define Q_FT_GLYPHSLOT_EMBOLDEN(slot) 
  #endif

  /* lsb/tm: override for LSB 4.0 */
  #if defined (__LSB_VERSION__) && ( __LSB_VERSION__ <= 40 )
  #undef Q_FT_GLYPHSLOT_EMBOLDEN 
  #define Q_FT_GLYPHSLOT_EMBOLDEN(slot) 
  #endif

*** javascript fix

  the provided javascript tests the GCC version for advanced configuration; however, the test is 
  miss-evaluated for LSB compilers; thus, we manually disable the extra features; recall
  that we do not need javascript for DESTool, we just want to succeed in compiling. This job would
  be much easier if we could disable javascript alltogether.

  file "src/3rdparty/webkit/Source/JavaScriptCore/wtf/Atomics.h" line 73ff

  /* lsb/tm: must use bits/atomicity for LSB-complience */
  #if ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2))) && (! defined (__LSB_VERSION__) )
  #include <ext/atomicity.h>
  #else
  #include <bits/atomicity.h>
  #endif

  LSB 4.0 apparently does not provide timegm(); we therefor disable this feature

  file "src/3rdparty/javascriptcore/JavaScriptCore/wtf/Platform.h" line 730
  file "src/3rdparty/webkit/Source/JavaScriptCore/wtf/Platform.h" line 778

  /* lsb/tm: override for LSB 4.0 */
  #if defined (__LSB_VERSION__) && ( __LSB_VERSION__ <= 40 )
  #undef HAVE_TIMEGM 
  #endif

  the javascript runtime refers to "pthread_getattr_np" which is available only in lsb 4.1;
  there is the suggestion to fix this by loading the symbol dynamically; since we do not need
  javascript anyway, we dont really care about success at this end; 

  file "src/3rdparty/javascriptcore/JavaScriptCore/runtime/Collector.cpp" line 662
  file "src/3rdparty/webkit/Source/JavaScriptCore/wtf/StackBounds.cpp" line 173
  file "src/3rdparty/webkit/Source/JavaScriptCore/heap/MachineStackMarker.cpp" line 387

  compact replacement code for "pthread_getattr_np" by dynamic lookup (should have mutex?)

  // lsb/tm Collector.cpp line 109, i.e. within namespace
  #if defined (__LSB_VERSION__) && ( __LSB_VERSION__ <= 40 )
  #include<dlfcn.h> 
  int pthread_getattr_np(pthread_t thread, pthread_attr_t * attr) {
    typedef int (*PthreadGetattrNp)(pthread_t,pthread_attr_t*);
    static  PthreadGetattrNp pthread_getattr_np_ptr = NULL; 
    if(pthread_getattr_np_ptr==NULL)
      pthread_getattr_np_ptr =
        reinterpret_cast<PthreadGetattrNp>(dlsym(RTLD_DEFAULT, "pthread_getattr_np"));
    ASSERT(pthread_getattr_np_ptr!=NULL); 
    return pthread_getattr_np_ptr(thread,attr);
  }


  presumably harmless features missing here

  file "src/3rdparty/webkit/Source/JavaScriptCore/wtf/OSAllocatorPosix.cpp" line 76

  #if (! defined( __LSB_VERSION__ )) || ( defined (__LSB_VERSION__) && ( __LSB_VERSION__ > 40 ) )  // lsb/tm
  flags |= MAP_NORESERVE;  
  #endif


*** core libraray

  thread local storage is no available in LSB 4.0 but is detected by macros/configure;
  we need to disable this feature

  file "src/corelib/thread/qthread_unix.cpp" line 118

  #if defined(Q_OS_LINUX) && defined(__GLIBC__) && (defined(Q_CC_GNU) || defined(Q_CC_INTEL))
  #if (! defined( __LSB_VERSION__ )) || ( defined (__LSB_VERSION__) && ( __LSB_VERSION__ > 40 ) )  // lsb/tm
  #define HAVE_TLS 
  #endif
  #endif

** buildkey

   to allow non-LSB Qt installations to load our plug-ins viogen etc, we set a generic build key as 
   configure command option; to make sure that it is accepted, tweak ./configure line 8235

   # lsb/tm: override build key by commandline
   if [ ! -z "$CFG_USER_BUILD_KEY" ];
     QT_BUILD_KEY="$CFG_USER_BUILD_KEY"
     QT_BUILD_KEY_COMPAT="$CFG_USER_BUILD_KEY"
   fi


*** configure command (

./configure -v -release -opensource -fast -prefix /opt/qt-4.8.7-lsb-4.0 -buildkey "x86_64 linux g++-4 full-config" -platform linux-lsb-g++ -no-neon -no-avx -no-qt3support -no-pch -no-largefile -no-accessibility -no-xrandr -no-xvideo -no-xsync -no-xshape -no-sm -no-libmng -no-gif -qt-libpng -no-cups -no-openssl -no-phonon -no-opengl -no-javascript-jit -no-declarative -no-dbus -no-javascript-jit -no-script -no-scripttools -no-multimedia -webkit -nomake demos -nomake translations -nomake examples 

for a first basic test, additional options may include "-no-webkit -nomake tools -nomake docs -no-gui"

*** overall status 

  Qt compiles fine, DESTool fully functional


=============================================================================================
=============================================================================================

In order to comply with the LGPL, we also use this source archieve to build the Qt libraries
that we distribute with the MS Windows and Mac OsX variants of DESTool. Since the above patches 
dont affect other platforms than LSB, compilation should be straight by the books. 

*** MS Windows

Versions: Windows 7 64bit, compilers from MS Visual C++ 2015 (reporting version 19.00)

C:\> "unpack to destination folder, e.g. C:\Qt-4.8.7"
C:\> "optionally get "jom"
C:\> cd \Qt-4.8.7
C:\Qt-4.8.7> set QTDIR=C:\Qt-4.8.7 
C:\Qt-4.8.7> set QTMAKESPEC=win32-msvc2015
C:\Qt-4.8.7> set PATH=%QTDIR%\bin;%PATH%
C:\Qt-4.8.7> call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
C:\Qt-4.8.7> configure -debug-and-release -opensource -platform win32-msvc2015 -no-3dnow -no-neon -no-qt3support -no-openssl -no-phonon -no-opengl  -no-declarative -no-script -no-scripttools -no-multimedia -webkit -nomake demos -nomake translations -nomake examples 
> nmake (or "jom -j 20")

notes: make sure that you do not have other compilers (e.g. MinGW)in your PATH; best start with a
fresh command prompt --- and do all PATH business locally anyway

notes: allthough Qt-4.8.7 has a ready prepared win32-msvc2015 mkspec, javascript breaks the
build for minor compatibility issues. Adjustments in this regard are tagged "cl19/tm", affected files
are "TypeTraits.h", "StringExtras.h", "WebKit.pri","ArgList.h", two copies each; there are also some 
minor adjustments to CLucene, see "StdHeader.h" and "VoidMap.h"; most of the changes have been extracted 
from an "unofficial build", the CLucene fix is from "codereview.qt-project.org"; use e.g. "grep -r cl19" 
to locate the tweaks


*** Mac Os X

Versions: Os X 10.7, targetting 64bit only, gcc 4.2.1, using XCode provided toochaine dated Os X 10.7

./configure -v -debug-and-release -opensource -fast -prefix /usr/local/qt-4.8.7 -platform macx-g++ -no-libmng -no-gif -qt-libpng -no-cups -no-openssl -no-phonon -no-opengl -no-declarative -no-dbus -no-script -no-scripttools -no-multimedia -webkit -nomake demos -nomake translations -nomake examples 

notes: the official instructions ask for "make -j 1 install"

notes: not one tweak required here
