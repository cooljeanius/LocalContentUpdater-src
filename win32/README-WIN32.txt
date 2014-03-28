
** Macromedia Flash Local Content Updater
** Visual Studio project

The .dsp project file in this directory has been tested with MS Visual
C++ 6.0, SP5.  It will probably work with any version of MS Visual C++
6 or 7 (.NET).

To build this project, you must obtain source code for the free zlib
compression library.  Visit http://www.gzip.org/zlib/ for information
and downloads.  Place the zlib source in a directory called 'zlib' in
the same directory as the .dsp project file.  The .dsp expects to find
zlib.h and a statically linked zlib.lib in the zlib directory.

To build zlib.lib, we used the command line:

  cd zlib
  nmake -f win32\Makefile.msc zlib.lib

As of this writing, zlib's Makefile.msc builds by default with the /MD
option (link against MSVCRT.DLL).  You should change this option
before building zlib.lib, because all code in an executable should be
built with the same /M option.  LocalContentUpdater.cpp builds with
/MLd for the Debug configuration, and /ML for the Release
configuration.  Change the value of the CFLAGS variable in
zlib\win32\Makefile.msc accordingly.

Also as of this writing, the zlib distribution includes a
contrib\vstudio directory that contains a Visual C++ .NET project that
will build zlib.lib.  You may be able to use that project instead of
nmake if you wish.

