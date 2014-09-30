/* Earth trading for Celestial Empire gm by Zer Dwagon */
/* (c) 1992 Dougal Scott */
/* $Header: /nelstaff/edp/dwagon/rfs/RCS/earth.c,v 1.49 1993/10/11 10:10:18 dwagon Exp $ */
/* $Log: earth.c,v $
 * Revision 1.49  1993/10/11  10:10:18  dwagon
 * Implemented restricted selling on Earth
 *
 * Revision 1.48  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.47  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.46  1992/12/14  04:35:30  dwagon
 * Tells you how much you spent.
 *
 * Revision 1.45  1992/12/08  06:15:51  dwagon
 * Fixed problem in global sell. The cargoleft was not being increased
 * properly.
 *
 * Revision 1.44  1992/09/16  13:53:32  dwagon
 * Initial RCS'd version
 * */

/* 26/4/92	Added CalcEarthDmg and modified various functions so that if
 *			Earth is damaged, it retains resources to rebuild itself 
 * 2/5/92	Changed the sell command to include global sell command
 * 18/5/92	Added game details structure
 * 21/5/92	Removed all underscores
 ****** VERSION 1.44 ******
 * 20/6/92	Fixed bug in BuyOre which applied earthmult to credits not score
 * 3/9/92	Earth will destroy 10% of all spacemines over it 
 */

#include "def.h"

#define TRERTH(x)	if(strstr(dbgstr,"EARTH") || strstr(dbgstr,"earth")) x

extern FILE   *bidfp;
extern FILE   *trns[NUMPLAYERS+1];
extern ship fleet[NUMSHIPS];
extern planet galaxy[NUMPLANETS];
extern game gamedet;
extern Number ecredit[NUMPLAYERS+1];
extern Number score[NUMPLAYERS+1];
extern Flag	  alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern Number price[10];
extern Player plr;
Number traded[10][2];		/* Amount of ore bought and sold */
int indneed=0,r4need=0;	/* Resources needed to rebuilt Earth */
int r8need=0,r9need=0;	
extern char *dbgstr;

/*****************************************************************************/
void RebuildEarth(void)
/*****************************************************************************/
/* If PDUs are less than original build some more to the limit of our 
resources, and if we have any more r8, r9 and ind left over, build some more
mine 4s. Also check for spacemines, and destroy 10% of all that exist.
*/
{
Amount amnt,a8,a9;
Planet erf=LocateEarth();		/* Planet number of Earth */

TRERTH(printf("earth:RebuildEarth()\n"));

if(galaxy[erf].deployed>0) {
	TRERTH(printf("Striping spacemines from Earth\n"));
	TRERTH(printf("Originally:%d\n",galaxy[erf].deployed));
	galaxy[erf].deployed=(galaxy[erf].deployed*9)/10;
	TRERTH(printf("Now:%d\n",galaxy[erf].deployed));
	}

if(galaxy[erf].pdu>=gamedet.earth.pdu) {
	TRERTH(printf("no rebuilding necessary\n"));
	return;
	}

/* Build more PDUs */
amnt=MIN(galaxy[erf].ore[4],galaxy[erf].indleft);
fprintf(stderr,"rebuilding %d PDUs on Earth\n",amnt);
galaxy[erf].pdu+=amnt;
galaxy[erf].indleft-=amnt;
galaxy[erf].ore[4]-=amnt;

/* Build more mine 4s */
a8=galaxy[erf].ore[8];
a9=galaxy[erf].ore[9];
if(a8>5 && a9>5) {
	amnt=MIN(MIN(a8/5,a9/5),galaxy[erf].indleft/10);
	galaxy[erf].mine[4]+=amnt;
	galaxy[erf].ore[8]-=amnt*5;
	galaxy[erf].ore[9]-=amnt*5;
	galaxy[erf].indleft-=10*amnt;
	galaxy[erf].income+=amnt;
	fprintf(stderr,"rebuilding %d mine 4s on Earth\n",amnt);
	}
return;
}

