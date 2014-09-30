/* ATK.C     for Celestial Empire  by Zer Dwagon   */
/* (c) 1992 Dougal Scott */
/* This file has in it all the procedures involving attacking */
/* $Header: /nelstaff/edp/dwagon/rfs/RCS/atk.c,v 1.60 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: atk.c,v $
 * Revision 1.60  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.59  1993/07/06  06:58:14  dwagon
 * Added definition for NEUTPLR instead of 9 for future changes
 *
 * Revision 1.58  1993/06/16  08:40:44  dwagon
 * Added EarthRetaliate() function that gets
 * NEUTRAL planets attack enemy ships that are above them
 *
 * Revision 1.57  1993/05/08  04:52:07  dwagon
 * Implemented scaled PDU attacks.
 *
 * Revision 1.56  1993/04/08  07:53:29  dwagon
 * Fixed problem in PduShp where a planet attacking a neutral ship would
 * cause the program to dump core. Occurs when hulls are tractored through
 * planets
 *
 * Revision 1.55  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.54  1993/02/05  03:33:54  dwagon
 * Fixed problem with ShpOre. If the planet owner was neutral, it complained
 * about owner friendly. Put check in for neutral ownership. Reported by
 * Markus Baumeister <baumeist@picasso.informatik.rwth-aachen.de> Game #13.
 *
 * Revision 1.53  1992/12/14  02:55:40  dwagon
 * Don't report PDU attacks if there are no PDUs attacking :)
 *
 * Revision 1.52  1992/12/04  04:25:02  dwagon
 * Fixed another segfault casued by the same thing. I thought I had fixed it
 * last time. This was in SpcminShp.
 *
 * Revision 1.51  1992/12/03  05:44:42  dwagon
 * Fixed bug where attacks on a neutral planet caused a segv as it tried to
 * inform player 9 of the attack. Checks in place only inform player if the
 * player number is not 9.
 *
 * Revision 1.50  1992/11/29  01:52:10  dwagon
 * Players are informed of all attacks that their spacemines and PDUs do
 * including attacks against ships that are flying through their systems.
 *
 * Revision 1.49  1992/11/19  03:33:12  dwagon
 * Fixed bug with previous code modification. Forgot to change the name of
 * a variable. :(
 *
 * Revision 1.48  1992/11/19  03:30:13  dwagon
 * Fixed ShpMin code to simplify it, and also fix a bug where if there was no
 * mines and the number of shots was the repoted number of ore destroyed.
 *
 * Revision 1.47  1992/11/09  03:50:39  dwagon
 * Implemented ore attack command ShpOre()
 *
 * Revision 1.46  1992/10/20  02:39:47  dwagon
 * PDUs no longer get two attacks: 1 for automatic defense, the other for
 * manual defense.
 *
 * Revision 1.45  1992/10/16  04:54:10  dwagon
 * Industry destroyed this turn will no longer be able to produce
 * anything that turn
 *
 * Revision 1.44  1992/09/16  13:50:38  dwagon
 * Initial RCS'd version
 *
 * Revision 1.1  1992/09/16  13:45:54  dwagon
 * Initial revision
 * */

/* 26/4/92	Fixed bug where if you had no shots the attack was still processed
 *			in full, instead of being ignored
 * 2/5/92	Added ALLY status
 * 18/5/92	Added game details structure
 * 21/5/92	Changed names to get rid of `_'s
 *			Moved shots to librfs
 * 14/9/92	Checked for attacking neutral players PDUs in ShpPdu.
 */

#include "def.h"

#define TRATK(x)	if(strstr(dbgstr,"ATK") || strstr(dbgstr,"atk")) x

extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern char name[NUMPLAYERS+1][10];
extern Player plr;
extern int shipvict[NUMSHIPS][NUMPLAYERS+1][2];
extern char *dbgstr;
extern Ship shiptr;

