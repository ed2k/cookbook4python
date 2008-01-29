*NO* warranty expressed or implied: use at your own risk!

Copyright 2006 by Alex Martelli (aleaxit@gmail.com)

This work is licensed under the Creative Commons Attribution 2.5
License (see http://creativecommons.org/licenses/by/2.5/).


These programs (where it matters, that is, for hand generation and
double-dummy evaluation) run on MacOSX 10.4.*, with Universal Python
2.4.3, on Macintosh machines with either Intel or PowerPC processors; or
else, on modern Linux distributions for Intel 32-bit chips, with a
regular build of Python 2.4.3.  Zipfile ddsprogs.zip expands into a
directory ddsprogs which has two directories, names linux85 and macosx,
each of which contains an executable named dealer and a Python extension
module named dds.so: if you want to use the Python script dodds.py
(which is part of what goes right into directory ddsprogs), copy file
dealer into a directory that is on your $PATH, and file dds.so into a
directory that is on your sys.path (choose the versions of these files
from the appropriate subdirectory of ddsprogs, of course).

Everything herein described expands from zipfile ddsprogs.zip right into
directory ddsprogs, and except for hand generation and double-dummy
analysis runs on any machine with a properly installed Python 2.4.3 or
later.  (To learn more about Python and frely download the needed
version, see http://www.python.org)

The intent is for this zipfile (and directory) to supply all you need
(besides Python, and a Mac of any kind [with the latest MacOSX] or a
Linux box [with a 32-bit intel CPU and a modern Linux distribution]) to
reproduce and extend the research results presented in file strat1.txt
(also present in this zipfile).  As per the above-mentioned Creative
Commons license, you may freely republish my work, and, if you wish, add
more materials to it, but if you do so them you must give me credit and
clearly distinguish your contributions from mine.


Contents of this zipfile and/or the containing directory:

dealer is a fat-binary build (or: Linux-86 binary executable) of the
hand-generation program whose sources and docs are available at
http://www.dombo.org/henk/dealer.html .

ntraise.dds is a controlfile for dealer, to deal flat (4333) hands with
15-17 HCP in North and 8-10 HCP in South.

dds.so is a MacOSX 10.4/Universal/Python 2.4.3 version (or: a
Linux-86/Python 2.4.3 version) of Bo Haglund's 'DDS', available at
http://web.telia.com/~u88910365/ in a pure Windows version, for double
dummy evaluation (sorry, no docs of the Python interfacing available
yet: reverse engineer it from the provided Python sources!).  The
sources from which dds.so was built are the property of Bo Haglund (Alex
Martelli provided only the porting to MacOSX as well as Linux, and
Python interfacing).  ((Special thanks to Bo for making the sources
available to me!!!))

dodds.py uses dealer and dds.so to generate and evaluate bridge deals.

In the same directory as this README.txt file (or the zipfile which
contains this README.txt file),
http://www.aleax.it/Bridge/dds1n2n.txt.bz2 is a bzip2 compressed version
the textual logfile dodds.py produced, with 400,000 deals used for these
research studies (one deal per line, preceded by the HCP counts of N and
S and the number of tricks that NS will make with North declaring
Notrumps at double-dummy.  Careful: this file is over 1OMB!  Because of
its huge size I have NOT included it in the zipfile, but rather just
compressed it separately with the best available free compressor.

summary.py can read the big logfile (must be bunzip2'd first!) and
summarize it into a Python dictionary; a pickled version of that
dictionary is in pick_dds.pick.  analyze.py reads the dictionary from
the .pick file and performs analysis of various kind (currently, you
must edit the sourcefile to determine exactly what analysis it does).


Assuming you don't want to regenerate and double-dummy analyze many
hands (which requires one or more macs or linux boxes, and many hours or
days), one suggested line of exploration is to change summary.py to
perform a different kind of hand evauation (not the HCP's which are
given in the logfile as a convenience), and analyze.py to use the proper
ranges and kinds of strategies for other hand-evaluation techniques.
All of this requires Python only (as well as bunzip2 for decompressing
the logfile) and thus can run on any machine having Python, be it a Mac
or otherwise.

Happy hacking!-)


Alex Martelli, Palo Alto (CA), May 23 2006