/*****************************************************************************/
void CalcEarthDmg(void)
/*****************************************************************************/
/* Calculate how much industry and ore is needed to rebuild Earth if damaged */
/* so players don't override Earths needs */
{
Amount defamnt,amnt,a8,a9;
Planet erf=LocateEarth();		/* Planet number of Earth */

TRERTH(printf("earth:CalcEarthDmg()\n"));
if(galaxy[erf].pdu>=gamedet.earth.pdu) {
	return;
	}

/* Build more PDUs */
defamnt=MIN(galaxy[erf].ore[4],galaxy[erf].indleft);
r4need=defamnt;
indneed=defamnt;
TRERTH(printf("Going to rebuild %d PDUs\n",defamnt));

/* Build more mine 4s */
a8=galaxy[erf].ore[8];
a9=galaxy[erf].ore[9];
if(a8>5 && a9>5) {
	amnt=MIN(MIN(a8/5,a9/5),(galaxy[erf].indleft-defamnt)/10);
	indneed+=amnt*10;
	r8need=amnt*5;
	r9need=amnt*5;
	TRERTH(printf("Going to rebuild %d industry\n",amnt));
	}
TRERTH(printf("Need %d R4, %d R8, %d R9 and %d ind\n",r4need,r8need,r9need,indneed));
return;
}

/*****************************************************************************/
void DoEarth(void)
/*****************************************************************************/
/* Process the contracting done on Earth in bid order, highest first */
{
Ship sbj;
char type;
Amount amt,bid;

TRERTH(printf("earth:DoEarth()\n"));
while(1) {
	if(fscanf(bidfp,"%hd %c %d %d %hd",&sbj,&type,&amt,&bid,&plr)==EOF)     
		return;                        
	switch(type) {
		case 'c':   ContCarg(sbj,amt,bid);
					break;
		case 'f':   ContFght(sbj,amt,bid);
					break;
		case 't':   ContTrac(sbj,amt,bid);
					break;
		case 's':   ContShld(sbj,amt,bid);
					break;
		default:
			fprintf(stderr,"earth.c:\tsnafu: Unknown contract type:%c\n",type);
			break;
		}
	}
}

/*****************************************************************************/
void ContCarg(Ship shp,Amount amt,Number bid)
/*****************************************************************************/
/* Build cargo units on ship over earth */
{
int espent=0,sspent=0;

TRERTH(printf("earth:ContCarg(shp: %d,amt: %d, bid: %d)\n",shp,amt,bid));

fprintf(trns[plr],"S%dB%dC%d\t",shp+100,amt,bid);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	TRERTH(fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100));
	return;
	}

if(!IsEarth(fleet[shp].planet)) {
	fprintf(trns[plr],"Ship %d is not over Earth\n",shp+100);
	TRERTH(fprintf(stderr,"Ship %d is not over Earth\n",shp+100));
	return;
	}

if(alliance[plr][plr]==ENEMY) {
	fprintf(trns[plr],"Earth doesn't do business with enemies\n");
	return;
	}

if(bid<gamedet.earth.cbid) {
	fprintf(trns[plr],"Minimum bid for cargo is %d\n",gamedet.earth.cbid);
	return;
	}
if(fleet[shp].ore[1]<amt) {
	fprintf(trns[plr],"R1 ");
	amt=fleet[shp].ore[1];
	}
if(bid*amt>ecredit[plr]+(score[plr]/gamedet.earth.earthmult)) {
	fprintf(trns[plr],"$ ");
	amt=((score[plr]/gamedet.earth.earthmult)+ecredit[plr])/bid;
	}
if(galaxy[fleet[shp].planet].indleft-indneed<amt) {
	fprintf(trns[plr],"I ");
	amt=galaxy[fleet[shp].planet].indleft-indneed;
	}

espent=MIN(ecredit[plr],amt*bid);
ecredit[plr]-=amt*bid;
if(ecredit[plr]<0) {
	sspent=-ecredit[plr]*gamedet.earth.earthmult;	
	score[plr]-=sspent;
	ecredit[plr]=0;
	}
galaxy[fleet[shp].planet].indleft-=amt;
fleet[shp].cargo+=amt;
fleet[shp].cargleft+=2*amt;		/* 1 for ore, tother for new carg units */
fleet[shp].ore[1]-=amt;
fleet[shp].type=CalcType(shp);
fprintf(trns[plr],"S%dB%dC%d ",shp+100,amt,bid);
if(amt>0)
	fprintf(trns[plr]," Spent ");
if(espent>0)
	fprintf(trns[plr],"%d credits",espent);
if(espent>0 && sspent>0)
	fprintf(trns[plr]," and ");
if(sspent>0)
	fprintf(trns[plr],"%d score",sspent);
fprintf(trns[plr],"\n");
return;
}

