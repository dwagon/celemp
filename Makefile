# Makefile for Celestial Empire
# (c) 1992 Dougal Scott
# $Header: /nelstaff/edp/dwagon/rfs/RCS/Makefile,v 1.65 1993/11/02 02:41:03 dwagon Exp $
# $Log: Makefile,v $
# Revision 1.65  1993/11/02  02:41:03  dwagon
# Added typname.h dependencies to the functions that now include it
#
# Revision 1.64  1993/09/17  07:17:22  dwagon
# Remdae dependencies with new galcreat file planet.h
#
# Revision 1.63  1993/07/15  07:00:44  dwagon
# file.c is no put into the librfs library, rather than linked with all
# executables.
#
# Revision 1.62  1993/07/12  06:30:28  dwagon
# Fix put back in, but not made by default
#
# Revision 1.60.1.1  1993/07/12  06:25:29  dwagon
# Make should no longer make fix by default.
#
# Revision 1.60  1993/07/06  06:06:12  dwagon
# Make sure that no-one touches the .tex files, but chmoding them closed
#
# Revision 1.59  1993/07/06  04:44:37  dwagon
# Documentation now included in the source distribution
#
# Revision 1.58  1993/07/06  04:43:18  dwagon
# Added the administrators guide
#
# Revision 1.57  1993/06/23  07:55:15  dwagon
# Changed all rm calls to be forced and not to barf on errors
#
# Revision 1.56  1993/06/17  02:03:48  dwagon
# Removed bakup and bakall from everything command
#
# Revision 1.55  1993/06/16  08:55:40  dwagon
# Ignore errors caused by some files already being out, when doing co
#
# Revision 1.54  1993/05/20  03:33:35  dwagon
# Making dox, now sends proc.c to the archive machine as well.
#
# Revision 1.53  1993/05/19  04:00:41  dwagon
# Added texprocessing of latex files
#
# Revision 1.52  1993/02/19  02:28:10  dwagon
# galc.h now released with source to let other people be administrators.
#
# Revision 1.51  1992/11/29  01:40:08  dwagon
# Fixed bug with makedepend. All files are checked out before a makedepend,
# otherwise the dependency tree is broken.
#
# Revision 1.50  1992/11/10  04:01:03  dwagon
# Removed optimizing flag as GCC 2.3.1 generates bad code under the
# optimizer
#
# Revision 1.49  1992/11/09  12:31:21  dwagon
# Fixed path bug in bakup script
#
# Revision 1.48  1992/11/09  12:21:18  dwagon
# bakup changed to tar the RCS directory only
#
# Revision 1.47  1992/11/09  01:05:48  dwagon
# Make sure restricted galc.h doesn't get released in source code
#
# Revision 1.46  1992/11/09  01:04:19  dwagon
# Source backup now uses tar instead of zoo
#
# Revision 1.45  1992/09/16  14:43:27  dwagon
# Implemented make depend
#
# Revision 1.44  1992/09/16  14:06:44  dwagon
# Initial RCS'd version
#
#

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
	-co RCS/*
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

atk.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
atk.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
atk.o: /usr/include/unistd.h /usr/include/sys/param.h
atk.o: /usr/include/machine/param.h /usr/include/sys/signal.h
atk.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h proto.h
bld.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
bld.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
bld.o: /usr/include/unistd.h /usr/include/sys/param.h
bld.o: /usr/include/machine/param.h /usr/include/sys/signal.h
bld.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h proto.h
def.o: /usr/include/stdlib.h /usr/include/ansi_compat.h /usr/include/stdio.h
def.o: /usr/include/ctype.h /usr/include/string.h /usr/include/unistd.h
def.o: /usr/include/sys/param.h /usr/include/machine/param.h
def.o: /usr/include/sys/signal.h /usr/include/sys/types.h
def.o: /usr/include/sys/smp_lock.h version.h proto.h
earth.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
earth.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
earth.o: /usr/include/unistd.h /usr/include/sys/param.h
earth.o: /usr/include/machine/param.h /usr/include/sys/signal.h
earth.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
earth.o: proto.h
edit.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
edit.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
edit.o: /usr/include/unistd.h /usr/include/sys/param.h
edit.o: /usr/include/machine/param.h /usr/include/sys/signal.h
edit.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
edit.o: proto.h edit.h typname.h
file.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
file.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
file.o: /usr/include/unistd.h /usr/include/sys/param.h
file.o: /usr/include/machine/param.h /usr/include/sys/signal.h
file.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
file.o: proto.h
fix.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
fix.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
fix.o: /usr/include/unistd.h /usr/include/sys/param.h
fix.o: /usr/include/machine/param.h /usr/include/sys/signal.h
fix.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h proto.h
galcreat.o: /usr/include/math.h /usr/include/ansi_compat.h def.h
galcreat.o: /usr/include/stdlib.h /usr/include/stdio.h /usr/include/ctype.h
galcreat.o: /usr/include/string.h /usr/include/unistd.h
galcreat.o: /usr/include/sys/param.h /usr/include/machine/param.h
galcreat.o: /usr/include/sys/signal.h /usr/include/sys/types.h
galcreat.o: /usr/include/sys/smp_lock.h version.h proto.h galc.h galcreat.h
galcreat.o: planet.h
jump.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
jump.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
jump.o: /usr/include/unistd.h /usr/include/sys/param.h
jump.o: /usr/include/machine/param.h /usr/include/sys/signal.h
jump.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
jump.o: proto.h
libproto.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
libproto.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
libproto.o: /usr/include/unistd.h /usr/include/sys/param.h
libproto.o: /usr/include/machine/param.h /usr/include/sys/signal.h
libproto.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
libproto.o: proto.h
librfs.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
librfs.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
librfs.o: /usr/include/unistd.h /usr/include/sys/param.h
librfs.o: /usr/include/machine/param.h /usr/include/sys/signal.h
librfs.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
librfs.o: proto.h typname.h
load.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
load.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
load.o: /usr/include/unistd.h /usr/include/sys/param.h
load.o: /usr/include/machine/param.h /usr/include/sys/signal.h
load.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
load.o: proto.h
main.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
main.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
main.o: /usr/include/unistd.h /usr/include/sys/param.h
main.o: /usr/include/machine/param.h /usr/include/sys/signal.h
main.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
main.o: proto.h cmnd.h
misc.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
misc.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
misc.o: /usr/include/unistd.h /usr/include/sys/param.h
misc.o: /usr/include/machine/param.h /usr/include/sys/signal.h
misc.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
misc.o: proto.h
proc.o: /usr/include/stdio.h /usr/include/ansi_compat.h
proc.o: /usr/include/strings.h /usr/include/sys/types.h /usr/include/unistd.h
proc.o: /usr/include/errno.h /usr/include/limits.h
product.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
product.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
product.o: /usr/include/unistd.h /usr/include/sys/param.h
product.o: /usr/include/machine/param.h /usr/include/sys/signal.h
product.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
product.o: proto.h typname.h
sane.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
sane.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
sane.o: /usr/include/unistd.h /usr/include/sys/param.h
sane.o: /usr/include/machine/param.h /usr/include/sys/signal.h
sane.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
sane.o: proto.h
tend.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
tend.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
tend.o: /usr/include/unistd.h /usr/include/sys/param.h
tend.o: /usr/include/machine/param.h /usr/include/sys/signal.h
tend.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
tend.o: proto.h
trans.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
trans.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
trans.o: /usr/include/unistd.h /usr/include/sys/param.h
trans.o: /usr/include/machine/param.h /usr/include/sys/signal.h
trans.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
trans.o: proto.h trans.h cmnd.h allname.h
trans.o: /usr/include/ctype.h
turn.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
turn.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
turn.o: /usr/include/unistd.h /usr/include/sys/param.h
turn.o: /usr/include/machine/param.h /usr/include/sys/signal.h
turn.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
turn.o: proto.h turn.h typname.h allname.h
turn.o: /usr/include/string.h
unbld.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
unbld.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
unbld.o: /usr/include/unistd.h /usr/include/sys/param.h
unbld.o: /usr/include/machine/param.h /usr/include/sys/signal.h
unbld.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
unbld.o: proto.h
unload.o: def.h /usr/include/stdlib.h /usr/include/ansi_compat.h
unload.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
unload.o: /usr/include/unistd.h /usr/include/sys/param.h
unload.o: /usr/include/machine/param.h /usr/include/sys/signal.h
unload.o: /usr/include/sys/types.h /usr/include/sys/smp_lock.h version.h
unload.o: proto.h
