/* Definitions for all the commands useable */
/* (c) 1992 Dougal Scott */
/* $Header: /nelstaff/edp/dwagon/rfs/RCS/cmnd.h,v 1.46 1993/10/20 03:59:11 dwagon Exp $ */
/* $Log: cmnd.h,v $
 * Revision 1.46  1993/10/20  03:59:11  dwagon
 * Added GAMELEN command to change game length
 *
 * Revision 1.45  1992/11/09  03:20:31  dwagon
 * Added ore attack command
 *
 * Revision 1.44  1992/09/16  13:52:19  dwagon
 * Initial RCS'd version
 * */

#define NOOPERAT		0

/* Game length change */
#define GAMELEN			5

/* Name and gift orders */
#define NAMESHIP        10
#define NAMEPLAN        20
#define GIFTSHIP        40
#define GIFTPLAN        50

/* Formal Alliance changing orders */
#define ALLNCUP			70
#define ALLNCDN			80

/* Unload orders */
#define UNLODIND		100
/* Unload space mines */
#define UNLODSPM		105
#define UNLODDEF		110
#define UNLODORE		120
#define UNLODMIN		130
#define UNLODALL		145

/* Load orders */
#define LOADIND         200
/* Load Space Mines */
#define LOADSPM			205
#define LOADDEF         210
#define LOADORE         220
#define LOADMIN         240
/* Load all with priority list */
#define LOADPRI			243
#define LOADALL         245

/* Earth contract orders */
/* 2/5/92 Sell all ore */
#define SELLALL			305	
#define SELLORE         310
#define BUYORE          315
#define CONTSTART       320
#define CONTCARG        335
#define CONTFGHT        340
#define CONTTRAC		345
#define CONTSHLD		350
#define CONTEND         351

/* Spacemine deployment orders */
#define DEPLPLN			355
#define DEPLSHP			360
#define UNDEPL			365

/* Attack orders */
/* PDUs attack ship */
#define PLANATT			400
/* PDUs attack spacemines */
#define PLANSPM			405
#define ATTKSHP         410	
#define ATTKDEF         420
#define ATTKIND         430
#define ATTKMIN         440
/* Attack ore but not mines */
#define ATTKORE			442
/* Attack space mines */
#define ATTKSPM			445
/* Resolve all attacks */
#define RSLVATT			450
#define ENGTRAC			455

/* 6/5/92 Added Tending */
/* TEND orders */
#define TENDIND         480
#define TENDSPM			483
#define TENDDEF         486
#define TENDORE         489
#define TENDMIN         492
#define TENDALL         495

/* 19/6/92 Added unbuilding */
/* Ship destruction order */
#define UNBLDCAR		500
#define UNBLDFGT		510
#define UNBLDTRC		520
#define UNBLDSLD		530

/* 20/6/92 Moved trans to before building */
/* Transmute ore */
#define TRANSORE		550

/* Planetary build orders */
#define BUILDIND        600	
/* Build space mines */
#define BUILDSPM		605	
#define BUILDDEF        610
/* Rebuild Earth defences */
#define EARTHDEF		620
#define BUILDMIN        630
#define BUILDHYP        640

/* Ship build orders */
#define BUILDCAR		650
#define BUILDFGT		660
#define BUILDTRC		670
#define BUILDSLD		680

/* Movement Orders */
#define JUMP1           700	
#define JUMP2           705
#define JUMP3           710
#define JUMP4           715
#define JUMP5           720

/* Scanning orders */
#define SCAN			800	

/* Define Standing Orders */
#define STORD			900
