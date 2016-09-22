# Makefile for Celestial Empire
# (c) 2016 Dougal Scott

OBJS=atk.o bld.o earth.o jump.o load.o main.o misc.o product.o tend.o unbld.o unload.o 
CFLAGS= $(DFLAGS) -g3 -Wall -Wwrite-strings
STRIP=
LDFLAGS= $(STRIP) -L. -lrfs -lproto
CC=g++
AR=ar
DVIPS=dvips
CPP=/usr/lib/cpp
UTIL=sanity 
REMACH=cgl03.cc.monash.edu.au
ARCHMACH=yoyo.cc.monash.edu.au
ARCHDIR=/home/ftp/pub/celemp
PROC=texproc
TEX=latex
CAT=/bin/cat

.SUFFIXES:.zd .tex .dvi .ps .txt

.zd.tex:
	-rm -f $*.tex
	$(CAT) $< | $(PROC) > $*.tex
	chmod 0400 $*.tex

.tex.dvi:
	$(TEX) $<
	$(TEX) $<

.dvi.ps:
	dvips $<

.dvi.txt:
	dvi2tty -w 132 $< > $*.txt

all: libproto.a librfs.a $(UTIL) celemp galcreat turn trans edit celemp.ps celemp.txt admin.ps admin.txt

everything: all dox admin src 

bakall:
	tar -cf /tmp/rfsbak.tar .
	compress /tmp/rfsbak.tar

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
	
admin: admin.zd admin.txt admin.dvi admin.tex admin.ps

dox: celemp.zd celemp.txt celemp.dvi celemp.tex celemp.ps
	-co proc.c
	rcp proc.c $(ARCHMACH):$(ARCHDIR)/.
	compress celemp.zd
	rcp celemp.zd.Z $(ARCHMACH):$(ARCHDIR)/.
	uncompress celemp.zd
	compress celemp.txt
	rcp celemp.txt.Z $(ARCHMACH):$(ARCHDIR)/.
	uncompress celemp.txt
	compress celemp.dvi
	rcp celemp.dvi.Z $(ARCHMACH):$(ARCHDIR)/.
	uncompress celemp.dvi
	compress celemp.tex
	rcp celemp.tex.Z $(ARCHMACH):$(ARCHDIR)/.
	uncompress celemp.tex
	compress celemp.ps
	rcp celemp.ps.Z $(ARCHMACH):$(ARCHDIR)/.
	uncompress celemp.ps

src: 
	-co RCS/*
	tar -cf celempsrc.tar *.[ch] Makefile protofile *.zd
	compress celempsrc.tar
	rcp celempsrc.tar.Z $(ARCHMACH):$(ARCHDIR)/.
	-rm -f celempsrc.tar.Z

bakup:
	rcp $(REMACH):bak/rfsbak.tar.Z $(REMACH):bak/rfsbak2.tar.Z
	tar -cf /tmp/rfsbak.tar RCS
	compress /tmp/rfsbak.tar
	rcp /tmp/rfsbak.tar.Z $(REMACH):bak/rfsbak.tar.Z

fix: fix.c def.h librfs.a
	$(CC) $(CFLAGS) fix.c -o fix $(LDFLAGS)

celemp: $(OBJS) librfs.a
	$(CC) $(OBJS) $(CFLAGS) -o celemp $(LDFLAGS)

$(OBJS): def.h proto.h

clean:
	-rm -f *.o core *.aux *.lot *.toc *.log *.lof 

clobber: 
	-rm -f celemp galcreat turn trans celemp.ps edit celemp.dvi $(UTIL)
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
