/* TEND.C Module of Celestl Empire that deals with all the tending of cargo */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/tend.c,v 1.46 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: tend.c,v $
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  14:00:57  dwagon
 * Initial RCS'd version
 * */

/* 7/5/92 	File created 
 * 21/5/92	Removed underscores
 */

#include "def.h"

#define TRTEND(x)	if(strstr(dbgstr,"TEND") || strstr(dbgstr,"tend")) x

extern FILE *trns[NUMPLAYERS+1];
extern ship fleet[NUMSHIPS];
extern Player plr;
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern char *dbgstr;

/*****************************************************************************/
void TendSpcmine(Ship sbj,Ship vict,Amount amt)
/*****************************************************************************/
/* Transfer spacemines from one ship to another */
{
TRTEND(printf("TendSpcmine(sbj:%d,vict:%d,amt:%d)\n",sbj,vict,amt));

fprintf(trns[plr],"S%dT%dS%dS\t",sbj+100,amt,vict+100);
if(fleet[sbj].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,sbj+100);
	return;
	}

if(fleet[vict].owner!=plr && alliance[fleet[vict].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own ship %d\n",vict+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,vict+100);
	return;
	}

if(fleet[vict].planet!=fleet[sbj].planet) {
	fprintf(trns[plr],"Ships are not around same planet\n");
	return;
	}
if(fleet[sbj].spacemines<amt) {
	amt=fleet[sbj].spacemines;
	fprintf(trns[plr],"SM ");
	}
if(fleet[vict].cargleft<amt) {
	amt=fleet[vict].cargleft;
	fprintf(trns[plr],"C ");
	}

fleet[vict].spacemines+=amt;
fleet[vict].cargleft-=amt;
fleet[sbj].spacemines-=amt;
fleet[sbj].cargleft+=amt;
fprintf(trns[plr],"S%dT%dS%dS\n",sbj+100,amt,vict+100);
return;
}

/*****************************************************************************/
void TendOre(Oretype typ,Ship sbj,Ship vict,Amount amt)
/*****************************************************************************/
/* Transfer ore of a specific type from one ship to another */
{
TRTEND(printf("TendOre(typ:%d,sbj:%d,vict:%d,amt:%d\n",typ,sbj,vict,amt));

fprintf(trns[plr],"S%dT%dS%dR%d\t",sbj+100,amt,vict+100,typ);
if(fleet[sbj].owner != plr) {
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,sbj+100);
	return;
	}
if(fleet[vict].owner!=plr && alliance[fleet[vict].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own ship %d\n",vict+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,vict+100);
	return;
	}	
if(fleet[vict].planet!=fleet[sbj].planet) {
	fprintf(trns[plr],"Ships are not around same planet\n");
	return;
	}
if(typ<0 || typ>9) {
	fprintf(trns[plr],"Ore type %d not valid\n",typ);
	return;
	}
if(fleet[sbj].ore[typ]<amt) {
	amt = fleet[sbj].ore[typ];
	fprintf(trns[plr],"R%d ",typ);
	}
if(fleet[vict].cargleft<amt) {
	amt=fleet[vict].cargleft;
	fprintf(trns[plr],"C ");
	}
fleet[vict].ore[typ] += amt;
fleet[sbj].ore[typ] -= amt;
fleet[vict].cargleft -= amt;
fleet[sbj].cargleft += amt;
fprintf(trns[plr],"S%dT%dS%dR%d\n",sbj+100,amt,vict+100,typ);
}

/*****************************************************************************/
void TendDef(Ship sbj,Ship vict,Amount amt)
/*****************************************************************************/
/* Transfer PDUs from one ship to another */
{
TRTEND(printf("TendDef(sbj:%d,vict:%d,amt:%d)\n",sbj,vict,amt));

fprintf(trns[plr],"S%dT%dS%dD\t",sbj+100,amt,vict+100);
if(fleet[sbj].owner !=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,sbj+100);
	return;
	}
if(fleet[vict].owner != plr && alliance[fleet[vict].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own ship %d\n",vict+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,vict+100);
	return;
	}
if(fleet[vict].planet!=fleet[sbj].planet) {
	fprintf(trns[plr],"Ships are not around same planet\n");
	return;
	}

if(fleet[sbj].pdu<amt) {
	amt = fleet[sbj].pdu;
	fprintf(trns[plr],"PDU ");
	}
if(fleet[vict].cargleft<amt*2) {
	amt=fleet[vict].cargleft/2;
	fprintf(trns[plr],"C ");
	}
fleet[vict].pdu+=amt;
fleet[sbj].pdu-=amt;
fleet[vict].cargleft-=amt*2;
fleet[sbj].cargleft+=amt*2;
fprintf(trns[plr],"S%dT%dS%dD\n",sbj+100,amt,vict+100);
}

