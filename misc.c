/* Miscellaneous stuff for Celestial Empire */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/misc.c,v 1.46 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: misc.c,v $
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  13:59:23  dwagon
 * Initial RCS'd version
 * */

/* 21/5/92	Remove underscores
 * 31/5/92	Added ore transmutation
 */

#include "def.h"

#define TRMSC(x)	if(strstr(dbgstr,"MISC") || strstr(dbgstr,"misc")) x

extern char name[NUMPLAYERS+1][10];
extern FILE *trns[NUMPLAYERS+1];
extern ship fleet[NUMSHIPS];
extern planet galaxy[NUMPLANETS];
extern Player plr;
extern char *game_path;
extern int gm;
extern char *dbgstr;

/*****************************************************************************/
void Transmute(Planet num,Amount amt,Oretype from,Oretype to)
/*****************************************************************************/
/* Transmute `amt' of ore from type `from' to type `to' on planet `num' */
{
TRMSC(printf("Transmute(num:%d,amt:%d,from:%d,to:%d)\n",num,amt,from,to));

fprintf(trns[plr],"%dX%dR%dR%d\t",num+100,amt,from,to);
if(galaxy[num].owner!=plr) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}
if(galaxy[num].indleft<amt*10) {
	amt=galaxy[num].indleft/10;
	fprintf(trns[plr],"I ");
	}
if(galaxy[num].ore[from]<amt) {
	amt=galaxy[num].ore[from];
	fprintf(trns[plr],"R%d ",from);
	}
galaxy[num].ore[to]+=amt;
galaxy[num].ore[from]-=amt;
galaxy[num].indleft-=amt*10;
fprintf(trns[plr],"%dX%dR%dR%d\n",num+100,amt,from,to);
return;
}

