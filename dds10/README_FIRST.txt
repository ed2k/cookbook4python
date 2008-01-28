This archive contains all of Bo Haglund's DDS 1.0 sources and docs also
available at Bo's site (http://web.telia.com/~u88910365/), with the
patch needed to make them portable and clean (warnings-less even when
compiled with -Wall) on Windows with the latest mingw32 compiler, on
MacOSX with gcc 4.0.1 (Xcode 2.2 or 2.3), and on Linux with gcc 3.3.5;
plus a wrapper that makes a Python extension module using DDS 1.0 for
Python 2.4 on any of the above platforms.  All the patches and Python
work are by Alex Martelli, aleaxit@gmail.com, and available at his site
http://www.aleax.it/Bridge/ .

In detail:

DDS_alg_descr-draft.rtf  Bo's draft document of his algorithms
DLL-dds_10_e.rtf         Bo's document for the DDS's DLL external interface
GPL.txt                  text of the GNU Public License
README_FIRST.txt         this file
dds.cpp                  the C++'s source for Bo's work (patched)
dll.h                    the C/C++ header for ditto (patched)
dds10.patch              the patchfile wrt Bo's published zipfile
readme.txt               Bo's notes re compilation options

setup.py                 the distutils file to compile everything
pydds.c                  sources for the pydds wrapper (early draft!)
t1                       a textfile with hands for trydds.py
trydds.py                simple Python 2.4 script to try out pydds

