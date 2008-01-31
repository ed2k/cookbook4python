
tut.html: tutlog.tcl log2html.tcl
	cat log2html.tcl tutlog.tcl | /usr/local/bin/tclsh \
		| sed "s/Franco/Turbot/g" > tut.html

curses2text: curses2text.c
	gcc -O curses2text.c -o curses2text
