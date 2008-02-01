loginserver.tcl: loginserver.tcl_w loginserver.tcl_wtest
	./scat loginserver1.tcl loginserver.tcl loginserver.tcl_w loginserver.tcl_wtest loginserver2.tcl > new
	echo > loginserver.tcl_w
	echo > loginserver.tcl_wtest
	mv new loginserver.tcl
	touch loginserver.tcl

pwfile: loginserver.tcl
	fgrep "set {pw(" loginserver.tcl > $@


# "make dump" is run via ls.tcl
dump:
	@echo make dump is disabled
#	(./dumppw | gzip --to-stdout | crypt s1n2o3w4b5a6l7l8s9 > $(WEBDIR)/rawresults/p) || (echo make dump failed | mail floater@floater.org)


all: loginserver.tcl