/*****************************************************************************/
void EngageTractor(Ship shp,Ship targ)
/*****************************************************************************/
/* Engage tractor beams on target ship */
{
Ship tmp;
int weight;

TRATK(printf("atk:EngageTractor(shp:%d,targ:%d)\n",shp,targ));

fprintf(trns[plr],"S%dES%d\t",shp+100,targ+100);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

if(fleet[shp].planet!=fleet[targ].planet) {
	fprintf(trns[plr],"Target ship not around same planet\n");
	fprintf(stderr,"Ship %d not near ship %d\n",shp+100,targ+100);
	return;
	}

weight=CalcWeight(targ);

if(weight>fleet[shp].tractor) {	/* Target too heavy */
	fprintf(trns[plr],"Target ship %d is too heavy to tow\n",targ+100);
	TRATK(fprintf(stderr,"Target ship %d too heavy to tow by ship %d\n",targ+100,shp+100));
	return;
	}

if(fleet[shp].engage>0) {	/* Engaging ship is itself engaged */
	fprintf(trns[plr],"Ship %d is itself engaged\n",shp+100);
	TRATK(fprintf(stderr,"Ship %d is itself engaged\n",shp+100));
	return;
	}

if(fleet[shp].engage<0) {	/* Ship is already engaging */
	fprintf(trns[plr],"Ship %d is already towing another ship\n",shp+100);
	TRATK(fprintf(stderr,"Ship %d is already towing another ship\n",shp+100));
	return;	
	}

if(fleet[targ].engage>0) {	/* Engaged ship engaged by another */
	tmp=fleet[shp].engage-1;
	if(fleet[shp].tractor<fleet[tmp].tractor) {
		fprintf(trns[plr],"Opposing ship out-tractor'd your ship\n");
		TRATK(fprintf(stderr,"Opposing ship out-tractor'd ship\n"));
		return;
		}
	else {		/* Change engager */
		fleet[tmp].engage=0;	/* Revoke current engager */
		fprintf(trns[fleet[tmp].owner],"Ship %d wrested from ship %d grasp\n",targ+100,tmp+100);
		}
	}
fleet[targ].engage=shp+1;
fleet[shp].engage= -targ-1;
fprintf(trns[plr],"S%dES%d\n",shp+100,targ+100);
return;
}

/*****************************************************************************/
void ShpShp(Ship shp,Ship targ,Units amnt)
/*****************************************************************************/
/* Ship attack ship */
{
Number shts;
int rex=0;

TRATK(printf("atk:ShpShp(shp:%d,targ:%d,amnt:%d)\n",shp,targ,amnt));
if(amnt<0)
	fprintf(trns[plr],"S%dAS%d\t",shp+100,targ+100);
else
	fprintf(trns[plr],"S%dA%dS%d\t",shp+100,amnt,targ+100);

if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Player %d does not own ship %d\n",plr,shp+100);
	return;
	}

if(fleet[shp].planet!=fleet[targ].planet) {
	fprintf(trns[plr],"Your ship %d is not near target ship %d\n",shp+100,targ+100);
	return;
	}

if(shp==targ) {
	fprintf(trns[plr],"Your ship refuses to fire upon itself\n");
	return;
	}

if(alliance[plr][fleet[targ].owner]>=FRIEND) {
	fprintf(trns[plr],"Cannot attack a friendly ship\n");
	return;
	}

if(fleet[shp].figleft<amnt) {
	fprintf(trns[plr],"F ");
	rex=1;
	amnt=fleet[shp].figleft;
	}

shts=Shots(shp,(amnt==-1?fleet[shp].figleft:amnt));
if(shts==0) {
	fprintf(trns[plr],"Your ship has no shots\n");
	return;
	}

fleet[shp].moved=1;
if(amnt<0)
	fleet[shp].figleft=0;
else
	fleet[shp].figleft-=amnt;
fleet[targ].hits+=shts;
shipvict[targ][plr][SHIP]+=shts;
if(amnt<0)
	fprintf(trns[plr],"%cS%dAS%d\n",(rex==1)?'\0':'\t',shp+100,targ+100);
