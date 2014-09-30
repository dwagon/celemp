/******************************************************************************/
/* RFSLIB.C - Copyright (c) 1992 Zer Dwagon                                   */
/* Library file for Celestial Empire                                          */
/******************************************************************************/

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/librfs.c,v 1.53 1993/11/02 02:30:27 dwagon Exp $ */
/* $Log: librfs.c,v $
 * Revision 1.53  1993/11/02  02:30:27  dwagon
 * Changed the ship types around to be more similar to each other
 * Added DeathStartType() function
 *
 * Revision 1.52  1993/07/15  07:06:19  dwagon
 * Got rid off game number specific code as all games currently running
 * should use current options
 *
 * Revision 1.51  1993/07/12  06:18:46  dwagon
 * Changed special of home planet from -plr-10 to -plr
 *
 * Revision 1.50  1993/06/16  08:40:44  dwagon
 * Ship efficiency now applies to game 1 for testing purposes
 *
 * Revision 1.49  1993/06/16  08:17:01  dwagon
 * PDU and shield scaling occurs with game 1 for testing purposes
 *
 * Revision 1.48  1993/05/24  03:12:07  dwagon
 * Moved CalcPlrInc() to librfs from turn.c
 *
 * Revision 1.47  1993/05/19  04:01:26  dwagon
 * Added IsEmpty(), Shields() and Pdus()
 *
 * Revision 1.46  1993/05/15  05:57:03  dwagon
 * Added Pdus and Shield scaling functions
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  13:57:37  dwagon
 * Initial RCS'd version
 * */

/* 21/5/92	Added shots function from atk.c
 *			Converted all names to exclude underscores
 * 23/6/92	Added EffEff for jumping
 */

#include "def.h"
#include "typname.h"

#define TRLIB(x)	if(strstr(dbgstr,"LIB") || strstr(dbgstr,"lib")) x
#define TRLIB2(x)	if(strstr(dbgstr,"LI2") || strstr(dbgstr,"li2")) x

extern ship fleet[NUMSHIPS];
extern planet galaxy[NUMPLANETS];
extern Ship shiptr;
extern game gamedet;
extern int turn;
extern int gm;
extern char *dbgstr;

/*****************************************************************************/
int CalcPlrInc(Player plr)
/*****************************************************************************/
/* Calculate the income of a player */
{
int tmp=0,count,numres=NumRes(plr);

for(count=0;count<NUMPLANETS;count++) {
	if(galaxy[count].owner==plr)
		tmp+=galaxy[count].income;
	}
if(numres<4)
	tmp+=numres*numres*numres*numres;
else
	tmp+=256;
return(tmp);
}

/******************************************************************************/
int IsEmpty(Ship shp)
/******************************************************************************/
/* Is the ship an empty hulk? */
{
if(fleet[shp].cargo>0)
	return(0);
if(fleet[shp].fight>0)
	return(0);
if(fleet[shp].tractor>0)
	return(0);
if(fleet[shp].shield>0)
	return(0);
return(1);
}

/******************************************************************************/
int Pdus(Amount amnt)
/******************************************************************************/
/* Return the number of shots that amnt PDUs generate */
/* Vertices on graph:  {(0,1),(20,2),(100,3),(500,4)} */
{
TRLIB(printf("librfs:Pdus(amnt:%d)\n",amnt));
if(amnt>500)
	return(amnt*4);
if(amnt>100)
	return((int)((float)amnt*(0.0025*(float)amnt+2.75)));
if(amnt>20)
	return((int)((float)amnt*(0.0125*(float)amnt+1.75)));
return((int)((float)amnt*(0.05*(float)amnt+1)));
}

