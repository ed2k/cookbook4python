# $Id: Makefile 18199 2006-05-21 14:36:05Z kukosa $

VERSION = 1.0.99

PROTOCOL_NAME=jser
DISSECTOR_FILES=packet-$(PROTOCOL_NAME).c packet-$(PROTOCOL_NAME).h
DOTSO=../../plugins/jser/.libs/jser.la ../../plugins/jser/.libs/jser.so


all: generate_dissector copy_files

generate_dissector: $(DISSECTOR_FILES) 

$(DISSECTOR_FILES): ../../tools/asn2wrs.py $(PROTOCOL_NAME).asn packet-$(PROTOCOL_NAME)-template.c packet-$(PROTOCOL_NAME)-template.h $(PROTOCOL_NAME).cnf
	$(PYTHON) "../../tools/asn2wrs.py"  -p $(PROTOCOL_NAME) -c $(PROTOCOL_NAME).cnf -s packet-$(PROTOCOL_NAME)-template $(PROTOCOL_NAME).asn

clean:
	rm -f $(OBJECTS) $(PROTOCOL_NAME).dll $(PROTOCOL_NAME).exp $(PROTOCOL_NAME).lib *.pdb

copy_files:
	cp $(DISSECTOR_FILES) ../../plugins/$(PROTOCOL_NAME)/.
	cd ../../plugins/jser/;make
	#cp $(DOTSO) /local/scratch/usr/local/lib/wireshark/plugins/$(VERSION)/
	cp $(DOTSO) ~/.wireshark/plugins/


