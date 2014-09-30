/* unbld  Has all the code for unbuilding everything */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/unbld.c,v 1.46 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: unbld.c,v $
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  14:03:30  dwagon
 * Initial RCS'd version
 * */

/*
 ****** Version 1.44 ******
 * 19/6/92	Module created
 */

#include "def.h"

#define TRUNB(x)	if(strstr(dbgstr,"UNBUILD") || strstr(dbgstr,"unbuild")) x

extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern Ship shiptr;
extern Player plr;
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern char *dbgstr;

/******************************************************************************/
void UnbldTrac(Ship shp,Amount amt)
/******************************************************************************/
/* Unbuild tractor units on ship */
{
Planet num=fleet[shp].planet;

TRUNB(printf("UnbldTrac(shp: %d,amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dU%dT\t",shp+100,amt);
if(plr!=fleet[shp].owner && alliance[plr][fleet[shp].owner]!=ALLY) {
	fprintf(stderr,"UnbldTrac:Plr %d doesnt own ship %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(plr!=galaxy[num].owner) {
	fprintf(stderr,"UnbldTrac:Plr %d doesnt own planet %d\n",plr+1,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(galaxy[num].indleft<amt*2) {	/* Check available industry */
	fprintf(trns[plr],"I ");
	amt=galaxy[num].indleft/2;
	}
galaxy[num].indleft-=amt*2;		/* Reduce industry available */
galaxy[num].ore[7]+=amt;		/* Reduce ore stockpile */
fleet[shp].tractor-=amt;		/* Increase tractor beams */
/* Recalc ship type - not-needed but could be in future */
fleet[shp].type=CalcType(shp);	
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dU%dT\n",shp+100,amt);
fprintf(trns[plr],"S%dU%dT\n",shp+100,amt);
}

/******************************************************************************/
void UnbldShield(Ship shp,Amount amt)
/******************************************************************************/
/* Unbuild shield units on ship */
{
Planet num=fleet[shp].planet;

TRUNB(printf("UnbldShield(shp: %d,amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dU%dS\t",shp+100,amt);
if(plr!=fleet[shp].owner && alliance[plr][fleet[shp].owner]!=ALLY) {
	fprintf(stderr,"UnbldShield:Plr %d does not own ship %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(plr!=galaxy[num].owner) {
	fprintf(stderr,"UnbldShield:Plr %d does not own planet %d\n",plr+1,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(galaxy[num].indleft<amt*2) {
	fprintf(trns[plr],"I ");
	amt=galaxy[num].indleft/2;
	}
fleet[shp].shield-=amt;
fleet[shp].type=CalcType(shp);
galaxy[num].indleft-=amt*2;
galaxy[num].ore[5]+=amt/2;
galaxy[num].ore[6]+=amt/2;
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dU%dS\n",shp+100,amt);
fprintf(trns[plr],"S%dU%dS\n",shp+100,amt);
}

/******************************************************************************/
void UnbldCargo(Ship shp,Amount amt)
/******************************************************************************/
/* Unbuild cargo units on ship  */
{
Planet   num;

TRUNB(printf("UnbldCargo(shp: %d, amt: %d)\n",shp,amt));

num = fleet[shp].planet;
fprintf(trns[plr],"S%dU%dC\t",shp+100,amt);
if(galaxy[num].owner!=plr) {
	fprintf(stderr,"UnbldCargo:Plr %d does not own planet %d\n",plr+1,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(fleet[shp].owner!=plr && alliance[plr][fleet[shp].owner]!=ALLY) { 
	fprintf(stderr,"UnbldCargo:Plr %d does not own ship %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(galaxy[num].indleft < amt) {
	fprintf(trns[plr],"I ");
	amt = galaxy[num].indleft;
	}
if(fleet[shp].cargleft<amt) {
	fprintf(trns[plr],"Cargo ");
	amt=fleet[shp].cargleft;
	}
galaxy[num].indleft-= amt;
galaxy[num].ore[1]+= amt/2;
fleet[shp].cargo-= amt;
fleet[shp].cargleft-= amt;
fleet[shp].type = CalcType(shp);
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dU%dC\n",shp+100,amt);
fprintf(trns[plr],"S%dU%dC\n",shp+100,amt);
}

/******************************************************************************/
void UnbldFight(Ship shp,Amount amt)
/******************************************************************************/
/* Unbuild fighter units on ship   */
{
Planet   num;

TRUNB(printf("UnbldFight(shp: %d, amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dU%dF\t",shp+100,amt);
num = fleet[shp].planet;
if(galaxy[num].owner!=plr) {
	fprintf(stderr,"UnbldFight:Plr %d does not own planet %d\n",plr+1,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(fleet[shp].owner!=plr && alliance[plr][fleet[shp].owner]!=ALLY) {
	fprintf(stderr,"UnbldFight:Plr %d does not own ship %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(galaxy[num].indleft<amt*2) {
	fprintf(trns[plr],"I ");
	amt=galaxy[num].indleft/2;
	}
galaxy[num].indleft-= amt*2;
galaxy[num].ore[2]+= amt/2;
galaxy[num].ore[3]+= amt/2;
fleet[shp].fight-= amt;
fleet[shp].type = CalcType(shp);
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dU%dF\n",shp+100,amt);
fprintf(trns[plr],"S%dU%dF\n",shp+100,amt);
}
