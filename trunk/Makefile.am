oldladydir = $(pythondir)/oldlady
oldlady_PYTHON =	\
	__init__.py	\
	ai.py		\
	bridge.py	\
	ui.py
nodist_oldlady_PYTHON =	\
	defs.py

defs.py: defs.py.in
	sed -e "s|@xDATADIRx@|$(datadir)|"		\
	    -e "s|@xPKGDATADIRx@|$(pkgdatadir)|"	\
	    $< > $@

BUILT_SOURCES =		\
	defs.py

EXTRA_DIST =		\
	defs.py.in.in

CLEANFILES =			\
	$(BUILT_SOURCES)