else
	fprintf(trns[plr],"%cS%dA%dS%d\n",(rex==1)?'\0':'\t',shp+100,amnt,targ+100);
}

/*****************************************************************************/
void PlanetAttack(Planet sbj,Ship vict,Units amnt)
/*****************************************************************************/
/* Get the planet to attack the ship */
{
TRATK(printf("atk:PlanetAttack(sbj:%d, vict:%d, amnt:%d)\n",sbj,vict,amnt));

if(amnt<0)
	fprintf(trns[plr],"%dAS%d\t",sbj+100,vict+100);
else
	fprintf(trns[plr],"%dA%dS%d\t",sbj+100,amnt,vict+100);

if(galaxy[sbj].owner!=plr) {
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	fprintf(stderr,"Player %d does not own planet %d\n",plr,sbj+100);
	return;
	}

if(sbj!=fleet[vict].planet) {
	fprintf(trns[plr],"Target ship is not above planet %d\n",sbj+100);
	return;
	}

if(alliance[galaxy[sbj].owner][fleet[vict].owner]!=ENEMY) {
	fprintf(trns[plr],"Player %s is not a declared enemy\n",name[vict]);
	return;
	}

if(galaxy[sbj].pduleft<amnt) {
	fprintf(trns[plr],"D ");
	amnt=galaxy[sbj].pduleft;
	}

PduShp(sbj,vict,(amnt==-1)?galaxy[sbj].pduleft:amnt);
shipvict[vict][plr][PDU]+=(amnt==-1)?galaxy[sbj].pduleft:amnt;

if(amnt<0) 
	fprintf(trns[plr],"%dAS%d\n",sbj+100,vict+100);
else
	fprintf(trns[plr],"%dA%dS%d\n",sbj+100,amnt,vict+100);
return;
}

/*****************************************************************************/
void PlanetSpcmin(Planet sbj,Amount amnt)
/*****************************************************************************/
/* Get the planet to attack the spacemines around it */
{
int tmp=0;

TRATK(printf("atk:PlanetSpcmin(sbj:%d,amnt:%d)\n",sbj,amnt));

if(amnt<0)
	fprintf(trns[plr],"%dASM\t",sbj+100);
else
	fprintf(trns[plr],"%dA%dSM\t",sbj+100,amnt);
if(galaxy[sbj].owner!=plr) {
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	fprintf(stderr,"Player %d does not own planet %d\n",plr,sbj+100);
	return;
	}

if(amnt>galaxy[sbj].pduleft) {
	amnt=galaxy[sbj].pduleft;
	fprintf(trns[plr],"D ");
	}

if(amnt<0) {
	tmp=MIN(galaxy[sbj].pduleft,galaxy[sbj].deployed);
	galaxy[sbj].deployed-=galaxy[sbj].pduleft;
	galaxy[sbj].pduleft=0;
	}
else {
	tmp=MIN(amnt,galaxy[sbj].deployed);
	galaxy[sbj].deployed-=amnt;
	galaxy[sbj].pduleft-=amnt;
	}
if(galaxy[sbj].deployed<0)
	galaxy[sbj].deployed=0;

if(amnt<0)
	fprintf(trns[plr],"%dASM\t",sbj+100);
else
	fprintf(trns[plr],"%dA%dSM\t",sbj+100,amnt);
fprintf(trns[galaxy[sbj].owner],"%d spacemines destroyed\n",tmp);
}

/*****************************************************************************/
void PduShp(Planet num,Ship shp,Amount amnt)
/*****************************************************************************/
/* PDU's vs ship */
{
TRATK(printf("atk:PduShp(num: %d,shp: %d,amnt:%d)\n",num,shp,amnt));

if(galaxy[num].owner==fleet[shp].owner) {
	TRATK(printf("Planet owner is ship owner\n"));
	fprintf(stderr,"Cannot attack own ship\n");
	return;
	}
	
if(alliance[galaxy[num].owner][fleet[shp].owner]>=FRIEND) {
	TRATK(printf("Planet owner is an ally\n"));
	if(galaxy[num].owner!=NEUTPLR)
		fprintf(trns[galaxy[num].owner],"Cannot attack friends ship\n");
	return;
	}

fleet[shp].pduhits+=Pdus(amnt);
galaxy[num].pduleft-=amnt;
if(amnt>0) {
	fprintf(trns[galaxy[num].owner],"%d PDUs on planet %d attacked ship %d\n",amnt,num+100,shp+100);
	fprintf(trns[fleet[shp].owner],"%d PDUs on planet %d attacked ship %d\n",amnt,num+100,shp+100);
	}
}

