/* bld  Has all the code for building everything, except for Earth building */
/* Celestial Empire (c) 1992 Dougal Scott */
/* $Header: /nelstaff/edp/dwagon/rfs/RCS/bld.c,v 1.46 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: bld.c,v $
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  13:52:05  dwagon
 * Inital RCS'd version
 *
 * Revision 1.1  1992/09/16  13:46:43  dwagon
 * Initial revision
 * */

/* 2/5/92	Added ALLY status to building.  You can build on allies ships.
 * 3/5/92 	Allies are notified if you build units onto their ships.
 * 21/5/92	Converted names so no `_'s
 ****** VERSION 1.44 ******
 * 20/6/92	Fixed up ally system, for unballanced alliances
 */

#include "def.h"

#define TRBLD(x)	if(strstr(dbgstr,"BLD") || strstr(dbgstr,"bld")) x

extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern Ship shiptr;
extern Player plr;
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern char *dbgstr;

/******************************************************************************/
void BuildSpcmines(Planet plan,Amount amt,Oretype type)
/******************************************************************************/
/* Build space mines on a planet */
{
TRBLD(printf("BuildSpcmines(plan:%d,amt:%d,type:%d)\n",plan,amt,type));

fprintf(trns[plr],"%dB%dS%d\t",plan+100,amt,type);
if(galaxy[plan].owner!=plr) {
	fprintf(stderr,"BuildSpcmines:Plr %d doesn't own planet %d\n",plr,plan+100);
	fprintf(trns[plr],"You do not own planet %d\n",plan+100);
	return;
	}

if(galaxy[plan].indleft<amt) {
	fprintf(trns[plr],"I ");
	amt=galaxy[plan].indleft;
	}
if(galaxy[plan].ore[type]<amt) {
	fprintf(trns[plr],"R%d ",type);
	amt=galaxy[plan].ore[type];
	}
galaxy[plan].spacemine+=amt;
galaxy[plan].indleft-=amt;
galaxy[plan].ore[type]-=amt;
fprintf(trns[plr],"%dB%dS%d\n",plan+100,amt,type);
}

