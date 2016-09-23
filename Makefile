# Makefile for Celestial Empire
# (c) 2016 Dougal Scott

OBJS=atk.o bld.o earth.o jump.o load.o main.o misc.o product.o tend.o unbld.o unload.o 
CFLAGS= $(DFLAGS) -g3 -Wall -Wwrite-strings
STRIP=
LDFLAGS= $(STRIP) -L. -lrfs -lproto
CC=gcc
AR=ar
UTIL=sanity 
PROC=texproc
TEX=latex
CAT=/bin/cat

.SUFFIXES:.zd .tex .dvi .pdf .txt

.zd.tex:
	-rm -f $*.tex
	$(CAT) $< | ./$(PROC) > $*.tex
	chmod 0400 $*.tex

.tex.dvi:
	$(TEX) $<
	$(TEX) $<

.dvi.pdf:
	dvipdf $<

.dvi.txt:
	dvi2tty -w 132 $< > $*.txt

all: libproto.a librfs.a $(UTIL) celemp galcreat turn trans edit celemp.pdf celemp.txt admin.pdf admin.txt

everything: all dox admin src 

bakall:
	tar -zcf /tmp/rfsbak.tar.gz .

depend:
	makedepend *.[ch]

librfs.a: librfs.o file.o
	$(AR) r librfs.a librfs.o file.o
	ranlib librfs.a

librfs.o: librfs.c def.h typname.h
	$(CC) $(CFLAGS) -c librfs.c

libproto.a: libproto.o
	$(AR) r libproto.a libproto.o
	ranlib libproto.a

libproto.o: libproto.c def.h
	$(CC) $(CFLAGS) -c libproto.c

sanity: sane.c def.h 
	$(CC) $(CFLAGS) sane.c -o sanity $(LDFLAGS)

texproc: proc.o
	$(CC) $(CFLAGS) proc.c -o texproc $(LDFLAGS)
	
admin: admin.zd admin.txt admin.dvi admin.tex admin.pdf

dox: celemp.zd celemp.txt celemp.dvi celemp.tex celemp.pdf

src: 
	tar -czf celempsrc.tar.gz *.[ch] Makefile protofile *.zd

bakup:
	tar -czf /tmp/rfsbak.tar.gz

fix: fix.c def.h librfs.a
	$(CC) $(CFLAGS) fix.c -o fix $(LDFLAGS)

celemp: $(OBJS) librfs.a
	$(CC) $(OBJS) $(CFLAGS) -o celemp $(LDFLAGS)

$(OBJS): def.h proto.h

clean:
	-rm -f *.o core *.aux *.lot *.toc *.log *.lof 

clobber: 
	-rm -f celemp galcreat turn trans celemp.pdf edit celemp.dvi $(UTIL)
	-rm -f librfs.a libproto.a admin.dvi admin.tex celemp.tex

nuke: clean clobber

galcreat: galcreat.o librfs.a libproto.a
	$(CC) galcreat.o $(CFLAGS) -o galcreat $(LDFLAGS)

trans: trans.o librfs.a cmnd.h allname.h
	$(CC) trans.o $(CFLAGS) -o trans $(LDFLAGS)

turn: turn.o librfs.a allname.h
	$(CC) turn.o $(CFLAGS) -o turn $(LDFLAGS)

edit: edit.o librfs.a libproto.a
	$(CC) edit.o $(CFLAGS) -o edit $(LDFLAGS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
