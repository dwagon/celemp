/*      UNLOAD.C        Module of Celestial Empire */
/* that deals with all the unloading of cargo   */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/unload.c,v 1.47 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: unload.c,v $
 * Revision 1.47  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.46  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.45  1992/11/09  03:56:35  dwagon
 * PDUs can now be used on the turn that they are unloaded
 *
 * Revision 1.44  1992/09/16  14:03:39  dwagon
 * Initial RCS'd version
 * */

/* 21/5/92	Removed all underscores
 */

#include "def.h"

#define TRUNL(x)	if(strstr(dbgstr,"UNLOAD") || strstr(dbgstr,"unload")) x

extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern Player plr;
extern char *dbgstr;

/*****************************************************************************/
void UnlSpcmin(Ship shp,Amount amt)
/*****************************************************************************/
/* Unload space mines from ship to planetary stockpile */
{
Planet num;

TRUNL(printf("unload:UnlSpcmin(shp:%d,amt:%d)\n",shp,amt));
fprintf(trns[plr],"S%dU%dD\t",shp+100,amt);
if(fleet[shp].owner!=plr) {
	fprintf(stderr,"unload:UnlSpcmin:Plr %d does not own ship %d\n",plr,shp);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
num=fleet[shp].planet;
if(galaxy[num].owner!=plr) {
	fprintf(stderr,"unload:UnlSpcmin:Plr %d does not own planet %d\n",plr,num);
	fprintf(trns[plr],"You do not own planet %d\n",num+100);
	return;
	}
if(amt>fleet[shp].spacemines)
	amt=fleet[shp].spacemines;
galaxy[num].spacemine+=amt;
fleet[shp].spacemines-=amt;
fleet[shp].cargleft+=amt;
fprintf(trns[plr],"S%dUS\n",shp+100);
return;
}

/*****************************************************************************/
void UnlOre(Oretype typ,Ship shp,Amount amt)
/*****************************************************************************/
/* Unload ore of a specific type        */
{
TRUNL(printf("unload:UnlOre(typ:%d,shp:%d,amt:%d)\n",typ,shp,amt));

fprintf(trns[plr],"S%dU%dR%d\t",shp+100,amt,typ);
if (fleet[shp].owner != plr) {
	fprintf(stderr,"unload:UnlOre:Plr %d does not own ship %d\n",plr,shp);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if (fleet[shp].ore[typ] < amt)
	amt = fleet[shp].ore[typ];
galaxy[fleet[shp].planet].ore[typ] += amt;
fleet[shp].ore[typ] -= amt;
fleet[shp].cargleft += amt;
fprintf(trns[plr],"S%dU%dR%d\n",shp+100,amt,typ);
}

/*****************************************************************************/
void UnlDef(Ship shp,Amount amt)
/*****************************************************************************/
/* Unload PDU's */
{
TRUNL(printf("unload:UnlDef(shp: %d, amt: %d)\n",shp,amt));

fprintf(trns[plr],"S%dU%dD\t",shp+100,amt);
if (fleet[shp].owner !=plr) {
	fprintf(stderr,"unload:UnlDef:Plr %d does not own ship %d\n",plr,shp);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if (fleet[shp].pdu < amt)
	amt = fleet[shp].pdu;
galaxy[fleet[shp].planet].pdu+=amt;
galaxy[fleet[shp].planet].pduleft+=amt;
fleet[shp].pdu-=amt;
fleet[shp].cargleft+=amt*2;
fprintf(trns[plr],"S%dU%dD\n",shp+100,amt);
}

/*****************************************************************************/
void UnlMin(Oretype typ,Ship shp,Amount amt)
/*****************************************************************************/
/* Unload mine  */
{
Player     num;

TRUNL(printf("unload:UnlMin(typ:%d,shp:%d,amt:%d)\n",typ,shp,amt));

fprintf(trns[plr],"S%dU%dM%d\t",shp+100,amt,typ);
num = fleet[shp].planet;
if (fleet[shp].owner!=plr) {
	fprintf(stderr,"unload:UnlMin:Plr %d doesnt own shp %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if(galaxy[num].mine[typ]==0) {
	fprintf(stderr,"unload:UnlMin:No mine of typ %d on plan %d\n",typ,num+100);
	fprintf(trns[plr],"No mine type %d on planet %d\n",typ,num+100);
	return;
	}

if (fleet[shp].mines<amt) {
	fprintf(trns[plr],"M ");
	amt=fleet[shp].mines;
	}
galaxy[num].mine[typ]+=amt;
fleet[shp].mines-=amt;
fleet[shp].cargleft+=amt*20;
galaxy[num].income+=amt;
fprintf(trns[plr],"S%dU%dM%d\n",shp+100,amt,typ);
}

/*****************************************************************************/
void UnlInd(Ship shp,Amount amt)
/*****************************************************************************/
/* Unload industry from ship */
{
Planet num=fleet[shp].planet;

TRUNL(printf("unload:UnlInd(shp:%d,amt:%d)\n",shp,amt));
fprintf(trns[plr],"S%dU%dI\t",shp+100,amt);
if (fleet[shp].owner!=plr) {
	fprintf(stderr,"unload:UnlInd:Plr %d does not own ship %d\n",plr,shp);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if (fleet[shp].ind<amt)
	amt=fleet[shp].ind;
galaxy[num].ind+=amt;
fleet[shp].ind-=amt;
fleet[shp].cargleft+=amt*10;
galaxy[num].income+=amt*5;
fprintf(trns[plr],"S%dU%dI\n",shp+100,amt);
}

/*****************************************************************************/
void UnlAll(Ship shp)
/*****************************************************************************/
/* Unload all ores except 0 from ship */
{
Planet num=fleet[shp].planet;
Oretype count;

TRUNL(printf("unload:UnlAll(shp:%d)\n",shp));
fprintf(trns[plr],"S%dU\t",shp+100);
if(plr!=fleet[shp].owner) {
	fprintf(stderr,"unload:UnlAll:Plr %d doesnt own shp %d\n",plr+1,shp+100);
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}

for (count=1;count<=10;count++) {
	galaxy[num].ore[count]+=fleet[shp].ore[count];
	fleet[shp].cargleft+=fleet[shp].ore[count];
	fleet[shp].ore[count]=0;
	}
fprintf(trns[plr],"S%dU\n",shp+100);
}