/*****************************************************************************/
void DeployShp(Ship shp,Amount amt)
/*****************************************************************************/
/* Deploy space mines from ship */
{
Planet num;

TRMSC(printf("misc:DeployShp(shp:%d,amt:%d)\n",shp,amt));

fprintf(trns[plr],"S%dD%d\t",shp+100,amt);
if(fleet[shp].owner!=plr) {
	fprintf(stderr,"misc:DeployShp:Plr %d doesnt own ship %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

num=fleet[shp].planet;				/* The planet the ship is orbitting */
if(amt>fleet[shp].spacemines) {		/* If there are not enough spacemines */
	amt=fleet[shp].spacemines;		/* Reduce the amount to fit */
	fprintf(trns[plr],"SM ");
	}
galaxy[num].deployed+=amt;	/* Increase the # deployed spacemines */
fleet[shp].spacemines-=amt;			/* Reduce the amount in the ships hold */
fleet[shp].cargleft+=amt;			/* Increase cargo space */
fprintf(trns[plr],"S%dD%d\n",shp+100,amt);
return;
}

/*****************************************************************************/
void DeployPlnt(Planet plnt,Amount amt)
/*****************************************************************************/
/* Deploy space mines from a planet */
{
TRMSC(printf("misc:DeployPlnt(plnt:%d,amt:%d)\n",plnt,amt));

fprintf(trns[plr],"%dD%d\t",plnt+100,amt);
if(galaxy[plnt].owner!=plr) {
	fprintf(stderr,"misc:DeployShp:Plr %d doesnt own plan %d\n",plr+1,plnt+100);
	fprintf(trns[plr],"You do not own planet %d\n",plnt+100);
	return;
	}

if(amt>galaxy[plnt].spacemine) {
	fprintf(trns[plr],"SM ");
	amt=galaxy[plnt].spacemine;
	}
galaxy[plnt].deployed+=amt;
galaxy[plnt].spacemine-=amt;
fprintf(trns[plr],"%dD%d\n",plnt+100,amt);
return;
}

/*****************************************************************************/
void Undeploy(Ship shp,Amount amt)
/*****************************************************************************/
/* Undeploy spacemines and stick in cargo hold */
{
int num;
TRMSC(printf("misc:Undeploy(shp:%d,amt:%d)\n",shp,amt));

fprintf(trns[plr],"S%dR%d\t",shp+100,amt);
if(fleet[shp].owner!=plr) {
	fprintf(stderr,"misc:Undeploy:Plr %d doesnt own ship %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

if(amt>fleet[shp].tractor) {		/* Can only retrieve one per tractor unit */
	amt=fleet[shp].tractor;
	fprintf(trns[plr],"T ");
	}
if(amt>fleet[shp].cargleft) {		/* Have to be able to fit them into hold */
	amt=fleet[shp].cargleft;
	fprintf(trns[plr],"C ");
	}
num=fleet[shp].planet;
if(galaxy[num].deployed<amt)	{ /* Reduce to the number available */
	amt=galaxy[num].deployed;
	fprintf(trns[plr],"SM ");
	}
galaxy[num].deployed-=amt;
fleet[shp].cargleft-=amt;
fleet[shp].spacemines+=amt;
fprintf(trns[plr],"S%dR%d\n",shp+100,amt);
return;
}

/*****************************************************************************/
void GiftShip(Ship sbj,Player vict)
/*****************************************************************************/
/* Gift a ship to another player */
{
static int numgift[NUMPLAYERS+1];
static int flag=0;

TRMSC(printf("misc:GiftShip(sbj: %d,vict: %d)\n",sbj,vict));

if(flag==0) {
	for(flag=0;flag<NUMPLAYERS;flag++)
		numgift[flag]=0;
	flag=1;
	}
fprintf(trns[plr],"S%dG%s\t",sbj+100,name[vict]);
if(fleet[sbj].owner!=plr) {
	fprintf(stderr,"misc:GiftShip:Plr %d not owner of ship %d\n",plr+1,sbj+100);
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	return;
	}
if(numgift[plr]++>=2) {
	fprintf(trns[plr],"You cannot give away more than 2 ships in a turn\n");
	return;
	}

fleet[sbj].owner=vict;
fprintf(trns[plr],"S%dG%s\n",sbj+100,name[vict]);
fprintf(trns[vict],"Ship S%d was given to you by %s\n",sbj+100,name[plr]);
return;
}

/*****************************************************************************/
void GiftPlan(Planet sbj,Player vict)
/*****************************************************************************/
/* Gift a planet to another player */
{
static int flag=0;
static int numgift[NUMPLAYERS+1];

TRMSC(printf("misc:GiftPlan(sbj: %d,vict: %d)\n",sbj,vict));

if(flag==0) {
	for(flag=0;flag<NUMPLAYERS;flag++)
		numgift[flag]=0;
	flag=1;
	}

fprintf(trns[plr],"%dG%s\t",sbj+100,name[vict]);
if(galaxy[sbj].owner!=plr) {
	fprintf(stderr,"misc:GiftPlan:Plr %d not owner of plan %d\n",plr+1,sbj+100);
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	return;
	}

if(IsHome(sbj,plr)) {
	fprintf(trns[plr],"You cannot give away your home planet\n");
	return;
	}
if(numgift[plr]++>=3) {
	fprintf(trns[plr],"You cannot give away more than 3 planets in a turn\n");
	return;
	}

galaxy[sbj].owner=vict;
fprintf(trns[plr],"%dG%s\n",sbj+100,name[vict]);
fprintf(trns[vict],"Planet %d was given to you by %s\n",sbj+100,name[plr]);
return;
}

/*****************************************************************************/
void NamePlanet(Planet sbj,char *pname)
/*****************************************************************************/
/* Name a planet */
{
TRMSC(printf("misc:NamePlanet(sbj: %d,pname: %s)\n",sbj,pname));

fprintf(trns[plr],"%d=%s\t",sbj+100,pname);
if(galaxy[sbj].owner!=plr) {
	fprintf(stderr,"misc:NamePlanet:Plr %d not owner of plan %d\n",plr+1,sbj+100);
	fprintf(trns[plr],"You do not own planet %d\n",sbj+100);
	return;
	}
strcpy(galaxy[sbj].name,pname);
fprintf(trns[plr],"%d=%s\n",sbj+100,pname);
return;
}

/*****************************************************************************/
void NameShip(Ship sbj,char *pname)
/*****************************************************************************/
/* Name a ship */
{ 
TRMSC(printf("misc:NameShip(sbj: %d,pname: %s)\n",sbj,pname));

fprintf(trns[plr],"S%d=%s\t",sbj+100,pname);
if(fleet[sbj].owner!=plr) {
	fprintf(stderr,"misc:NameShip:Plr %d not owner of ship %d\n",plr+1,sbj+100);
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	return;
	}
strcpy(fleet[sbj].name,pname);
fprintf(trns[plr],"S%d=%s\n",sbj+100,pname);
return;
}

/*****************************************************************************/
void HelpHelp(const char *target)
/*****************************************************************************/
/* Earth is under attack, ask for help */
{
FILE *motd;
char str[80];

TRMSC(printf("broadcast\n"));
sprintf(str,"%s%d/motd",game_path,gm);
if((motd=fopen(str,"a"))==NULL) {
	fprintf(stderr,"broadcast:Could not open file %s for appending\n",str);
	return;
	}
fprintf(motd,"********** HELP! HELP! **********\n");
fprintf(motd,"Earth %s under attack by %s\n",target,name[plr]);
fprintf(motd,"Any help that you can render will be appreciated\n");
fclose(motd);
return;
}

