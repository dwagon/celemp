/* Name file for the ship types */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/typname.h,v 1.45 1993/11/02 02:31:05 dwagon Exp $ */
/* $Log: typname.h,v $
 * Revision 1.45  1993/11/02  02:31:05  dwagon
 * Added lots more ship types to make each ship class similar.
 * Moved type defines to here from def.h
 *
 * Revision 1.44  1992/09/16  14:02:36  dwagon
 * Initial RCS'd version
 * */

/* Ship type definitions */
#define HULL	0
#define	SMCAR	1
#define MDCAR	2
#define	LGCAR	3
#define SUCAR	4
#define MGCAR	5
#define UTCAR	6
#define	SMSHP	7
#define MDSHP	8
#define	LGSHP	9
#define SUSHP	10
#define MGSHP	11
#define UTSHP	12
#define	SMBAT	13
#define MDBAT	14
#define	LGBAT	15
#define SUBAT	16
#define MGBAT	17
#define UTBAT	18
#define	SMDST	19
#define MDDST	20
#define	LGDST	21
#define SUDST	22
#define MGDST	23
#define UTDST	24
#define NUMTYPES 25

static char stypes[NUMTYPES][20] = {
	"Empty Ship", "Small Cargo","Medium Cargo","Large Cargo","Super Cargo",
	"Mega Cargo","Ultra Cargo", "Small Ship","Medium Ship","Large Ship",
	"Super Ship","Mega Ship","Ultra Ship", "Small Battle","Medium Battle",
	"Large Battle","Super Battle","Mega Battle","Ultra Battle", 
	"Small Deathstar","Medium Deathstar","Large Deathstar","Super Deathstar",
	"Mega Deathstar","Ultra Deathstar"
	};