/*****************************************************************************/
void ShpPdu(Ship shp,Units amnt)
/*****************************************************************************/
/* Ship attack PDU's */
{
int sh,tmp=0;
Planet num=fleet[shp].planet;

TRATK(printf("atk:ShpPdu(shp:%d,amnt:%d)\n",shp,amnt));

if(amnt<0)
	fprintf(trns[plr],"S%dAD\t",shp+100);
else
	fprintf(trns[plr],"S%dA%dD\t",shp+100,amnt);

if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

if(galaxy[num].owner!=NEUTPLR && alliance[plr][galaxy[num].owner]>=FRIEND) {
	TRATK(printf("Planet owner is an ally\n"));
	fprintf(trns[plr],"Planet owner is friendly\n");
	return;
	}

if(fleet[shp].figleft<amnt) {
	fprintf(trns[plr],"F ");
	amnt=fleet[shp].figleft;
	}

sh=Shots(shp,(amnt==-1?fleet[shp].figleft:amnt));
if(sh==0) {
	fprintf(trns[plr],"Your ship has no shots\n");
	return;
	}

if(amnt<0)
	fleet[shp].figleft=0;
else
	fleet[shp].figleft-=amnt;

PduShp(num,shp,galaxy[num].pdu);

fprintf(trns[galaxy[num].owner],"S%d attacked planet %d destroying %d PDUs\n",shp+100,num+100,MIN(sh/3,galaxy[num].pdu));
tmp=MIN(sh/3,galaxy[num].pdu);
galaxy[num].pdu-=sh/3;
if(galaxy[num].pdu<0)
	galaxy[num].pdu=0;

if(amnt<0)
	fprintf(trns[plr],"S%dAD\t",shp+100);
else
	fprintf(trns[plr],"S%dA%dD\t",shp+100,amnt);
if(IsEarth(num)) {
	alliance[plr][plr]=ENEMY;
	HelpHelp("defense");
	}
fleet[shp].moved=1;
fprintf(trns[plr],"destroyed %d PDUs\n",tmp);
}

/*****************************************************************************/
void ShpSpcmin(Ship shp,Units amnt)
/*****************************************************************************/
/* Ship attack space mines */
{
int sht,tmp=0;
Planet num;

TRATK(printf("atk:ShpSpcmin(shp:%d,amnt:%d)\n",shp,amnt));

if(amnt<0)
	fprintf(trns[plr],"S%dASM\t",shp+100);
else
	fprintf(trns[plr],"S%dA%dSM\t",shp+100,amnt);

if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

num=fleet[shp].planet;
if(galaxy[num].deployed==0) {
	fprintf(trns[plr],"There are no spacemines around planet %d\n",num+100);
	return;
	}
if(fleet[shp].figleft<amnt) {
	fprintf(trns[plr],"F ");
	amnt=fleet[shp].figleft;
	}

sht=Shots(shp,(amnt==-1?fleet[shp].figleft:amnt));
if(sht==0) {
	fprintf(trns[plr],"Your ship has no shots\n");
	return;
	}
fprintf(trns[galaxy[num].owner],"S%d attacked planet %d and destroyed %d spacemines\n",shp+100,num+100,MIN(galaxy[num].deployed,sht));
tmp=MIN(galaxy[num].deployed,sht);
galaxy[num].deployed-=sht;
if(galaxy[num].deployed<0)
	galaxy[num].deployed=0;
if(amnt<0) {
	fprintf(trns[plr],"S%dASM\t",shp+100);
	fleet[shp].figleft=0;
	}
else {
	fprintf(trns[plr],"S%dA%dSM\t",shp+100,amnt);
	fleet[shp].figleft-=amnt;
	}
fleet[shp].moved=1;
fprintf(trns[plr],"destroying %d spacemines\n",tmp);
return;
}