/******************************************************************************/
void BuildTrac(Ship shp,Amount amt)
/******************************************************************************/
/* Build tractor units on ship */
{
Planet num=fleet[shp].planet;

TRBLD(printf("BuildTrac(shp: %d,amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dB%dT\t",shp+100,amt);
if(plr!=fleet[shp].owner && alliance[fleet[shp].owner][plr]!=ALLY) {
	fprintf(stderr,"BuildTrac:Plr %d doesnt own ship %d\n",plr,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(plr!=galaxy[num].owner) {
	fprintf(stderr,"BuildTrac:Plr %d doesnt own planet %d\n",plr,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(galaxy[num].indleft<amt*2) {	/* Check available industry */
	fprintf(trns[plr],"I ");
	amt=galaxy[num].indleft/2;
	}
if(galaxy[num].ore[7]<amt*2) {	/* Check available ore */
	fprintf(trns[plr],"R7 ");
	amt=galaxy[num].ore[7]/2;
	}
galaxy[num].indleft-=amt*2;		/* Reduce industry available */
galaxy[num].ore[7]-=amt*2;		/* Reduce ore stockpile */
fleet[shp].tractor+=amt;		/* Increase tractor beams */
/* Recalc ship type - not-needed but could be in future */
fleet[shp].type=CalcType(shp);	
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dB%dT\n",shp+100,amt);
fprintf(trns[plr],"S%dB%dT\n",shp+100,amt);
}

/******************************************************************************/
void BuildShield(Ship shp,Amount amt)
/******************************************************************************/
/* Build shield units on ship */
{
Planet num=fleet[shp].planet;

TRBLD(printf("BuildShield(shp: %d,amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dB%dS\t",shp+100,amt);
if(plr!=fleet[shp].owner && alliance[fleet[shp].owner][plr]!=ALLY) {
	fprintf(stderr,"BuildShield:Plr %d does not own ship %d\n",plr,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(plr!=galaxy[num].owner) {
	fprintf(stderr,"BuildShield:Plr %d does not own planet %d\n",plr,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(galaxy[num].indleft<amt*2) {
	fprintf(trns[plr],"I ");
	amt=galaxy[num].indleft/2;
	}
if(galaxy[num].ore[5]<amt) {
	fprintf(trns[plr],"R5 ");
	amt=galaxy[num].ore[5];
	}
if(galaxy[num].ore[6]<amt) {
	fprintf(trns[plr],"R6 ");
	amt=galaxy[num].ore[6];
	}
fleet[shp].shield+=amt;
fleet[shp].type=CalcType(shp);
galaxy[num].indleft-=amt*2;
galaxy[num].ore[5]-=amt;
galaxy[num].ore[6]-=amt;
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dB%dS\n",shp+100,amt);
fprintf(trns[plr],"S%dB%dS\n",shp+100,amt);
}

/******************************************************************************/
void BuildInd(Planet sbj,Amount amt)
/******************************************************************************/
/* Build industry on planet   */
{
TRBLD(printf("BuildInd(sbj: %d,amt: %d)\n",sbj,amt));

fprintf(trns[plr],"%dB%dI\t",sbj+100,amt);
if(plr != galaxy[sbj].owner) {
	fprintf(stderr,"BuildInd:Plr %d doesnt own planet %d\n",plr,sbj+100);
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	return;
	}

if(galaxy[sbj].indleft < (10*amt)) {
	fprintf(trns[plr],"I ");
	amt = galaxy[sbj].indleft/10;
	}
if(galaxy[sbj].ore[8] < (5*amt)) {
	fprintf(trns[plr],"R8 ");
	amt = galaxy[sbj].ore[8]/5;
	}
if(galaxy[sbj].ore[9] < (5*amt)) {
	fprintf(trns[plr],"R9 ");
	amt = galaxy[sbj].ore[9]/5;
	}
galaxy[sbj].indleft -= 10*amt;
galaxy[sbj].ore[8] -= 5*amt;
galaxy[sbj].ore[9] -= 5*amt;
galaxy[sbj].ind += amt;
galaxy[sbj].income+=amt*5;
fprintf(trns[plr],"%dB%dI\n",sbj+100,amt);
}

/******************************************************************************/
void BuildMine(Planet sbj,Oretype type,Amount amt)
/******************************************************************************/
/* Build mine on planet */
{
TRBLD(printf("BuildMine(sbj: %d,type: %d,amt: %d)\n",sbj,type,amt));

fprintf(trns[plr],"%dB%dM%d\t",sbj+100,amt,type);
if(plr!=galaxy[sbj].owner) {
	fprintf(stderr,"BuildMine:Plr %d does not own planet %d\n",plr,sbj+100);
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	return;
	}

if(galaxy[sbj].indleft<10*amt) {
	fprintf(trns[plr],"I ");
	amt = galaxy[sbj].indleft/10;
	}
if(galaxy[sbj].ore[8]<5*amt) {
	fprintf(trns[plr],"R8 ");
	amt = galaxy[sbj].ore[8]/5;
	}
if(galaxy[sbj].ore[9]<5*amt) {
	fprintf(trns[plr],"R9 ");
	amt = galaxy[sbj].ore[9]/5;
	}
galaxy[sbj].indleft -= 10*amt;
galaxy[sbj].ore[8] -= 5*amt;
galaxy[sbj].ore[9] -= 5*amt;
galaxy[sbj].mine[type] += amt;
galaxy[sbj].income+=amt;
fprintf(trns[plr],"%dB%dM%d\n",sbj+100,amt,type);
}

/******************************************************************************/
void BuildHyp(Planet sbj,Amount amt,Units fgt,Units crg,Units trac,Units shld)
/******************************************************************************/
/* Build hyperdrive on planet */
{
int  count;

TRBLD(printf("BuildHyp(sbj:%d,amt:%d,fgt:%d,crg:%d,trac:%d,shld:%d)\n",sbj,amt,fgt,crg,trac,shld));

fprintf(trns[plr],"%dB%dH%d/%d/%d/%d\t",sbj+100,amt,fgt,crg,trac,shld);

if(shiptr+amt>NUMSHIPS) {
	amt = NUMSHIPS-shiptr;
	fprintf(trns[plr],"Max ships reached ");
	}

if(amt<0) {
	fprintf(stderr,"BuildHyp:Too many ships\n");
	fprintf(trns[plr],"Maximum number of ships in the galaxy reached\n");
	return;
	}

if(plr!=galaxy[sbj].owner) {
	fprintf(stderr,"BuildHyp:Plr %d does not own planet %d\n",plr,sbj+100);
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	return;
	}

if(galaxy[sbj].indleft<(40+fgt*2+crg+2*trac+2*shld)*amt) {
	fprintf(trns[plr],"I ");
	amt = galaxy[sbj].indleft/(40+fgt*2+crg+2*trac+2*shld);
	}
if(galaxy[sbj].ore[4]<10*amt) {
	fprintf(trns[plr],"R4 ");
	amt = galaxy[sbj].ore[4]/10;
	}
if(galaxy[sbj].ore[5]<(10+shld)*amt) {
	fprintf(trns[plr],"R5 ");
	amt = galaxy[sbj].ore[5]/(10+shld);
	}
if(galaxy[sbj].ore[6]<(10+shld)*amt) {
	fprintf(trns[plr],"R6 ");
	amt = galaxy[sbj].ore[6]/(10+shld);
	}
if(galaxy[sbj].ore[7]<(10+2*trac)*amt) {
	fprintf(trns[plr],"R7 ");
	amt = galaxy[sbj].ore[7]/(10+2*trac);
	}
if(galaxy[sbj].ore[1]<crg*amt) {
	fprintf(trns[plr],"R1 ");
	amt = galaxy[sbj].ore[1]/crg;
	}
if(galaxy[sbj].ore[2]<fgt*amt) {
	fprintf(trns[plr],"R2 ");
	amt = galaxy[sbj].ore[2]/fgt;
	}
if(galaxy[sbj].ore[3]<fgt*amt) {
	fprintf(trns[plr],"R3 ");
	amt = galaxy[sbj].ore[3]/fgt;
	}
galaxy[sbj].indleft -= (40+fgt*2+crg)*amt;
galaxy[sbj].ore[4] -= 10*amt;
galaxy[sbj].ore[5] -= (10+shld)*amt;
galaxy[sbj].ore[6] -= (10+shld)*amt;
galaxy[sbj].ore[7] -= (10+2*trac)*amt;
galaxy[sbj].ore[1] -= crg*amt;
galaxy[sbj].ore[2] -= fgt*amt;
galaxy[sbj].ore[3] -= fgt*amt;
for (count = 0;count<amt;count++) {
	fleet[shiptr].owner = plr;
	fleet[shiptr].cargo = crg;
	fleet[shiptr].fight = fgt;
	fleet[shiptr].tractor=trac;
	fleet[shiptr].shield=shld;
	fleet[shiptr].cargleft = crg;
	fleet[shiptr].planet = sbj;
	fleet[shiptr].engage=0;
	fleet[shiptr].type = CalcType(shiptr);
	shiptr++;
	}
fprintf(trns[plr],"%dB%dH%d/%d/%d/%d\n",sbj+100,amt,fgt,crg,trac,shld);
}

/******************************************************************************/
void BuildDef(Planet num,Amount amt)
/******************************************************************************/
/* Build PDU's on planet   */
{
TRBLD(printf("BuildDef(num: %d,amt: %d)\n",num,amt));

fprintf(trns[plr],"%dB%dD\t",num+100,amt);
if(galaxy[num].owner!=plr) {
	fprintf(stderr,"BuildDef:Plr %d doesnt own planet %d\n",plr,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(galaxy[num].indleft<amt) {
	fprintf(trns[plr],"I ");
	amt = galaxy[num].indleft;
	}
if(galaxy[num].ore[4]<amt) {
	fprintf(trns[plr],"R4 ");
	amt = galaxy[num].ore[4];
	}
galaxy[num].indleft-= amt;
galaxy[num].ore[4]-= amt;
galaxy[num].pdu+= amt;
fprintf(trns[plr],"%dB%dD\n",num+100,amt);
}

/******************************************************************************/
void BuildCargo(Ship shp,Amount amt)
/******************************************************************************/
/* Build cargo units on ship  */
{
Planet   num;

TRBLD(printf("BuildCargo(shp: %d, amt: %d)\n",shp,amt));

num = fleet[shp].planet;
fprintf(trns[plr],"S%dB%dC\t",shp+100,amt);
if(galaxy[num].owner!=plr) {
	fprintf(stderr,"BuildCargo:Plr %d does not own planet %d\n",plr,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(fleet[shp].owner!=plr && alliance[fleet[shp].owner][plr]!=ALLY) { 
	fprintf(stderr,"BuildCargo:Plr %d does not own ship %d\n",plr,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(galaxy[num].indleft < amt) {
	fprintf(trns[plr],"I ");
	amt = galaxy[num].indleft;
	}
if(galaxy[num].ore[1] < amt)  {
	fprintf(trns[plr],"R1 ");
	amt = galaxy[num].ore[1];
	}
galaxy[num].indleft-= amt;
galaxy[num].ore[1]-= amt;
fleet[shp].cargo+= amt;
fleet[shp].cargleft+= amt;
fleet[shp].type = CalcType(shp);
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dB%dC\n",shp+100,amt);
fprintf(trns[plr],"S%dB%dC\n",shp+100,amt);
}

/******************************************************************************/
void BuildFight(Ship shp,Amount amt)
/******************************************************************************/
/* Build fighter units on ship   */
{
Planet   num;

TRBLD(printf("BuildFight(shp: %d, amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dB%dF\t",shp+100,amt);
num = fleet[shp].planet;
if(galaxy[num].owner!=plr) {
	fprintf(stderr,"BuildFight:Plr %d does not own planet %d\n",plr,num+100);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}

if(fleet[shp].owner!=plr && alliance[fleet[shp].owner][plr]!=ALLY) {
	fprintf(stderr,"BuildFight:Plr %d does not own ship %d\n",plr,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(galaxy[num].indleft<amt*2) {
	fprintf(trns[plr],"I ");
	amt=galaxy[num].indleft/2;
	}
if(galaxy[num].ore[2]<amt) {
	fprintf(trns[plr],"R6 ");
	amt=galaxy[num].ore[2];
	}
if(galaxy[num].ore[3] < amt) {
	fprintf(trns[plr],"R6 ");
	amt = galaxy[num].ore[3];
	}
galaxy[num].indleft-= amt*2;
galaxy[num].ore[2]-= amt;
galaxy[num].ore[3]-= amt;
fleet[shp].fight+= amt;
fleet[shp].type = CalcType(shp);
if(plr!=fleet[shp].owner)
	fprintf(trns[fleet[shp].owner],"Ally did S%dB%dF\n",shp+100,amt);
fprintf(trns[plr],"S%dB%dF\n",shp+100,amt);
}