/*****************************************************************************/
void TendMin(Ship sbj,Ship vict,Amount amt)
/*****************************************************************************/
/* Transfer mines from one ship to another */
{
TRTEND(printf("TendMin(sbj:%d,vict:%d,amt:%d)\n",sbj,vict,amt));

fprintf(trns[plr],"S%dT%dS%dM\t",sbj+100,amt,vict+100);
if(fleet[sbj].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,sbj+100);
	return;
	}
if(fleet[vict].owner!=plr && alliance[fleet[vict].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own ship %d\n",vict+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,vict+100);
	return;
	}
if(fleet[vict].planet!=fleet[sbj].planet) {
	fprintf(trns[plr],"Ships are not around same planet\n");
	return;
	}
if(fleet[sbj].mines<amt) {
	amt=fleet[sbj].mines;
	fprintf(trns[plr],"M ");
	}
if(fleet[vict].cargleft<amt*20) {
	amt=fleet[vict].cargleft/20;
	fprintf(trns[plr],"C ");
	}
fleet[sbj].mines-=amt;
fleet[vict].mines+=amt;
fleet[sbj].cargleft-=amt*20;
fleet[vict].cargleft+=amt*20;
fprintf(trns[plr],"S%dT%dS%dM\n",sbj+100,amt,vict+100);
}

/*****************************************************************************/
void TendInd(Ship sbj,Ship vict,Amount amt)
/*****************************************************************************/
/* Transfer industry from one ship to another */
{
TRTEND(printf("TendInd(sbj:%d,vict:%d,amt:%d)\n",sbj,vict,amt));

fprintf(trns[plr],"S%dT%dS%dI\t",sbj+100,amt,vict+100);
if(fleet[sbj].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,sbj+100);
	return;
	}
if(fleet[vict].owner!=plr && alliance[fleet[vict].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own ship %d\n",vict+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,vict+100);
	return;
	}
if(fleet[vict].planet!=fleet[sbj].planet) {
	fprintf(trns[plr],"Ships are not around same planet\n");
	return;
	}

if(fleet[sbj].ind<amt) {
	amt=fleet[sbj].ind;
	fprintf(trns[plr],"I ");
	}

if(fleet[vict].cargleft<amt*10) {
	amt=fleet[vict].cargleft/10;
	fprintf(trns[plr],"C ");
	}

fleet[sbj].ind-=amt;
fleet[sbj].cargleft+=amt*10;
fleet[vict].ind+=amt;
fleet[vict].cargleft-=amt*10;

fprintf(trns[plr],"S%dT%dS%dI\n",sbj+100,amt,vict+100);
}

/*****************************************************************************/
void TendAll(Ship sbj,Ship vict)
/*****************************************************************************/
/* Transfer all ore from one ship to another except type 0 */
{
Oretype count;
Amount amt;

TRTEND(printf("TendAll(sbj:%d,vict:%d)\n",sbj,vict));

fprintf(trns[plr],"S%dTS%d\t",sbj+100,vict+100);
if(fleet[sbj].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",sbj+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,sbj+100);
	return;
	}
if(fleet[vict].owner!=plr && alliance[fleet[vict].owner][plr]!=ALLY) {
	fprintf(trns[plr],"You do not own ship %d\n",vict+100);
	fprintf(stderr,"Plr %d does not own ship %d\n",plr,vict+100);
	return;
	}
if(fleet[vict].planet!=fleet[sbj].planet) {
	fprintf(trns[plr],"Ships are not around same planet\n");
	return;
	}

for (count=1;count<10;count++) {
	amt=fleet[sbj].ore[count];
	if(fleet[vict].cargleft<amt)
		amt=fleet[vict].cargleft;
	fleet[sbj].ore[count]-=amt;
	fleet[sbj].cargleft+=amt;
	fleet[vict].ore[count]+=amt;
	fleet[vict].cargleft-=amt;
	}
fprintf(trns[plr],"S%dTS%d\n",sbj+100,vict+100);
}