/*****************************************************************************/
void SpcminShp(Ship shp)
/*****************************************************************************/
/* Spacemine attack the ship */
{
Planet num=fleet[shp].planet;
int depl=galaxy[num].deployed;

TRATK(printf("atk:SpcminShp(shp:%d)\n",shp));

fleet[shp].hits+=(int)(galaxy[num].deployed/2);
galaxy[num].deployed=(int)((float)galaxy[num].deployed*0.9);
if(galaxy[num].deployed==depl)
	galaxy[num].deployed--;
fprintf(trns[galaxy[num].owner],"Spacemines on planet %d attacked ship %d\n",num+100,shp+100);
fprintf(trns[fleet[shp].owner],"Spacemines on planet %d attacked ship %d\n",num+100,shp+100);
return;
}

/*****************************************************************************/
void ShpOre(Oretype typ,Ship shp,Units amnt)
/*****************************************************************************/
/* Ship attack ore type */
{
int sht, oredmg=0;
Planet num=fleet[shp].planet;

TRATK(printf("atk:ShpOre(typ:%d,shp:%d.amnt:%d)\n",typ,shp,amnt));

if(amnt==-1)
	fprintf(trns[plr],"S%dAR%d\t",shp+100,typ);
else
	fprintf(trns[plr],"S%dA%dR%d\t",shp+100,amnt,typ);

if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

if(galaxy[num].owner!=NEUTPLR && alliance[plr][galaxy[num].owner]>=FRIEND) {
	fprintf(trns[plr],"Planet owner is friendly\n");
	fprintf(stderr,"S%dAR%d Planet %d owner is friendly to %d\n",shp+100,typ,num,plr);
	return;
	}

if(fleet[shp].figleft<amnt) {
	fprintf(trns[plr],"F ");
	amnt=fleet[shp].figleft;
	}

sht=Shots(shp,(amnt==-1?fleet[shp].figleft:amnt));
if(sht==0) {
	fprintf(trns[plr],"Your ship has no shots\n");
	return;
	}
PduShp(num,shp,galaxy[num].pdu);
if(amnt<0)
	fleet[shp].figleft=0;
else
	fleet[shp].figleft-=amnt;
oredmg=MIN(sht,galaxy[num].ore[typ]);
galaxy[num].ore[typ]-= oredmg;
fprintf(trns[galaxy[num].owner],"S%d attacked planet %d destroying %d ore of type %d\n",shp+100,num+100,oredmg,typ);
if(amnt==-1)
	fprintf(trns[plr],"S%dAR%d\t",shp+100,typ);
else
	fprintf(trns[plr],"S%dA%dR%d\t",shp+100,amnt,typ);
if(IsEarth(num)) {
	alliance[plr][plr]=ENEMY;
	HelpHelp("ore");
	}
fleet[shp].moved=1;
fprintf(trns[plr],"destroyed %d ore\n",oredmg);
}

