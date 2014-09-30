/* LOAD.C Module of Celestl Empire that deals with all the loading of cargo */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/load.c,v 1.46 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: load.c,v $
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  13:57:54  dwagon
 * Initial RCS'd version
 * */

/* 2/5/92	Added ALLY status, allies can load from allies planets 
 * 3/5/92	Fixed LoadDef so if home defense is less than orig you don't load
 *			negative defense.
 * 18/5/92	Added game details structure
 * 21/5/92	Removed underscores
 * 5/8/92	Added priority loading
 * 4/9/92	Fixed bug with loading mines. The check for home was the wrong way
 */

#include "def.h"

#define TRLOAD(x)	if(strstr(dbgstr,"LOAD") || strstr(dbgstr,"load")) x

extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern Player plr;
extern game gamedet;
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern char *dbgstr;

/*****************************************************************************/
void LoadSpcmine(Ship shp,Amount amt)
/*****************************************************************************/
/* Load a ship with space-mines from a planetary stockpile */
{
Planet num;

TRLOAD(printf("load:LoadSpcmine(shp:%d,amt:%d)\n",shp,amt));

fprintf(trns[plr],"S%dL%dS\t",shp+100,amt);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadSpcmine:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}

num=fleet[shp].planet;
if(galaxy[num].owner!=plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadSpcmine:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}
if(fleet[shp].cargleft<amt) {
	amt=fleet[shp].cargleft;
	fprintf(trns[plr],"C ");
	}
if(galaxy[num].spacemine<amt) {
	amt=galaxy[num].spacemine;
	fprintf(trns[plr],"SM ");
	}
galaxy[num].spacemine-=amt;
fleet[shp].spacemines+=amt;
fleet[shp].cargleft-=amt;
fprintf(trns[plr],"S%dL%dS\n",shp+100,amt);
return;
}

/*****************************************************************************/
void LoadOre(Oretype typ,Ship shp,Amount amt)
/*****************************************************************************/
/* Load ore of a specific type        */
{
Planet num=fleet[shp].planet;

TRLOAD(printf("LoadOre(typ:%d,shp:%d,amt:%d)\n",typ,shp,amt));

fprintf(trns[plr],"S%dL%dR%d\t",shp+100,amt,typ);
if(fleet[shp].owner != plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadOre:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}
if(galaxy[num].owner!=plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadOre:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}	
if(typ<0 || typ>9) {
	fprintf(trns[plr],"Ore type %d not valid\n",typ);
	return;
	}
if(galaxy[num].ore[typ]<amt) {
	amt = galaxy[num].ore[typ];
	fprintf(trns[plr],"R%d ",typ);
	}
if(fleet[shp].cargleft<amt) {
	amt=fleet[shp].cargleft;
	fprintf(trns[plr],"C ");
	}
fleet[shp].ore[typ] += amt;
galaxy[num].ore[typ] -= amt;
fleet[shp].cargleft -= amt;
fprintf(trns[plr],"S%dL%dR%d\n",shp+100,amt,typ);
}

/*****************************************************************************/
void LoadDef(Ship shp,Amount amt)
/*****************************************************************************/
/* Load PDU's */
{
Planet num=fleet[shp].planet;

TRLOAD(printf("Load %d pdus onto ship %d\n",amt,shp));

fprintf(trns[plr],"S%dL%dD\t",shp+100,amt);
if(fleet[shp].owner !=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadDef:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}
if(galaxy[num].owner != plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadDef:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}

if(galaxy[num].pdu<amt) {
	amt = galaxy[num].pdu;
	fprintf(trns[plr],"PDU ");
	}
if(IsHome(num,plr) || (alliance[galaxy[num].owner][plr]==ALLY && IsHome(num,galaxy[num].owner)))
	if(galaxy[num].pdu-gamedet.home.pdu<amt) {
		amt=galaxy[num].pdu-gamedet.home.pdu;
		if(amt<0)
			amt=0;
		fprintf(trns[plr],"POP ");
		}
if(fleet[shp].cargleft<amt*2) {
	amt=fleet[shp].cargleft/2;
	fprintf(trns[plr],"C ");
	}
fleet[shp].pdu+=amt;
galaxy[num].pdu-=amt;
fleet[shp].cargleft-=amt*2;
fprintf(trns[plr],"S%dL%dD\n",shp+100,amt);
}

/*****************************************************************************/
void LoadMin(Oretype typ,Ship shp,Amount amt)
/*****************************************************************************/
/* Load mine  */
{
Planet num = fleet[shp].planet;

TRLOAD(printf("Loading %d mine type %d onto ship %d\n",amt,typ,shp));

fprintf(trns[plr],"S%dL%dM%d\t",shp+100,amt,typ);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadMin:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}
if(galaxy[num].owner!=plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadMin:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}
if(!IsHome(num,plr)) {
	fprintf(trns[plr],"Planet %d not home planet\n",num+100);
	return;
	}
