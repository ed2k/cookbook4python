# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2
#
# The contents of this file are subject to the Mozilla Public License Version 
# 1.1 (the "License"); you may not use this file except in compliance with 
# the License. You may obtain a copy of the License at 
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Floater Bridge Network.
#
# The Initial Developer of the Original Code is
# Geoff Pike  <pike@EECS.Berkeley.EDU>.
# Portions created by the Initial Developer are Copyright (C) 1996--2003
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#    Lex Spoon <lex@cc.gatech.edu>
#
# Alternatively, the contents of this file may be used under the terms
# of either the GNU General Public License Version 2 or later (the
# "GPL"), in which case the provisions of the GPL are applicable instead
# of those above. If you wish to allow use of your version of this file
# only under the terms of the GPL, and not to allow others to use your
# version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the
# notice and other provisions required by the GPL. If you do not delete
# the provisions above, a recipient may use your version of this file
# under the terms of either the MPL or the GPL.
# ***** END LICENSE BLOCK *****


#  This file is an input file used by the GNU "autoconf" program to
#  generate the file "configure".  "configure" is then used to
#  set up a build directory with paths and settings appropriate
#  to the current machine.


AC_INIT(Floater, 1.4.15)
AC_CONFIG_SRCDIR(common/floater.h)
AC_CONFIG_AUX_DIR(build)

AC_CANONICAL_SYSTEM



#------------------------------------------------------------------------
# differences of host
#------------------------------------------------------------------------
if (echo $target | grep -q mingw)
then
    windows=yes
else
    windows=no
fi
AC_SUBST(windows)

#------------------------------------------------------------------------
# options
#------------------------------------------------------------------------

if test "$windows"  = "no" -a -d loginserver
  # It shouldn't be hard, but no one has bothered to make the 
  # servers compile for Windows
then
	AC_ARG_ENABLE(server,
                      AS_HELP_STRING(--enable-server,
                                     [compile the server in addition to the client]),
                      ENABLE_SERVER=yes,
                      ENABLE_SERVER=no)
else
	ENABLE_SERVER=no
fi
AC_SUBST(ENABLE_SERVER)


# these should be real autoconf options, if they are to be supported
# at all
OPTION_SWITCHES="-DTCL_IN_C=1 -DUSER_TCL=1 -DBEEPONJOIN"

if test "$windows" = "yes"
then
    OPTION_SWITCHES="$OPTION_SWITCHES -DMS_WINDOWS"
    LDFLAGS="$LDFLAGS"
fi

AC_SUBST(OPTION_SWITCHES)
AC_SUBST(LD_FLAGS)


#------------------------------------------------------------------------
# directories
#------------------------------------------------------------------------

# XXX these eval's look funny.  Are they okay?  Check the autoconf pages.
webdir="$libdir/www"
AC_SUBST(webdir)

rsdir="$libdir/floater_rs"
AC_SUBST(rsdir)


# libdir must be a fully qualified path and (not ${exec_prefix}/lib)
# XXX this is suboptimal.  Uses in makefiles should be replaced by
# things like $srcdir, and uses in source code files should be
# replaced by *installation* directories, not the source directory.
# Once everything is replaced this strange var could be eliminated.

libdir="$libdir"
FLOATER_SRC_DIR=`cd $srcdir ; pwd`


#------------------------------------------------------------------------
# deal with different flavors of VERSION
#------------------------------------------------------------------------
VERSION=$PACKAGE_VERSION

#------------------------------------------------------------------------
# Standard compiler checks
#------------------------------------------------------------------------

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL


#------------------------------------------------------------------------
# other tools
#------------------------------------------------------------------------