/*****************************************************************************/
void ContFght(Ship shp,Amount amt,Number bid)
/*****************************************************************************/
/* Build fighter units on ship over earth */
{
int espent=0,sspent=0;
Planet erf=LocateEarth();

TRERTH(printf("earth:ContFght(shp: %d,amt: %d,bid: %d)\n",shp,amt,bid));

fprintf(trns[plr],"S%dB%dF%d\t",shp+100,amt,bid);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	TRERTH(fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100));
	return;
	}

if(!IsEarth(fleet[shp].planet)) {
	fprintf(trns[plr],"Ship %d is not over Earth\n",shp+100);
	TRERTH(fprintf(stderr,"Ship %d is not over Earth\n",shp+100));
	return;
	}

if(alliance[plr][plr]==ENEMY) {
	fprintf(trns[plr],"Earth doesn't do business with enemies\n");
	return;
	}

if(bid<gamedet.earth.fbid) {
	fprintf(trns[plr],"Minimum bid for fighter is %d\n",gamedet.earth.fbid);
	return;
	}
if(fleet[shp].ore[2]<amt) {
	TRERTH(printf("Reducing fighter amount by R2 to %d\n",fleet[shp].ore[2]));
	amt=fleet[shp].ore[2];
	fprintf(trns[plr],"R2 ");
	}
if(fleet[shp].ore[3]<amt) {
	TRERTH(printf("Reducing fighter amount by R3 to %d\n",fleet[shp].ore[3]));
	fprintf(trns[plr],"R3 ");
	amt=fleet[shp].ore[3];
	}
if(bid*amt>(score[plr]/gamedet.earth.earthmult+ecredit[plr])) {
	TRERTH(printf("Reducing fighter amount by $ to %d\n",(score[plr]/gamedet.earth.earthmult+ecredit[plr])/(bid*2)));
	fprintf(trns[plr],"$ ");
	amt=(score[plr]/gamedet.earth.earthmult+ecredit[plr])/(bid*2);
	}
if((galaxy[erf].indleft-indneed)/2<amt) {
	TRERTH(printf("Reducing fighter amount by I to %d\n",(galaxy[erf].indleft-indneed)/2));
	fprintf(trns[plr],"I ");
	amt=(galaxy[erf].indleft-indneed)/2;
	}

espent=MIN(ecredit[plr],amt*bid*2);
ecredit[plr]-=amt*bid*2;
if(ecredit[plr]<0) {
	sspent= -ecredit[plr]*gamedet.earth.earthmult;
	score[plr]-= sspent;
	ecredit[plr]=0;
	}
galaxy[fleet[shp].planet].indleft-=2*amt;
fleet[shp].fight+=amt;
fleet[shp].cargleft+=amt*2;
fleet[shp].ore[2]-=amt;
fleet[shp].ore[3]-=amt;
fleet[shp].type=CalcType(shp);
fprintf(trns[plr],"S%dB%dF%d",shp+100,amt,bid);
if(amt>0)
	fprintf(trns[plr]," Spent ");
if(espent>0)
	fprintf(trns[plr],"%d credits",espent);
if(espent>0 && sspent>0)
	fprintf(trns[plr]," and ");
if(sspent>0)
	fprintf(trns[plr],"%d score",sspent);
fprintf(trns[plr],"\n");
}

/*****************************************************************************/
void ContTrac(Ship shp,Amount amt,Number bid)
/*****************************************************************************/
/* Contract to build tractor units on Earth */
{
int espent=0,sspent=0;

TRERTH(printf("earth:ContTrac(shp: %d,amt: %d,bid: %d)\n",shp,amt,bid));

fprintf(trns[plr],"S%dB%dT%d\t",shp+100,amt,bid);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	TRERTH(fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100));
	return;
	}

if(!IsEarth(fleet[shp].planet)) {
	fprintf(trns[plr],"Ship %d is not over Earth\n",shp+100);
	TRERTH(fprintf(stderr,"Ship %d is not over Earth\n",shp+100));
	return;
	}

if(alliance[plr][plr]==ENEMY) {
	fprintf(trns[plr],"Earth doesn't do business with enemies\n");
	return;
	}

if(bid<gamedet.earth.tbid) {
	fprintf(trns[plr],"Minimum bid for tractor is %d\n",gamedet.earth.tbid);
	return;
	}
if(fleet[shp].ore[7]<amt*2) {
	amt=fleet[shp].ore[7]/2;
	fprintf(trns[plr],"R7 ");
	}
if((score[plr]+ecredit[plr])<bid*amt*2) {
	amt=(score[plr]/gamedet.earth.earthmult+ecredit[plr])/(2*bid);
	fprintf(trns[plr],"$ ");
	}