if(galaxy[num].mine[typ]<amt) {
	amt=galaxy[num].mine[typ];
	fprintf(trns[plr],"M ");
	}
if(fleet[shp].cargleft<amt*20) {
	amt=fleet[shp].cargleft/20;
	fprintf(trns[plr],"C ");
	}
galaxy[num].mine[typ]-=amt;
fleet[shp].mines+=amt;
fleet[shp].cargleft-=amt*20;
galaxy[num].income-=amt;
fprintf(trns[plr],"S%dL%dM%d\n",shp+100,amt,typ);
}

/*****************************************************************************/
void LoadInd(Ship shp,Amount amt)
/*****************************************************************************/
/* Load industry from ship */
{
Planet num=fleet[shp].planet;

TRLOAD(printf("Loading %d industry onto ship %d\n",amt,shp));

fprintf(trns[plr],"S%dL%dI\t",shp+100,amt);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadInd:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}
if(galaxy[num].owner!=plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadInd:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}
if(!IsHome(num,plr)) {
	fprintf(trns[plr],"Planet %d not home planet\n",num+100);
	return;
	}
if(galaxy[num].ind-gamedet.home.ind<amt) {
	amt=galaxy[num].ind-gamedet.home.ind;
	fprintf(trns[plr],"POP ");
	}
if(fleet[shp].cargleft<amt*10) {
	amt=fleet[shp].cargleft/10;
	fprintf(trns[plr],"C ");
	}
galaxy[num].ind-=amt;
fleet[shp].ind+=amt;
fleet[shp].cargleft-=amt*10;
galaxy[num].indleft-=amt;
galaxy[num].income-=amt*5;
fprintf(trns[plr],"S%dL%dI\n",shp+100,amt);
}

/*****************************************************************************/
void LoadAll(Ship shp)
/*****************************************************************************/
/* Load all ores except 0 onto ship */
{
Planet num=fleet[shp].planet;
Oretype count;
Amount amt;
Units cglf=fleet[shp].cargleft;

TRLOAD(printf("Loading all ore onto ship %d\n",shp));

fprintf(trns[plr],"S%dL\t",shp+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadAll:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}
if(galaxy[num].owner!=plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadAll:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}

for (count=1;count<10;count++) {
	amt=galaxy[num].ore[count];
	if(cglf<amt) {
		amt=cglf;
		}
	cglf-=amt;
	fleet[shp].ore[count]+=amt;
	galaxy[num].ore[count]-=amt;
	}
fleet[shp].cargleft=cglf;
fprintf(trns[plr],"S%dL\n",shp+100);
}

/*****************************************************************************/
void LoadPri(Ship shp,int pri[13])
/*****************************************************************************/
/* Load all ores with priority list */
{
Planet num=fleet[shp].planet;
Oretype count;
Amount amt;
Units cglf=fleet[shp].cargleft;
char prilist[13];
int pricnt=0;

TRLOAD(printf("Loading priority ore onto ship %d\n",shp));

fprintf(trns[plr],"S%dL",shp+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	fprintf(stderr,"LoadAll:Plr %d does not own ship %d\n",plr,shp+100);
	return;
	}
if(galaxy[num].owner!=plr && alliance[galaxy[num].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	fprintf(stderr,"LoadAll:Plr %d does not own planet %d\n",plr,num+100);
	return;
	}

fprintf(trns[plr],"(");
for(count=0;count<13 && pri[count]!=-1;count++) {
	switch(pri[count]) {
		case DEFENSE:
			amt=galaxy[num].pdu;
			if(cglf<amt*2)
				amt=cglf/2;
			cglf-=amt*2;
			fleet[shp].pdu+=amt;
			galaxy[num].pdu-=amt;
			fprintf(trns[plr],"D");
			prilist[pricnt++]='D';
			break;
		case SPACEMINE:
			amt=galaxy[num].spacemine;
			if(cglf<amt)
				amt=cglf;
			cglf-=amt;
			fleet[shp].spacemines+=amt;
			galaxy[num].spacemine-=amt;
			fprintf(trns[plr],"S");
			prilist[pricnt++]='S';
			break;
		default:
			amt=galaxy[num].ore[pri[count]];
			if(cglf<amt)
				amt=cglf;
			cglf-=amt;
			fleet[shp].ore[pri[count]]+=amt;
			galaxy[num].ore[pri[count]]-=amt;
			fprintf(trns[plr],"%d",pri[count]);
			prilist[pricnt++]=pri[count]+'0';
			break;
		}
	}
fprintf(trns[plr],")\t");

for(count=1;count<10;count++) {
	amt=galaxy[num].ore[count];
	if(cglf<amt)
		amt=cglf;
	cglf-=amt;
	fleet[shp].ore[count]+=amt;
	galaxy[num].ore[count]-=amt;
	}
fleet[shp].cargleft=cglf;
fprintf(trns[plr],"S%dL\n",shp+100);
}
