# 
#
# This file is copyrighted under the GNU Public License.
# See http://www.gnu.org/copyleft/gpl.html for further
# details.
#
# Makefile for "deal", a bridge hand generator.
#

TCL_DIR=/usr/local

CC=gcc
#DEBUG_FLAGS=-pg -g -ansi -Wall 
DEBUG_FLAGS=-O3 -ansi -Wall 

# Change TCL_INCL to point to the directory containing Tcl's
# include files
TCL_INCL=$(TCL_DIR)/include/tcl8.3
#TCL_INCL=./tcl8.1.1/generic

# Change TCL_LIB to point to the directory where you have a copy
# of libtcl.
# 
TCL_LIB=$(TCL_DIR)/lib

######
# For Solaris2, you need some more libraries
#
#LDFLAGS=  -L$(TCL_LIB) -ltcl80 -lm -ldl -lsocket -lnsl
LDFLAGS=  -L$(TCL_LIB) -ltcl8.4 -lm




#
ADVISOR=

# Since strdup is not implemented on some Ultrix systems
# (and is not required by ANSI C) you might want to
# uncomment the following line:
#
# EXTRAS_OBJS= util.o
#
EXTRA_OBJS= 

# On system which don't have "random" and "srandom",
# you might have to use the horrible "rand" for random
# number generation
#
#EXTRA_CFLAGS = -Drandom=rand -Dsrandom=srand
#
#EXTRA_CFLAGS = 
#EXTRA_CFLAGS = -DUSE_RAND48

COMPILE.c= $(CC) $(CFLAGS) $(CPPFLAGS) -c

CFLAGS= $(DEBUG_FLAGS) -I$(TCL_INCL) $(EXTRA_CFLAGS)


OBJS=random.o additive.o hand.o deal.o formats.o tcl_deal.o maindeal.o stat.o counttable.o \
	vector.o dist.o stringbox.o dealtypes.o keywords.o holdings.o $(EXTRA_OBJS) 