if((galaxy[fleet[shp].planet].indleft-indneed)/2<amt) {
	amt=galaxy[fleet[shp].planet].indleft-indneed;
	fprintf(trns[plr],"I ");
	}
galaxy[fleet[shp].planet].indleft-=2*amt;
espent=MIN(ecredit[plr],amt*bid*2);
ecredit[plr]-=amt*bid*2;
if(ecredit[plr]<0) {
	sspent=-ecredit[plr]*gamedet.earth.earthmult;
	score[plr]-=sspent;
	ecredit[plr]=0;
	}
fleet[shp].tractor+=amt;
fleet[shp].cargleft+=amt*2;
fleet[shp].ore[7]-=amt*2;
fprintf(trns[plr],"S%dB%dT%d",shp+100,amt,bid);
if(amt>0)
	fprintf(trns[plr]," Spent ");
if(espent>0)
	fprintf(trns[plr],"%d credits",espent);
if(espent>0 && sspent>0)
	fprintf(trns[plr]," and ");
if(sspent>0)
	fprintf(trns[plr],"%d score",sspent);
fprintf(trns[plr],"\n");
}

/*****************************************************************************/
void ContShld(Ship shp,Amount amt,Number bid)
/*****************************************************************************/
/* Contract to build shield units on Earth */
{
int espent=0,sspent=0;

TRERTH(printf("earth:ContShld(shp: %d,amt: %d, bid: %d)\n",shp,amt,bid));

fprintf(trns[plr],"S%dB%dS%d\t",shp+100,amt,bid);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	TRERTH(fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100));
	return;
	}

if(!IsEarth(fleet[shp].planet)) {
	fprintf(trns[plr],"Ship %d is not over Earth\n",shp+100);
	TRERTH(fprintf(stderr,"Ship %d is not over Earth\n",shp+100));
	return;
	}

if(alliance[plr][plr]==ENEMY) {
	fprintf(trns[plr],"Earth doesn't do business with enemies\n");
	return;
	}

if(bid<gamedet.earth.sbid) {
	fprintf(trns[plr],"Minimum bid for shield is %d\n",gamedet.earth.sbid);
	return;
	}
if(fleet[shp].ore[5]<amt) {
	amt=fleet[shp].ore[5];
	fprintf(trns[plr],"R5 ");
	}
if(fleet[shp].ore[6]<amt) {
	amt=fleet[shp].ore[6];
	fprintf(trns[plr],"R6 ");
	}
if((ecredit[plr]/gamedet.earth.earthmult+score[plr])<bid*amt*2) {
	amt=(ecredit[plr]+score[plr])/(2*bid);
	fprintf(trns[plr],"$ ");
	}
if((galaxy[fleet[shp].planet].indleft-indneed)/2<amt) {
	amt=galaxy[fleet[shp].planet].indleft-indneed;
	fprintf(trns[plr],"I ");
	}

galaxy[fleet[shp].planet].indleft-=2*amt;
espent=MIN(ecredit[plr],amt*bid*2);
ecredit[plr]-=bid*amt*2;
if(ecredit[plr]<0) {
	sspent=-ecredit[plr]*gamedet.earth.earthmult;
	score[plr]-=sspent;
	ecredit[plr]=0;
	}
fleet[shp].shield+=amt;
fleet[shp].ore[5]-=amt;
fleet[shp].ore[6]-=amt;
fleet[shp].cargleft+=amt*2;
fprintf(trns[plr],"S%dB%dS%d",shp+100,amt,bid);
if(amt>0)
	fprintf(trns[plr]," Spent ");
if(espent>0)
	fprintf(trns[plr],"%d credits",espent);
if(espent>0 && sspent>0)
	fprintf(trns[plr]," and ");
if(sspent>0)
	fprintf(trns[plr],"%d score",sspent);
fprintf(trns[plr],"\n");
}

/*****************************************************************************/
void BuyOre(Ship shp,Amount amt,Oretype type)
/*****************************************************************************/
/* Buy ore from Earth Market */
{
int espent=0,sspent=0;

Planet erf=LocateEarth();		/* Planet number of Earth */
TRERTH(printf("earth:BuyOre(shp: %d,amt: %d,type: %d)\n",shp,amt,type));

fprintf(trns[plr],"S%dP%dR%d\t",shp+100,amt,type);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	TRERTH(fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100));
	return;
	}

