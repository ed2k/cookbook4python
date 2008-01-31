This directory holds various icons files.  The *.xcf files are the
originals, developed in gimp.  *.png and *.gif are exported from Gimp.
*.xpm are created via the makefile.  floater.ico is for Win32, and is
also updatd via the makefile.  floater.icns if for Mac's, and is
created manually on a Mac using Icon Composer.

All of these files are added to CVS, because they are rarely
changed and because they doesn't seem worth adding an extra build
dependency.

icon_images.c sets up some icons to be accessible to TCL code.