SRCS=additive.c hand.c deal.c formats.c tcl_deal.c dist.c vector.c stat.c counttable.c stringbox.c dealtypes.c holdings.c keywords.c maindeal.c random.c
SRCKIT=additive.c hand.c deal.c formats.c tcl_deal.c dist.c vector.c stat.c makecounttable.c stringbox.c dealtypes.c holdings.c keywords.c maindeal.c random.c
HFILES=deck.h deal.h tcl_incl.h vector.h stat.h tcl_dist.h dist.h formats.h additive.h stringbox.h dealtypes.h holdings.h keywords.h ansidecl.h
EXAMPLES= ex/*.tcl
BUILDFILES=Makefile Make.dep 
OTHERFILES=CHANGES LICENSE GPL input format lib deal.tcl docs

SOURCEKIT=$(SRCKIT) $(HFILES) $(EXAMPLES) $(BUILDFILES) $(OTHERFILES) $(EXTRAS)
UUKIT=$(EXAMPLES) $(OTHERFILES) deal

deal: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o deal $(LDFLAGS)

deal.cgi:
	make clean
	make CFLAGS="$(CFLAGS) -DCGI"

ideal: $(OBJS)
	proof $(CC) $(CFLAGS) $(OBJS) -o ideal $(LDFLAGS)

vector.so: vector.c
	$(CC) -c $(CFLAGS) -PIC vector.c
	ld vector.o -o vector.so

dist.so: dist.c
	$(CC) -c $(CFLAGS) -PIC dist.c
	ld dist.o -o dist.so

stringbox.so: stringbox.c
	$(CC) -c $(CFLAGS) -PIC stringbox.c
	ld stringbox.o -o stringbox.so

ccenter:
	 #load $(CFLAGS) $(SRCS) $(LDFLAGS)

ccentero: $(OBJS)
	 #load -w $(CFLAGS) $(OBJS) $(LDFLAGS)

cov:
	$(MAKE) CC="covc"
	cat qccov_work/*.M > deal.M

expert:
	$(MAKE) CC="exbuild $(CC)"

rtc:
	$(MAKE) clean
	$(MAKE) CC="/s/latest/CenterLine/bin/ncc -rtc -Xa" DEBUG_FLAGS=-g deal

$(SRCS): deal.h

counttable.c: makecounttable
	./makecounttable > counttable.c

makecounttable: makecounttable.c
	$(CC) $(CFLAGS) makecounttable.c -o makecounttable $(LDFLAGS)

checkin:
	checkin.script $(SOURCEKIT)

checkout:
	co -u $(SOURCEKIT)

firstcheckin:
	ci $(SOURCEKIT)

shar:
	mv Make.dep Make.dep.saved
	touch Make.dep
	shar $(SOURCEKIT) >deal.shar
	mv Make.dep.saved Make.dep

KITNAME=deal
DEAL_VERSION=305
OLD_VERSION=304
RELEASE=$(KITNAME)$(DEAL_VERSION)
SRCDIR=$(RELEASE)
SRCZIP=$(SRCDIR).zip
EXEZIP=$(SRCDIR)exe.zip
DOCZIP=$(SRCDIR)doc.zip
SRCTAR=$(SRCDIR).tar
SRCGZIP=$(SRCTAR).gz
BINZIP=$(KITNAME)exe.zip
DIFFZIP=deal$(DEAL_VERSION)diff.zip
OLDZIP=../deal/deal$(OLD_VERSION).zip
OLDDIR=$(KITNAME)$(OLD_VERSION)

FTP=../ftp

allzip: zip xzip dzip gzip diffzip

CHANGES: docs/html/CHANGES.txt
	cp docs/html/CHANGES.txt CHANGES

zip: $(SRCZIP) 

xzip: $(EXEZIP)

dzip: $(DOCZIP)

$(SRCDIR): $(SOURCEKIT) docs/html docs/graphics
	mv Make.dep Make.dep.saved
	touch Make.dep
	rm -rf $(SRCDIR)
	mkdir $(SRCDIR)
	ls -1d $(SOURCEKIT) | xargs tar cf - | (cd $(SRCDIR) ; tar xf -)
	mv Make.dep.saved Make.dep

$(SRCZIP): $(SRCDIR)
	zip -r $(SRCZIP) $(SRCDIR) -x \*~

$(EXEZIP): $(SRCDIR)
	rm -f $(EXEZIP)
	zip -r $(EXEZIP) $(SRCDIR)/ex $(SRCDIR)/input $(SRCDIR)/format $(SRCDIR)/docs $(SRCDIR)/CHANGES $(SRCDIR)/LICENSE $(SRCDIR)/GPL -x \*~

$(DOCZIP): $(SRCDIR)
	rm -f $(DOCZIP)
	zip -r $(DOCZIP) $(SRCDIR)/CHANGES $(SRCDIR)/LICENSE $(SRCDIR)/GPL $(SRCDIR)/docs -x \*~

gzip: $(SRCTAR).gz

$(SRCTAR).gz: $(SRCDIR)
	rm -f $(SRCTAR).gz
	tar cf $(SRCTAR) $(SRCDIR)
	gzip $(SRCTAR)

$(BINZIP): deal
	strip deal
	ls -1 $(UUKIT) | grep -v /RCS | xargs zip $(BINZIP)
	#zip $(BINZIP) $(UUKIT)

diffzip: $(DIFFZIP)

$(DIFFZIP): $(SRCZIP)
	rm -rf tempdir $(DIFFZIP)
	mkdir tempdir
	cd tempdir ; unzip ../$(SRCZIP) ; unzip ../$(OLDZIP)
	cd tempdir ; diff -r $(OLDDIR) $(SRCDIR) > ../diff.out || echo Done
	zip $(DIFFZIP) diff.out

tar:
	mv Make.dep Make.dep.saved
	touch Make.dep
	tar cvf deal.tar $(SOURCEKIT)
	mv Make.dep.saved Make.dep

ftp: $(SRCZIP)
	cp $(SRCZIP) $(FTP)

uu: deal
	touch uu.header
	cp uu.header deal.uu
	rm -f $(TARFILE).[zZ]
	strip deal
	tar crf $(TARFILE) $(UUKIT)
	$(COMPRESSOR) $(TARFILE)
	uuencode $(TARFILE).[zZ] $(TARFILE).[zZ] >>deal.uu

depends:
	$(CC) $(CFLAGS) -M *.c >Make.dep

test:
	##setopt sys_load_cflags #$${sys_load_cflags} -L/tmp

clean:
	rm -rf deal $(OBJS) $(SRCDIR) $(SRCZIP) $(SRCGZIP) $(DOCZIP) $(EXEZIP)


include Make.dep