if(!IsEarth(fleet[shp].planet)) {
	fprintf(trns[plr],"Ship %d is not over Earth\n",shp+100);
	TRERTH(fprintf(stderr,"Ship %d is not over Earth\n",shp+100));
	return;
	}

if(alliance[plr][plr]==ENEMY) {
	fprintf(trns[plr],"Earth doesn't do business with enemies\n");
	return;
	}

if(amt>fleet[shp].cargleft) {
	fprintf(trns[plr],"C ");
	amt=fleet[shp].cargleft;
	}
if((score[plr]/gamedet.earth.earthmult+ecredit[plr])<amt*price[type]) {
	amt=(ecredit[plr]+score[plr]/gamedet.earth.earthmult)/price[type];
	fprintf(trns[plr],"$ ");
	}
if(galaxy[erf].ore[type]-!!(type==4)*r4need-!!(type==8)*r8need-!!(type==9)*r9need<amt) {
	amt=galaxy[erf].ore[type];
	fprintf(trns[plr],"R%d ",type);
	}
	
galaxy[erf].ore[type]-=amt;
fleet[shp].cargleft-=amt;
fleet[shp].ore[type]+=amt;
espent=MIN(ecredit[plr],amt*price[type]);
ecredit[plr]-=amt*price[type];
if(ecredit[plr]<0) {
	sspent=-ecredit[plr]*gamedet.earth.earthmult;
	score[plr]-=sspent;
	ecredit[plr]=0;
	}
traded[type][BOUGHT]+=amt;
fprintf(trns[plr],"S%dP%dR%d",shp+100,amt,type);
if(amt>0)
	fprintf(trns[plr]," Spent ");
if(espent>0)
	fprintf(trns[plr],"%d credits",espent);
if(espent>0 && sspent>0)
	fprintf(trns[plr]," and ");
if(sspent>0)
	fprintf(trns[plr],"%d score",sspent);
fprintf(trns[plr],"\n");
}

/*****************************************************************************/
void SellOre(Ship shp,Amount amt,Oretype type)
/*****************************************************************************/
/* Sell ore to Earth Market */
{
int tmp;
TRERTH(printf("earth:SellOre(shp: %d,amt: %d,type: %d)\n",shp,amt,type));

if(amt<0)
	fprintf(trns[plr],"S%dX\t",shp+100);
else
	fprintf(trns[plr],"S%dX%dR%d\t",shp+100,amt,type);
if(plr!=fleet[shp].owner) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	TRERTH(fprintf(stderr,"Plr %d does not own ship %d\n",plr,shp+100));
	return;
	}

if(!IsEarth(fleet[shp].planet)) {
	fprintf(trns[plr],"Ship %d is not over Earth\n",shp+100);
	TRERTH(fprintf(stderr,"Ship %d is not over Earth\n",shp+100));
	return;
	}

if(alliance[plr][plr]==ENEMY) {
	fprintf(trns[plr],"Earth doesn't do business with enemies\n");
	return;
	}

if(amt>fleet[shp].ore[type]) {
	amt=fleet[shp].ore[type];
	fprintf(trns[plr],"R%d ",type);
	}

if(amt>0 && gamedet.earth.flag&WBUY100ORE) {
	if(amt>100-galaxy[fleet[shp].planet].ore[type])
		amt=100-galaxy[fleet[shp].planet].ore[type];
		fprintf(trns[plr],"EL ");
	}

/* Global sell command */
if(amt<0) {
	for(tmp=9;tmp>0;tmp--) {
		amt=fleet[shp].ore[tmp];
		if(gamedet.earth.flag&WBUY100ORE)
			if(amt>100-galaxy[fleet[shp].planet].ore[tmp])
				amt=100-galaxy[fleet[shp].planet].ore[tmp];
		galaxy[fleet[shp].planet].ore[tmp]+=amt;
		ecredit[plr]+=(int)((float)amt*(float)price[tmp]*(float)2/(float)3);
		fleet[shp].cargleft+=amt;
		fleet[shp].ore[tmp]-=amt;
		traded[tmp][SOLD]+=amt;
		}
	fprintf(trns[plr],"S%dX\n",shp+100);
	}
else {
	galaxy[fleet[shp].planet].ore[type]+=amt;
	ecredit[plr]+=(int)((float)amt*(float)price[type]*(float)2/(float)3);
	fleet[shp].ore[type]-=amt;
	fleet[shp].cargleft+=amt;
	traded[type][SOLD]+=amt;
	fprintf(trns[plr],"S%dX%dR%d\n",shp+100,amt,type);
	}
}