/******************************************************************************/
int Shields(Ship shp)
/******************************************************************************/
/* Return the number of shots that can be absorbed by the ship shp */
/* Vertices on shield graph {(0,1),(20,2),(90,4)} */
{
int totunits=0;
float ratio,shldrat;

TRLIB(printf("librfs:Shields(shp:%d)\n",shp));
totunits=fleet[shp].cargo+fleet[shp].fight+fleet[shp].tractor+fleet[shp].shield;
if(totunits==0) {
	TRLIB(printf("librfs:No units on ship\n"));
	return(0);
	}
if(fleet[shp].shield==0)
	return(0);
ratio=100*fleet[shp].shield/totunits;
TRLIB(printf("librfs:Shield ratio:%f\n",ratio));
if(ratio>90) {
	shldrat=4;
	TRLIB(printf("librfs:Shield class one: %f\n",shldrat));
	return((int)(fleet[shp].shield*shldrat));
	}
if(ratio<20) {
	shldrat=ratio/20+1;
	TRLIB(printf("librfs:Shield class three: %f\n",shldrat));
	return((int)(fleet[shp].shield*shldrat));
	}
shldrat=ratio*0.0286+1.4286;
TRLIB(printf("librfs:Shield class two: %f\n",shldrat));
return((int)(fleet[shp].shield*shldrat));
}

/******************************************************************************/
int CalcWeight(Ship shp)
/******************************************************************************/
{
int weight;

TRLIB(printf("librfs:Calcweight(shp:%d)\n",shp));

weight=1+fleet[shp].cargo+fleet[shp].tractor/2+fleet[shp].shield+(fleet[shp].cargo-fleet[shp].cargleft)/2;
weight+=fleet[shp].fight/10-fleet[shp].shield/2;
TRLIB(printf("librfs:Ship %d weighs %d\n",shp,weight));
return(weight);
}

/******************************************************************************/
Number CalcType(Ship shp)
/******************************************************************************/
/* Calculate the type of ship that it is        */
{
int  ratio;

TRLIB(printf("librfs:Calctype(shp: %d)\n",shp));

if (fleet[shp].cargo==0) {
	if(fleet[shp].fight==0)
		return(HULL);
	return(DeathStarType(shp));
	}
ratio=(100*fleet[shp].fight)/fleet[shp].cargo;
if(ratio>900)
	return(DeathStarType(shp));
if(ratio>200)
	return(BattleType(shp));
if(ratio>50)
	return(ShipType(shp));
return(CargoType(shp));
}

/******************************************************************************/
Number DeathStarType(Ship shp)
/******************************************************************************/
/* Calculate the type of Deathstar class ship that it is   */
{
Units f=fleet[shp].fight;

TRLIB(printf("librfs:DeathStarType(shp: %d)\n",shp));

if(f>250)
	return(UTDST);
if(f>150)
	return(MGDST);
if(f>75)
	return(SUDST);
if(f>30)
	return(LGDST);
if(f>10)
	return(MDDST);
return(SMDST);
}

/******************************************************************************/
Number BattleType(Ship shp)
/******************************************************************************/
/* Calculate the type of battle class ship that it is   */
{
Units f=fleet[shp].fight;

TRLIB(printf("librfs:BattleType(shp: %d)\n",shp));

if(f>250)
	return(UTBAT);
if(f>150)
	return(MGBAT);
if(f>75)
	return(SUBAT);
if(f>30)
	return(LGBAT);
if(f>10)
	return(MDBAT);
return(SMBAT);
}

/******************************************************************************/
Number ShipType(Ship shp)
/******************************************************************************/
/* Calculate the type of ship class ship that it is   */
{
Units f=fleet[shp].fight;

TRLIB(printf("librfs:ShipType(shp: %d)\n",shp));

if(f>250)
	return(UTSHP);
if(f>150)
	return(MGSHP);
if(f>75)
	return(SUSHP);
if(f>30)
	return(LGSHP);
if(f>10)
	return(MDSHP);
return(SMSHP);
}

/******************************************************************************/
Number CargoType(Ship shp)
/******************************************************************************/
/* Calculate the type of cargo class ship that it is   */
{
Units c=fleet[shp].cargo;

TRLIB(printf("librfs:CargoType(shp: %d)\n",shp));

if(c>250)
	return(UTCAR);
if(c>150)
	return(MGCAR);
if(c>75)
	return(SUCAR);
if(c>30)
	return(LGCAR);
if(c>10)
	return(MDCAR);
return(SMCAR);
}

/******************************************************************************/
int IsHome(Planet plan,Player plr)
/******************************************************************************/
/* Is the planet the players home planet */
{
TRLIB(printf("lib:IsHome(plan:%d,plr:%d)\n",plan,plr));

if(galaxy[plan].spec== -plr) {
	TRLIB(printf("IsHome:yes\n"));
	return(TRUE);
	}
else {
	TRLIB(printf("IsHome:no\n"));
	return(FALSE);
	}
}