/*****************************************************************************/
void ShpMin(Oretype typ,Ship shp,Units amnt)
/*****************************************************************************/
/* Ship attack mine type */
{
int sht,mindmg=0,oredmg=0;
Planet num=fleet[shp].planet;

TRATK(printf("atk:ShpMin(typ:%d,shp:%d.amnt:%d)\n",typ,shp,amnt));

if(amnt==-1)
	fprintf(trns[plr],"S%dAM%d\t",shp+100,typ);
else
	fprintf(trns[plr],"S%dA%dM%d\t",shp+100,amnt,typ);

if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

if(alliance[plr][galaxy[num].owner]>=FRIEND) {
	fprintf(trns[plr],"Planet owner is friendly\n");
	fprintf(stderr,"S%dAM%d Planet %d owner is friendly to %d\n",shp+100,typ,num,plr);
	return;
	}

if(fleet[shp].figleft<amnt) {
	fprintf(trns[plr],"F ");
	amnt=fleet[shp].figleft;
	}

sht=Shots(shp,(amnt==-1?fleet[shp].figleft:amnt));
if(sht==0) {
	fprintf(trns[plr],"Your ship has no shots\n");
	return;
	}
PduShp(num,shp,galaxy[num].pdu);
mindmg=MIN(sht/10,galaxy[num].mine[typ]);
sht-=mindmg*10;
oredmg=MIN(sht,galaxy[num].ore[typ]);
galaxy[num].mine[typ]-=mindmg;
galaxy[num].ore[typ]-=oredmg;
fprintf(trns[galaxy[num].owner],"S%d attacked planet %d destroying %d mines and %d ore of type %d\n",shp+100,num+100,mindmg,oredmg,typ);

if(amnt==-1)
	fprintf(trns[plr],"S%dAM%d\t",shp+100,typ);
else
	fprintf(trns[plr],"S%dA%dM%d\t",shp+100,amnt,typ);
if(IsEarth(num)) {
	alliance[plr][plr]=ENEMY;
	HelpHelp("mines");
	}
fleet[shp].moved=1;
fprintf(trns[plr],"destroyed %d mines and %d ore\n",mindmg,oredmg);
}

/*****************************************************************************/
void ShpInd(Ship shp,Units amnt)
/*****************************************************************************/
/* Ship attack industry */
{
int sht,tmp;
Planet num=fleet[shp].planet;

TRATK(printf("atk:ShpInd(shp:%d,amnt:%d)\n",shp,amnt));

if(amnt==-1)
	fprintf(trns[plr],"S%dAI\t",shp+100);
else
	fprintf(trns[plr],"S%dA%dI\t",shp+100,amnt);

if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

if(alliance[plr][galaxy[num].owner]>=FRIEND) {
	fprintf(trns[plr],"Planet owner is friendly\n");
	fprintf(stderr,"S%dAI Planet %d owner is friendly to %d\n",shp+100,num,plr);
	return;
	}

if(fleet[shp].figleft<amnt) {
	fprintf(trns[plr],"F ");
	amnt=fleet[shp].figleft;
	}

sht=Shots(shp,(amnt==-1?fleet[shp].figleft:amnt));
if(sht==0) {
	fprintf(trns[plr],"Your ship has no shots\n");
	return;
	}
PduShp(num,shp,galaxy[num].pdu);
tmp=sht/10;
if(galaxy[num].ind<tmp)
	tmp=galaxy[num].ind;
galaxy[num].ind-=tmp;
galaxy[num].indleft-=tmp;
galaxy[num].income-=tmp*5;
fprintf(trns[galaxy[num].owner],"S%d fired upon planet %d destroying %d industry\n",shp+100,num+100,tmp);
if(amnt<0)
	fleet[shp].figleft=0;
else
	fleet[shp].figleft-=amnt;
if(amnt==-1)
	fprintf(trns[plr],"S%dAI\t",shp+100);
else
	fprintf(trns[plr],"S%dA%dI\t",shp+100,amnt);
fleet[shp].moved=1;
if(IsEarth(num)) {
	alliance[plr][plr]=ENEMY;
	HelpHelp("industry");
	}
fprintf(trns[plr],"destroyed %d industry\n",tmp);
}

/*****************************************************************************/
void EarthRetaliate(void)
/*****************************************************************************/
/* Neutral planets attack all hostile ships */
{
Ship shp;

TRATK(printf("atk:EarthRetaliate()\n"));

for(shp=0;shp<shiptr;shp++) {
	if(galaxy[fleet[shp].planet].owner==NEUTPLR) {
		if(alliance[fleet[shp].owner][fleet[shp].owner]==ENEMY) {
			PduShp(fleet[shp].planet,shp,galaxy[fleet[shp].planet].pduleft);
			}
		}
	}
return;
}
