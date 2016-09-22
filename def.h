/* Definitions for Celestial Empire  */
/* (c) 2016 Dougal Scott */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include "version.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Address to reply to */
#define NAME	"Zer Dwagon"
#define ADDRESS "dwagon"
#define HOST	"@pobox.com"

/* Constant definitions */
#define NUMPLAYERS		9
#define NUMSHIPS       200
#define NUMPLANETS     256
#define RESEARCH		200
#define EARTH			100
#define SHIP			0
#define PDU				1
#define BOUGHT			0
#define SOLD			1
#define TOOBIG			-1
#define NOFUEL			0
#define ENUFUEL			1
#define EFFGRAN			200
#define NEUTPLR			0

/* Definitions for alliances */
#define ENEMY			0
#define NEUTRAL			1
#define FRIEND			2
#define ALLY			3

/* definitions for galaxy creation */
#define NUMRES         27
#define NP              -1
#define UP              -2
#define ERTH            228

/* definitions for size of cmdarr array */
#define LENGTH 		1000

/* definitions for translation program */
#define ENDCOMMAND	-100
#define BLANK		-200
#define DEFENSE		11
#define SPACEMINE	12

/* Structures and sizes */
#define NAMESIZ	24
typedef short Player;
typedef short Oretype;
typedef short Link;
typedef short Planet;
typedef short Ship;
typedef short Units;
typedef int Amount;
typedef short Flag;
typedef int Number;

struct _planet {
char    name[NAMESIZ];	/* Name of the planet */
Player	owner;      	/* Owner of the planet */
Amount  ore[10],    	/* Stored ore on the planet */
        mine[10],   	/* Ore production on the planet */
        ind,        	/* Industry on the planet */
        indleft,    	/* Industry still unused on planet */
        pdu;        	/* PDU's on planet */
Link    link[4];    	/* Destination of the links */
Number  numlinks,   	/* Number of active links to planet */
        income;     	/* Income of the planet */
Number	spec;       	/* Speciality of planet, research or -plr home */
Number	scanned;		/* If scanned this turn and by who */
Amount 	spacemine;		/* Space mines */
Amount 	deployed;		/* Mines that are deployed around planet */
Units	pduleft;		/* Number of PDUs used this turn */
char	stndord[10];	/* Standing order */
char    spare[20];		/* Room for expansion */
};

struct _ship {
char    name[NAMESIZ]; 	/* Name of the ship */
Player	owner;          /* Owner of the ship */
Units   fight,          /* Number of fighter units on ship */
        cargo,          /* Number of cargo units on ship */
		shield,			/* number of shield units on ship */
		tractor;		/* Number of tractor units on ship */
Number	type;           /* Type of ship */
Amount  ore[10],        /* Amount of each type of ore on ship */
        ind,            /* Number of industry on ship */
        mines,          /* Number of mines on ship */
        pdu;            /* Number of pdu's on ship */
Units   cargleft;       /* Number of cargo units still unused */
Number	hits;           /* Number of shots that have been recieved */
Flag	engage,			/* Who engaged by+1, or if -ve who -engaging-1 */
		moved;			/* Whether already moved or attacked */
Planet  planet;         /* Planet which the ship orbits */
Number 	efficiency;		/* Drive efficiency */
Amount	spacemines;		/* Number of space mines carried */
Units	figleft;		/* Number of fighter units not used this turn */
char	stndord[10];	/* Standing order */
Number	pduhits;		/* Number of hits sustained from PDUs */
char 	spare[16];		/* Room for expansion */
};

/* Added 18/5/92 */
struct _game {
int winning;			/* What winning conditions apply */
int score;				/* If used what score wins */
int planets;			/* If used how many planets needed to win */
struct {
	Amount	ind;
	Amount	ore[10];
	Amount	mine[10];
	Amount	pdu;
	Amount	spacemine;
	Amount	deployed;
	int		spare[20];
	} home;	/* Initial Home planet */
struct {
	int 	amnesty;	/* When Earth amnesty ends */
	int		earthmult;	/* How many ecredits=1 score */
	int		fbid;		/* Minimum bid for fighter units */
	int		cbid;		/* Minimum bid for cargo units */
	int		sbid;		/* Minimum bid for shield units */
	int		tbid;		/* Minimum bid for tractor units */
	Amount	ind;
	Amount	ore[10];
	Amount	mine[10];
	Amount	pdu;
	Amount	spacemine;
	Amount	deployed;
	int		flag;		/* Earth behaviour modification flags */
	int		spare[16];
	} earth;	/* Initial Earth details */
struct {
	int		nomine;		/* Percent chance of having no mines on a planet */
	int		extramine;	/* Percent chance of having extra mines */
	int		extraore;	/* Percent chance of having extra mines */
	int		hasind;		/* Percent chance of having industry */
	int		haspdu;		/* Percent chance of having pdus */
	int		spare[20];
	} gal;		/* Galaxy setup details */
struct {
	int 	num;		/* Number of initial ships */
	int		fight;		/* Number of initial Fighter units */
	int		cargo;		/* Number of initial Cargo units */
	int		shield;		/* Number of initial Shield units */
	int		tractor;	/* Number of initial Tractor units */
	int		eff;		/* Initial efficiency */
	int		spare[20];
	} ship;		/* Initial Ship details */
struct {
	int 	num;		/* Number of initial ships */
	int		fight;		/* Number of initial Fighter units */
	int		cargo;		/* Number of initial Cargo units */
	int		shield;		/* Number of initial Shield units */
	int		tractor;	/* Number of initial Tractor units */
	int		eff;		/* Initial efficiency */
	int		spare[20];
	} ship2;	/* Initial Ship details */
int	income;		/* What income is needed to win */
int	credits;	/* What number of earth credits is needed to win */
int turn;		/* What turn is the current end turn */
int		spare[12];
};

/* Added 18/5/92 */
/* Winning condition flags */
#define WEARTH		0x01
#define WSCORE		0x02
#define WPLANETS	0x04
#define WINCOME		0x08
#define WCREDIT		0x10
#define WTURN		0x20
#define WFIXTURN	0x40

/* Added 18/5/92 */
typedef struct _planet planet;
typedef struct _ship ship;
typedef struct _game game;

/* Earth behaviour flags */
#define WBUYALLORE	0x01
#define WBUY100ORE	0x02

/* definition for scanned variable in planet structure */
#define PLR0	0x0200
#define PLR1	0x0001
#define PLR2	0x0002
#define PLR3	0x0004
#define PLR4	0x0008
#define PLR5	0x0010
#define PLR6	0x0020
#define PLR7	0x0040
#define PLR8	0x0080
#define PLR9	0x0100		

#include "proto.h"