/******************************************************************************/
int IsResearch(Planet plan)
/******************************************************************************/
/* Is the planet a research planet */
{
TRLIB2(printf("lib:IsResearch(plan:%d)\n",plan));

if(galaxy[plan].spec==RESEARCH) {
	TRLIB2(printf("IsResearch:yes\n"));
	return(TRUE);
	}
else {
	TRLIB2(printf("IsResearch:no\n"));
	return(FALSE);
	}
}

/******************************************************************************/
int IsEarth(Planet plan)
/******************************************************************************/
/* Is the planet Earth */
{
TRLIB(printf("lib:IsEarth(plan:%d)\n",plan));

if(galaxy[plan].spec==EARTH) {
	TRLIB(printf("IsEarth:yes\n"));
	return(TRUE);
	}
else {
	TRLIB(printf("IsEarth:yes\n"));
	return(FALSE);
	}
}

/*****************************************************************************/
int LocateEarth(void)
/*****************************************************************************/
/* Return which planet is Earth */
{
int count;

TRLIB(printf("LocateEarth\n"));
for(count=0;count<NUMPLANETS;count++) {
	if(galaxy[count].spec==EARTH) {
		TRLIB(printf("LocateEarth=%d\n",count));
		return(count);
		}
	}
	return(-1);
}

/*****************************************************************************/
int IsShip(Planet plan)
/*****************************************************************************/
/* Is there a ship on planet plan */
{
int count;

TRLIB(printf("isship(plan:%d)\n",plan));
for(count=0;count<shiptr;count++) {
	if(fleet[count].planet==plan) {
		TRLIB(printf("isship=%d\n",count));
		return(TRUE);
		}
	}
	return(FALSE);
}

/*****************************************************************************/
int NumRes(Player plr)
/*****************************************************************************/
/* Work out how many research planets the plr has */
{
int count,total=0;

TRLIB(printf("NumRes(plr:%d)\n",plr));
for(count=0;count<NUMPLANETS;count++)
	if(galaxy[count].owner==plr && IsResearch(count))
		total++;
return(total);
}

/*****************************************************************************/
int Shots(Ship shp,Amount shts)
/*****************************************************************************/
/* Calculate the number of shots that a ship has */
{
float ratio,tmpshts;
int weight;

TRLIB(printf("librfs:Shots(shp:%d,shts:%d)\n",shp,shts));

if(turn<gamedet.earth.amnesty && IsEarth(fleet[shp].planet)) {
	TRLIB(printf("librfs:Zero shots due to Earth amnesty\n"));
	return(0);
	}

weight=CalcWeight(shp);

ratio=(float)shts/(float)weight;
TRLIB(printf("librfs:Ship ratio:%f\n",ratio));

if(ratio>=10) {
	tmpshts=3*shts;
	TRLIB(printf("librfs:Class three ratio:%f\n",tmpshts));
	return((int)tmpshts);
	}

if(ratio<1) {
	tmpshts=((float)ratio*(float)shts);
	TRLIB(printf("librfs:Class one ratio:%f\n",tmpshts));
	return((int)tmpshts);
	}

tmpshts=(0.222*(float)(ratio)+0.777)*(float)shts;
TRLIB(printf("librfs:Class two ratio:%f\n",tmpshts));
return((int)tmpshts);
}

/*****************************************************************************/
int CalcIncome(Planet num)
/*****************************************************************************/
/* Calculate the income of a planet */
{
int inc=20;
int cnt;

TRLIB(printf("CalcIncome(num:%d)\n",num));
inc+=5*galaxy[num].ind;
for(cnt=0;cnt<10;cnt++)
	inc+=galaxy[num].mine[cnt];
return(inc);
}

/*****************************************************************************/
int EffEff(Ship shp)
/*****************************************************************************/
/* Calculate the effective efficiency of a ship */
{
int tot;

TRLIB(printf("EffEff(shp:%d)\n",shp));
tot=fleet[shp].cargo+fleet[shp].fight+fleet[shp].tractor+fleet[shp].shield;
TRLIB(printf("Game>=12:Eff=%d\n",MAX(4,fleet[shp].efficiency-tot/EFFGRAN)));
return(MIN(4,fleet[shp].efficiency-tot/EFFGRAN));
}