if test "$windows" = "yes"
then
    AC_CHECK_TOOL(WINDRES, windres,
                  [AC_MSG_ERROR([Can't find windres.])])
fi


#--------------------------------------------------------------------
# Look for include files we need.
#--------------------------------------------------------------------
AC_HAVE_HEADERS(stdlib.h malloc.h string.h)

#--------------------------------------------------------------------
# See if we have a random() library call.  If not, define USE_RAND
#--------------------------------------------------------------------
AC_MSG_CHECKING(for random)
AC_TRY_LINK([#include <stdlib.h>], [random();], have_random=1, have_random=0)
if test "$have_random" = 1 ; then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
    AC_DEFINE(USE_RAND)
fi

#--------------------------------------------------------------------
#	Check for the existence of various libraries.  The order here
#	is important, so that then end up in the right order in the
#	command line generated by Make.
#--------------------------------------------------------------------
EXTRA_LIBS=""
AC_HAVE_LIBRARY(Xbsd, [EXTRA_LIBS="$EXTRA_LIBS -lXbsd"])
AC_HAVE_LIBRARY(socket, [EXTRA_LIBS="$EXTRA_LIBS -lsocket"])
AC_HAVE_LIBRARY(nsl, [EXTRA_LIBS="$EXTRA_LIBS -lnsl"])
AC_HAVE_LIBRARY(dnet_stub, [EXTRA_LIBS="EXTRA_$LIBS -ldnet_stub"])
AC_SUBST(EXTRA_LIBS)

#--------------------------------------------------------------------
# Find curses
#--------------------------------------------------------------------
AC_ARG_WITH(curses,
            AS_HELP_STRING(--with-curses=path,
                           [Specify a path for curses/ncurses]),
            curses=$withval,
            curses="none")

CURSES_INCLUDES=""
CURSES_LIB=""

if test "$curses" = "no"
then
    AC_SUBST(enable_backend_curses, "no")
else
    AC_SUBST(enable_backend_curses, "yes")
fi

if test "$curses" = "no" ; then
    AC_DEFINE(GUI_ONLY)
elif test "$curses" != "none" ; then
    # a path was given.  first look for ncurses
    if test -d "$curses/include/ncurses" ; then
	CURSES_INCLUDES="-I$curses/include/ncurses"
	if test -r $curses/lib/libncurses.a -o -r $curses/lib/libncurses.so ; then
	    CURSES_LIB="-L$curses/lib -lncurses"
	elif test -r $curses/lib/ncurses/libncurses.a -o -r $curses/lib/ncurses/libncurses.so -o -r $curses/lib/curses.lib ; then
	    CURSES_LIB="-L$curses/lib/ncurses -lncurses"
	else
	    AC_MSG_ERROR([can't find libncurses in $curses/lib.  Try running 
configure again with the --with-curses flag])
	fi
    fi

    # a path was given but ncurses wasn't there -- look for curses
    if test -z "$CURSES_INCLUDES" ; then
	if test -r $curses/include/curses.h ; then
	    CURSES_INCLUDES="-I$curses/include"
	fi
	if test -r $curses/lib/libcurses.a -o -r $curses/lib/libcurses.so -o -r$curses/lib/curses.lib ; then
	    CURSES_LIB="-L$curses/lib -lcurses"
	fi
    fi
fi

if test "$curses" != "no" -a -z "$CURSES_INCLUDES" ; then
    # we want to uses curses but haven't found the header files
    # yet.  Keep looking...
    AC_CHECK_HEADER(curses.h, , AC_MSG_ERROR([can't find curses.h.  Try 
running configure again with the --with-curses flag]))
fi

if test "$curses" != "no" -a -z "$CURSES_LIB" ; then
    # we want to uses curses but haven't found the library yet.
    AC_CHECK_LIB(ncurses, initscr, CURSES_LIB="-lncurses")
    if test -z "$CURSES_LIB" ; then
	AC_CHECK_LIB(curses, initscr, CURSES_LIB="-lcurses",
		     AC_MSG_ERROR([can't find libcurses.  Try running configure again with 
the --with-curses flag]))
    fi
fi

AC_SUBST(CURSES_INCLUDES)
AC_SUBST(CURSES_LIB)

#--------------------------------------------------------------------
# Find X11 header files and libraries
# (XXX really, this is the support files for Tk, and should be renamed
#  as such.  Floater tries very hard not to talk directly to platform
#  specific libraries.)
#--------------------------------------------------------------------

if test $windows = "yes" 
then
  AC_PATH_X
  X_INCLUDES="-I$x_includes"    #oddly, Windows compiling needs the X headers
  X_LIB="-lgdi32 -lcomdlg32 -lwinspool -lcomctl32 -limm32"
else
  AC_PATH_X

  X_INCLUDES="-I$x_includes"
  X_LIB="-L$x_libraries -lX11"
fi

AC_SUBST(X_INCLUDES)
AC_SUBST(X_LIB)

#--------------------------------------------------------------------
# Find Tcl/Tk
#--------------------------------------------------------------------

AC_ARG_WITH(tcl-include, --with-tcl-include=path Specify a path for tcl.h,
tcli=$withval, tcli="none")

AC_ARG_WITH(tcl-lib,
	    AS_HELP_STRING(--with-tcl-lib=path,
			   [Specify a path for the Tcl library]),
            tcl=$withval,
            tcl="none")

AC_MSG_CHECKING(for tcl.h)
if test "$tcli" = "none"
then
    places="../tcl8.4/generic \
        /usr/include/tcl8.4 \
        /usr/include \
        /usr/local/include \
        /usr/contrib/include \
        $prefix/include \
        $x_includes "

    for place in $places
    do
        if test "$tcli" = "none"
        then
           if (grep "TCL_VERSION" $place/tcl.h > /dev/null 2> /dev/null)
           then
              tcli="$place"
           fi
        fi
    done

    if test "$tcli" = "none" ; then
        AC_MSG_RESULT(nope)
        AC_MSG_ERROR([Can't find tcl.h.  Try the --with-tcl-include option.])
    fi
fi

if (grep "TCL_VERSION" $tcli/tcl.h > /dev/null)
then
    :
else
    AC_MSG_ERROR([Invalid location for tcl.h: $tcli])
fi


TCL_INCLUDES="-I$tcli"
tclver=`grep "TCL_VERSION" $tcli/tcl.h | tr -dc 0-9.`
smushed_tclver=`echo $tclver | tr -d .`
AC_MSG_RESULT([version $tclver in $tcli])




TCL_LIB="no"

if test "$tcl" = "none" ; then
    AC_CHECK_LIB(tcl$tclver, Tcl_Init, TCL_LIB="-ltcl$tclver")
fi

AC_MSG_CHECKING(for libtcl)
places="$tcl $tcl/lib \
        $tcl/unix \
        ../tcl$tclver/unix \
        /usr/local/lib \
        /usr/lib \
        /usr/contrib/lib \
        $prefix/lib \
        $x_libraries"


for place in $places
do
    if test "$TCL_LIB" = "no"
    then
	if test -r $place/libtcl$tclver.so -o -r $place/libtcl$tclver.so.1.0 -o -r $place/libtcl$tclver.a
        then
	    TCL_LIB="-L$place -ltcl$tclver"
        else if  test -r $place/libtcl$smushed_tclver.dll -o -r $place/libtcl$smushed_tclver.a
        then
	    TCL_LIB="-L$place -ltcl$smushed_tclver"
	fi fi
    fi
done

if test "$TCL_LIB" = "no"
then
    AC_MSG_RESULT(nope)
    AC_MSG_ERROR([Can't find libtcl.  Try the --with-tcl option.])
else
    AC_MSG_RESULT($place/libtcl$tclver)
fi


AC_ARG_WITH(tk-lib,	--with-tk-lib=path Specify a path for Tk, tk=$withval, tk="none")
AC_ARG_WITH(tk-include, --with-tk-include=path Specify a path for tk.h,
tki=$withval, tki="none")


TK_INCLUDES="no"

AC_MSG_CHECKING(for tk.h)
places="$tki $tki/generic $tk/generic $tk/include \
        ../tk$tclver/generic \
        /usr/local/include \
        /usr/include \
        /usr/contrib/include \
        $prefix/include \
        $x_includes \
        /usr/include/tcl$tclver"


for place in $places
do
    if test "$TK_INCLUDES" = "no"
    then
        if (grep "TK_VERSION.*\"$tclver" $place/tk.h > /dev/null 2> /dev/null)
        then
	    TK_INCLUDES="-I$place"
        fi
    fi
done

if test "$TK_INCLUDES" = "no" ; then
    AC_MSG_RESULT(nope)
    AC_MSG_ERROR([Can't find tk.h.  Try the --with-tk option.])
else
    AC_MSG_RESULT($TK_INCLUDES)
fi

TK_LIB="no"

if test "$tk" = "none"
then
    AC_CHECK_LIB(tk$tclver, Tk_Init, TK_LIB="-ltk$tclver")
fi

AC_MSG_CHECKING(for libtk)
places="$tk $tk/lib \
        $tk/unix \
	../tk$tclver/unix \
        /usr/local/lib \
        /usr/lib \
        /usr/contrib/lib \
        $prefix/lib \
        $x_libraries"
    

for place in $places
do
    if test "$TK_LIB" = "no"
    then
	if test -r $place/libtk$tclver.so -o -r $place/libtk$tclver.so.1.0 -o -r $place/libtk$tclver.a
        then
	    TK_LIB="-L$place -ltk$tclver"
        else if test -r $place/libtk$smushed_tclver.dll -o -r $place/libtk$smushed_tclver.a
        then
	    TK_LIB="-L$place -ltk$smushed_tclver"
	fi fi
    fi
done


if test "$TK_LIB" = "no"
then
    AC_MSG_RESULT(nope)
    AC_MSG_ERROR([Can't find libtk.  Try the --with-tk option.])
else
    AC_MSG_RESULT($place/libtk$tclver)
fi


# For Windows, add ../win to the include path for Tcl and Tk includes,
# so files can include things like tkWinInt.h
if test $windows = "yes"
then
  TCL_INCLUDES="$TCL_INCLUDES $TCL_INCLUDES/../win"
  TK_INCLUDES="$TK_INCLUDES $TK_INCLUDES/../win"
fi

AC_SUBST(TCL_INCLUDES)
AC_SUBST(TCL_LIB)
AC_SUBST(TK_INCLUDES)
AC_SUBST(TK_LIB)

#------------------------------------------------------------
# other system dependent things
#------------------------------------------------------------

# XXX this (combined with the hp-specific line below) is a lame hack:
# XXX what is it needed for ?!?!
AC_CHECK_LIB(dl, dlopen, EXTRA_LIBS="$EXTRA_LIBS -ldl")


case "$target" in 
  hppa*-hp-hpux9*)
    # XXX should really check directly in the curses lib for this
    AC_DEFINE(NO_HALFDELAY)
    
    #XXX
    EXTRA_LIBS=`echo $EXTRA_LIBS | sed -e 's/-ldl/-ldld/'`
    ;;
esac

AC_SUBST(VERSION)
AC_SUBST(FLOATER_SRC_DIR)


#------------------------------------------------------------
# what files to compile
#------------------------------------------------------------


FLOATER_PLATFORM_OBJS=""
if test $windows = "yes"
then
    FLOATER_PLATFORM_OBJS="maximize.o winMain.o winico.o"
fi
AC_SUBST(FLOATER_PLATFORM_OBJS)


#------------------------------------------------------------
# do the substitutions
#------------------------------------------------------------


AC_CONFIG_FILES(Jamsettings)


if test "$ENABLE_SERVER" = "yes" 
then
  for subdir in loginserver resultserver pseudomailer res2html
  do
    cd $subdir
    for f in ../common/*.c ../common/*.h ../common/ref ../common/tcl2c ../common/tclcode
    do
       ln -sf $f .
    done
    cd ..
  done
fi

if test "$windows" = "yes"
then
    AC_CONFIG_FILES(common/winico.rc)
fi

AC_OUTPUT

