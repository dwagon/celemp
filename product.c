/*********************************************************************/
/* File to implement the production of mines in Celestial Empire     */
/* and also to facilitate change of ownership, damage reolution and  */
/* end of turn processing                                            */
/* Written by Zer Dwagon 1988 - 1992 (c) Zer Dwagon 1992 			 */
/*********************************************************************/

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/product.c,v 1.55 1993/11/02 02:31:58 dwagon Exp $ */
/* $Log: product.c,v $
 * Revision 1.55  1993/11/02  02:31:58  dwagon
 * Added include "typname.h" to respond to changes in location of type defs
 *
 * Revision 1.54  1993/10/20  03:58:13  dwagon
 * Added turn winning condition
 *
 * Revision 1.53  1993/09/16  04:53:59  dwagon
 * Added new winning condition: Earth credits
 *
 * Revision 1.52  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.51  1993/07/06  06:58:14  dwagon
 * Added definition for NEUTPLR instead of 9 for future changes
 *
 * Revision 1.50  1993/05/24  04:14:04  dwagon
 * Added income as one of the winninf conditions checked for
 *
 * Revision 1.49  1993/05/19  00:13:28  dwagon
 * Implemented scaled shields
 *
 * Revision 1.48  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.47  1992/11/09  01:06:55  dwagon
 * Fixed a minor bug with attack resolution
 *
 * Revision 1.46  1992/10/21  05:35:42  dwagon
 * PDU attack resolution merged with fighter attack resolution.
 * PerformPDUshots removed as unnecessary
 *
 * Revision 1.45  1992/10/05  01:32:42  dwagon
 * Fixed bug in planetary ownership code where the alliance level of the
 * neutral player was being interrogated off the end of the array
 *
 * Revision 1.44  1992/09/16  13:59:53  dwagon
 * Initial RCS'd version
 * */

               /******** VERSION 1.43 ********/
/* 26/4/92	When a person takes over a planet, they are told who owned
 *			it previously
 *			When a ship is captured, you are informed.
 * 28/4/92 Fixed a slight bug in the above, where it tells you you took over
 * 			a planet, even if you were the previous owner.
 *			Rewrote ChngOwner to be much nicer.
 * 2/5/92	Rewrote remove cargo. If there are now no cargo units, drop
 *			everything. Also changed parameters, it now only takes the ship
 *			number.
 *			Changed DistrMine so if the planet has no mines, then the mine
 *			is destroyed, stops program going into an endless loop, looking for
 *			a suitable mine.
 *			Changed CalcPrices so that it is harder to move the market, the
 *			further into the game it is
 *			Added ALLY status. Allies cannot change ownership of your planets
 * 18/5/92	Fixed up bug in CalcPrices which caused ore prices to go to 1
 *			Added game details structure
 *			Check winning conditions
 * 21/5/92	Remove underscores
 */

#include "def.h"
#include "typname.h"

/* There are three debugging levels in this file.
 * TRPROD prints out a message with function parameters every time a
 * major procedure is entered. It also prints damage resolution details.
 * TRPROD2 is for printing out details of procedures which occur large
 * numbers of times per turn, such as ship detection.
 * TRPROD3 is just for mine production, which occurs a lot every turn. */

#define TRPROD(x)	if(strstr(dbgstr,"PROD") || strstr(dbgstr,"prod")) x
#define TRPROD2(x)	if(strstr(dbgstr,"PRO2") || strstr(dbgstr,"pro2")) x
#define TRPROD3(x)	if(strstr(dbgstr,"PRO3") || strstr(dbgstr,"pro3")) x

extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];  
extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern int turn;
extern game gamedet;
extern Ship shiptr;
extern Number score[9];
extern Number price[10];
extern Amount traded[10][2];
extern int shipvict[NUMSHIPS][NUMPLAYERS+1][2];
extern char *dbgstr;
extern char name[NUMPLAYERS+1][10];
extern Number ecredit[NUMPLAYERS+1];
extern int desturn[NUMPLAYERS+1];

/*****************************************************************************/
void ProdMine(Planet num,Oretype min)
/*****************************************************************************/
/* Let mine type `min' produce ore on planet `num' */
{
TRPROD3(printf("product:ProdMine(num: %d,min: %d)\n",num,min));
galaxy[num].ore[min] += galaxy[num].mine[min];
}

/*****************************************************************************/
Ship DetectShip(Ship num,Ship start)
/*****************************************************************************/
/* Check to see if there is a ship above the planet `num'
 * and returns the number of the ship or -1 if no ship present.
 * Start looking from ship number `start'.
 * Empty Hulls do not count as ships for ownership purposes.
 */
{
Ship     count;

TRPROD2(printf("product:DetectShip(num: %d,start: %d)\n",num,start));

for(count=start;count<shiptr;count++)
	if(fleet[count].planet==num && fleet[count].type!=HULL)
		return(count);
return(-1);
}

/*****************************************************************************/
int MultiOwner(Planet num)
/*****************************************************************************/
/* Check to see if the ships that are above a planet are all of the same owner.
 * If there are multiple ships with different owners return TRUE else return
 * FALSE, hulls make no difference.
 */
{
Ship sh,sh2;

TRPROD2(printf("product:MultiOwner(num: %d)\n",num));
sh2=sh=DetectShip(num,0);
do {
	sh2=DetectShip(num,sh2+1);
	if(sh2>=0 && fleet[sh].owner!=fleet[sh2].owner) {
		TRPROD2(printf("MultiOwner:%d:%d:return(TRUE)\n",sh,sh2));
		return(TRUE);
		}
	} while (sh2>=0);
TRPROD2(printf("MultiOwner:return(FALSE)\n"));
return(FALSE);
}

/*****************************************************************************/
void ChngOwner(Planet num)
/*****************************************************************************/
/* Change the ownership of planet if necessary.
 * Allies ships above a planet do not change the ownership.
 */
{
Ship   shp;
Player shpown;

TRPROD2(printf("product:ChngOwner(num: %d)\n",num));

if(galaxy[num].pdu!=0) {
	TRPROD2(printf("Planet %d is defended\n",num+100));
	return;
	}

if((shp=DetectShip(num,0))==-1) {
	TRPROD2(printf("Planet %d has no ships above it\n",num+100));
	return;
	}

TRPROD2(printf("Ship %d above planet %d\n",shp,num));
if(MultiOwner(num)==TRUE) {
	TRPROD2(printf("Multiple ships above planet %d\n",num));
	return;
	}

shpown=fleet[shp].owner;
if(galaxy[num].owner!=NEUTPLR && alliance[shpown][galaxy[num].owner]==ALLY) {
	TRPROD2(printf("Players %d[ship] and %d[planet] are allied\n",shpown,galaxy[num].owner));
	return;
	}

if(shpown!=galaxy[num].owner) {
	fprintf(trns[shpown],"Planet %d liberated from %s\n",num+100,name[galaxy[num].owner]);
	fprintf(trns[galaxy[num].owner],"Planet %d defeated by space attack\n",num+100);
	}

galaxy[num].owner=shpown;
galaxy[num].stndord[0]=0;
TRPROD2(printf("Ownership of planet %d changed to player %d\n",num,galaxy[num].owner));
}

/*****************************************************************************/
void ProdAll(Planet num)
/*****************************************************************************/
/* Get all the production going */
{
Oretype  i;

TRPROD2(printf("product:ProdAll(num: %d)\n",num));

if(galaxy[num].owner!=NEUTPLR)
	for(i=0;i<10;i++)
		ProdMine(num,i);
}

/*****************************************************************************/
void ChngHull(Ship shp)
/*****************************************************************************/
/* Change the owner of empty ships to the owner of the planet  */
{
TRPROD2(printf("product:ChngHull(shp: %d)\n",shp));

if(fleet[shp].type==0 && fleet[shp].owner!=galaxy[fleet[shp].planet].owner) {
	fprintf(trns[fleet[shp].owner],"Ship %d lost to space pirates\n",shp+100);
	fprintf(trns[galaxy[fleet[shp].planet].owner],"Ship S%d has been boarded by your space marines\n",shp+100);
	fleet[shp].owner=galaxy[fleet[shp].planet].owner;
	fleet[shp].stndord[0]=0;
	}
}

/*****************************************************************************/
void EndOfTurn(void)
/*****************************************************************************/
/* Execute the end of turn sequence */
/* Includes production, and change of ownership */
{
Planet   count;
Number numres[NUMPLAYERS+1];

TRPROD(printf("product:\tendofturn\tChecking planets\n"));

for(count=0;count<NUMPLAYERS;count++)
	numres[count]=0;
CalcPrices();
ResolveDamage();
for (count = 0;count<NUMPLANETS;count++) {
	ProdAll(count);
	ChngOwner(count);
	galaxy[count].indleft=galaxy[count].ind;
	galaxy[count].pduleft=galaxy[count].pdu;
	if(galaxy[count].owner!=NEUTPLR) {
		score[galaxy[count].owner]+=galaxy[count].income;
		if(IsResearch(count))
			numres[galaxy[count].owner]++;
		}
	}
for(count=0;count<NUMPLAYERS;count++) 
	if(numres[count]<4)
		score[count]+=numres[count]*numres[count]*numres[count]*numres[count];
	else
		score[count]+=256;
TRPROD(printf("product\tendofturn\tChecking ships\n"));
for (count=0;count<shiptr;count++) {
	fleet[count].moved=0;
	fleet[count].engage=0;
	fleet[count].figleft=fleet[count].fight;
	ChngHull(count);
	}
turn++;
CheckWinning();
}

/*****************************************************************************/
void ResolveDamage(void)
/*****************************************************************************/
/* Go through all the ships in turn and resolve any damage done */
{
Ship shp;

TRPROD(printf("product:\tResolving damage\n"));
for(shp=0;shp<shiptr;shp++)
	if((fleet[shp].hits+fleet[shp].pduhits)>0)
		PerformShots(shp);
}

/*****************************************************************************/
void PerformShots(Ship shp)
/*****************************************************************************/
/* Activate the damage caused by shooting during the turn */
{
Units tmp=fleet[shp].hits+fleet[shp].pduhits;
Units su=0,cu=0,tu=0,fu=0,as=0;
int count;

TRPROD(printf("product:PerformShots(shp: %d)\n",shp));

for(count=0;count<NUMPLAYERS;count++) {
	if(shipvict[shp][count][SHIP]>0)
		fprintf(trns[count],"Of %d hits on S%d this turn your fighters did %d\n",tmp,shp+100,shipvict[shp][count][SHIP]);
	if(shipvict[shp][count][PDU]>0)
		fprintf(trns[count],"Of %d hits on S%d this turn your PDUs did %d\n",tmp,shp+100,shipvict[shp][count][PDU]);
		}
tmp-=Shields(shp);	/* Reduce shots by effective shields */
as=MIN(Shields(shp),tmp);	/* Number absorbed by shields */
if(tmp<=0) {	/* All shots absorbed by shields */
	TRPROD(printf("product:\tShots absorbed by shield on ship:%d\n",shp+100));
	fprintf(trns[fleet[shp].owner],"S%d sustained %d shots from enemy fire but all were absorbed by shields\n",shp+100,fleet[shp].hits+fleet[shp].pduhits);
	for(count=0;count<NUMPLAYERS;count++)
		if(shipvict[shp][count][SHIP]+shipvict[shp][count][PDU]>0)
			fprintf(trns[count],"All shots were absorbed by shields\n");
	return;
	}

if(tmp>fleet[shp].shield) {		/* Destroy all shields */
	su=fleet[shp].shield;		/* Shields destroyed */
	tmp-=fleet[shp].shield;
	TRPROD(printf("product:\tShields all destoyed on ship:%d\n",shp+100));
	fleet[shp].shield=0;
	}
else {							/* Destroy some shields */
	su=tmp;
	fleet[shp].shield-=tmp;
	TRPROD(printf("product:\t%d shields destoyed on ship:%d\n",tmp,shp+100));
	tmp=0;
	}
if(tmp>fleet[shp].fight) {		/* Destroy all fighter */
    fu=fleet[shp].fight;		/* Fighters destroyed */
	tmp-=fleet[shp].fight;
	fleet[shp].fight=0;
	TRPROD(printf("product:\tFighters all destoyed on ship:%d\n",shp+100));
	}
else {							/* Destroy some fighter */
	fleet[shp].fight-=tmp;
	fu=tmp;
	TRPROD(printf("product:\t%d fighters destoyed on ship:%d\n",tmp,shp+100));
	tmp=0;
	}
if(tmp>fleet[shp].tractor) {		/* Destroy all tractor */
	tu=fleet[shp].tractor;		/* Tractor units destroyed */
	tmp-=fleet[shp].tractor;
	fleet[shp].tractor=0;
	TRPROD(printf("product:\tTractors all destoyed on ship:%d\n",shp+100));
	}
else {							/* Destroy some tractor */
	tu=tmp;
	fleet[shp].tractor-=tmp;
	TRPROD(printf("product:\t%d tractors destoyed on ship:%d\n",tmp,shp+100));
	tmp=0;
	}
if(tmp>fleet[shp].cargo) {		/* Destroy all cargo */
	cu=fleet[shp].cargo;
	tmp-=fleet[shp].cargo;
	fleet[shp].cargo=0;
	RemoveCargo(shp);
	RecalcCargo(shp);
	TRPROD(printf("product:\tCargos all destoyed on ship:%d\n",shp+100));
	}
else {							/* Destroy some cargo */
	TRPROD(printf("product:\t%d cargos destoyed on ship:%d\n",tmp,shp+100));
	cu=tmp;
	fleet[shp].cargo-=tmp;
	RemoveCargo(shp);
	RecalcCargo(shp);
	tmp=0;
	}
fleet[shp].type=CalcType(shp);
TRPROD(printf("product:\tNew ship type:%d\n",fleet[shp].type));
fprintf(trns[fleet[shp].owner],"S%d sustained %d shots from enemy fire\n",shp+100,fleet[shp].hits+fleet[shp].pduhits);
	if(as!=0) {
		fprintf(trns[fleet[shp].owner],"\t%d absorbed by shields\n",as);
		for(count=0;count<NUMPLAYERS;count++)
			if(shipvict[shp][count][SHIP]+shipvict[shp][count][PDU]>0)
				fprintf(trns[count],"\t%d absorbed by shields\n",as);
		}
	if(su!=0) {
		fprintf(trns[fleet[shp].owner],"\t%d shields units destroyed\n",su);
		for(count=0;count<NUMPLAYERS;count++)
			if(shipvict[shp][count][SHIP]+shipvict[shp][count][PDU]>0)
				fprintf(trns[count],"\t%d shield units destroyed\n",su);
		}
	if(fu!=0) {
		fprintf(trns[fleet[shp].owner],"\t%d fighter units destroyed\n",fu);
		for(count=0;count<NUMPLAYERS;count++)
			if(shipvict[shp][count][SHIP]+shipvict[shp][count][PDU]>0)
				fprintf(trns[count],"\t%d fighter units destroyed\n",fu);
		}
	if(tu!=0) {
		fprintf(trns[fleet[shp].owner],"\t%d tractor units destroyed\n",tu);
		for(count=0;count<NUMPLAYERS;count++)
			if(shipvict[shp][count][SHIP]+shipvict[shp][count][PDU]>0)
				fprintf(trns[count],"\t%d tractor units destroyed\n",tu);
		}
	if(cu!=0) {
		fprintf(trns[fleet[shp].owner],"\t%d cargo units destroyed\n",cu);
		for(count=0;count<NUMPLAYERS;count++)
			if(shipvict[shp][count][SHIP]+shipvict[shp][count][PDU]>0)
				fprintf(trns[count],"\t%d cargo units destroyed\n",cu);
		}
fleet[shp].hits=0;
fleet[shp].pduhits=0;
}

/*****************************************************************************/
void RecalcCargo(Ship shp)
/*****************************************************************************/
/* Recalc cargleft after cargo unit destruction */
{
Oretype x;
Units cl=fleet[shp].cargo;

TRPROD(printf("product:RecalcCargo(shp: %d)\n",shp));

for(x=0;x<10;x++)
	cl-=fleet[shp].ore[x];
cl-=fleet[shp].mines*20;
cl-=fleet[shp].ind*10;
cl-=fleet[shp].pdu*2;
cl-=fleet[shp].spacemines;
fleet[shp].cargleft=cl;
}

/*****************************************************************************/
void RemoveCargo(Ship shp)
/*****************************************************************************/
/* Remove cargo from a ship that cannot support it, */
/* ie one that has just been shot to bits. :) */
{
Oretype x;
Planet num=fleet[shp].planet;
Units neg;	/* Number of overloaded units */

TRPROD(printf("product:RemoveCargo(shp: %d)\n",shp));

if(fleet[shp].cargo==0) {
	TRPROD(printf("removing everything from ship %d\n",shp+100));
	for(;fleet[shp].mines>0;DistrMin(num),fleet[shp].mines--);
	galaxy[num].ind+=fleet[shp].ind;
	galaxy[num].income+=fleet[shp].ind*5;
	fleet[shp].ind=0;
	galaxy[num].pdu+=fleet[shp].pdu;
	fleet[shp].pdu=0;
	galaxy[num].spacemine=fleet[shp].spacemines;
	fleet[shp].spacemines=0;
	for(x=0;x<10;x++) {
		galaxy[num].ore[x]+=fleet[shp].ore[x];
		fleet[shp].ore[x]=0;
		}
	}

neg=fleet[shp].cargo-fleet[shp].mines*20-fleet[shp].ind*10-fleet[shp].pdu*2-fleet[shp].spacemines;
for(x=0;x<10;x++)
	neg-=fleet[shp].ore[x];

if(neg>0) {
	TRPROD(printf("No cargo removal necessary on ship %d\n",shp+100));
	return;
	}

neg= -neg;

TRPROD2(printf("product:\tRemoving Mines\n"));
for(;neg>0 && fleet[shp].mines!=0;) {
		neg-=20;
		fleet[shp].mines--;
		DistrMin(num);
	}

if(neg<=0)
	return;

TRPROD2(printf("product:\tRemoving Industry\n"));
for(;neg>0 && fleet[shp].ind!=0;) {
		neg-=10;
		fleet[shp].ind--;
		galaxy[num].ind++;
		galaxy[num].income+=5;
	}

if(neg<=0)
	return;

TRPROD2(printf("product:\tRemoving PDUs\n"));
for(;neg>0 && fleet[shp].pdu!=0;) {
		neg-=2;
		fleet[shp].pdu--;
		galaxy[num].pdu++;
	}

if(neg<=0)
	return;

TRPROD2(printf("product:\tRemoving Spacemines\n"));
for(;neg>0 && fleet[shp].spacemines!=0;) {
		neg-=1;
		fleet[shp].spacemines--;
		galaxy[num].deployed++;
	}

if(neg<=0) 
	return;

TRPROD2(printf("product:\tRemoving Ore\n"));
for(x=NEUTPLR;neg>0 && x>=0;)  {
	if(fleet[shp].ore[x]!=0) {
		TRPROD2(printf("product:\tType %d\n",x));
		neg--;
		fleet[shp].ore[x]--;
		galaxy[num].ore[x]++;
		}
	else x--;
	}
}

/*****************************************************************************/
void DistrMin(Planet plan)
/*****************************************************************************/
/* Put a mine on a planet from ship cargo destruction */
/* If there are no mines there, destroy it */
{
Oretype z;
int tot=0;

TRPROD(printf("product:DistrMin(plan: %d)\n",plan));

for(z=0;z<10;z++)
	tot+=galaxy[plan].mine[z];

if(tot==0)
	return;

for(;;) {
	z=rand()%10;
	if(galaxy[plan].mine[z]!=0) {
		TRPROD(printf("DistrMin:Putting down mine type %d\n",z));
		galaxy[plan].mine[z]++;
		galaxy[plan].income++;
		return;
		}
	}
return;
}

/*****************************************************************************/
void CalcPrices(void)
/*****************************************************************************/
/* Calculate the new price for ore on Earth */
{
Oretype count;
Planet earth;
int tmpprice;

TRPROD(printf("product:CalcPrices\n"));
earth=LocateEarth();
if((galaxy[earth].indleft*10)<(galaxy[earth].ind*9)) {
	galaxy[earth].ind++;
	galaxy[earth].income+=5;
	}
for(count=0;count<10;count++) {
	tmpprice=33-(galaxy[earth].ore[count]*3)/10;
	tmpprice+=traded[count][BOUGHT]/10;
	tmpprice-=traded[count][SOLD]/10;
	if(tmpprice>price[count]) {
		price[count]+=(10*(tmpprice-price[count]))/(turn+1);
		}
	else if(tmpprice<price[count]) {
		price[count]-=(10*(price[count]-tmpprice))/(turn+1);
		}
	if(traded[count][BOUGHT]>0) {
		galaxy[earth].mine[count]++;
		galaxy[earth].income++;
		}
	galaxy[earth].ore[count]+=galaxy[earth].mine[count];
	if(galaxy[earth].ore[count]>100) 
		galaxy[earth].ore[count]-=2*(galaxy[earth].ore[count]-100);
	if(price[count]<1)
		price[count]=1;
	}
return;
}

/*****************************************************************************/
void CheckWinning(void)
/*****************************************************************************/
{
Planet earth;
int x,numplans[NUMPLAYERS+1];

TRPROD(printf("CheckWinning()\n"));
earth=LocateEarth();

/* Check for turn number */
if(gamedet.winning&WTURN) {
	if(!(gamedet.winning&WFIXTURN))
		CalcEndTurn();
	if(turn>=gamedet.turn)
		fprintf(stderr,"GAME OVER: Number of turns has been acheived\n");
	}
/* Check for ownership of Earth */
if(gamedet.winning&WEARTH) {
	if(galaxy[earth].owner!=NEUTPLR) {
		fprintf(stderr,"GAME OVER: Earth has been taken by player %d=%s\n",galaxy[earth].owner,name[galaxy[earth].owner]);
		}
	}

/* Check for having high enough score */
if(gamedet.winning&WSCORE) {
	for(x=1;x<NUMPLAYERS;x++)
		if(score[x]>=gamedet.score)
			fprintf(stderr,"GAME OVER: Score %d achieved by player %d=%s\n",score[x],x,name[x]);
	}

/* Check for having high enough income */
if(gamedet.winning&WINCOME) {
	for(x=1;x<NUMPLAYERS;x++)
		if(CalcPlrInc(x)>=gamedet.income)
			fprintf(stderr,"GAME OVER: Income %d achieved by player %d=%s\n",CalcPlrInc(x),x,name[x]);
	}

/* Check for having high enough credits */
if(gamedet.winning&WCREDIT) {
	for(x=1;x<NUMPLAYERS;x++)
		if(ecredit[x]>=gamedet.credits)
			fprintf(stderr,"GAME OVER: Credits %d achieved by player %d=%s\n",ecredit[x],x,name[x]);
	}

/* Check for having enough planets */
if(gamedet.winning&WPLANETS) {
	for(x=1;x<NUMPLAYERS+1;x++) 	/* Clear ownership array */
		numplans[x]=0;
/* Work out how many planets each player owns */
	for(x=0;x<NUMPLANETS;x++)		/* Fill ownership array */
		numplans[galaxy[x].owner]++;
	for(x=1;x<NUMPLAYERS;x++)
		if(numplans[x]>=gamedet.planets)
			fprintf(stderr,"GAME OVER: %d planets owned by player %d=%s\n",numplans[x],x,name[x]);
	}
}

/*****************************************************************************/
void CalcEndTurn(void)
/*****************************************************************************/
/* Work out what turn is the last turn */
{
int a,sum,avg;
TRPROD(printf("CalcEndTurn()\n"));

sum=gamedet.turn*2;
for(a=1;a<NUMPLAYERS;a++)
	sum+=desturn[a];
avg=sum/(NUMPLAYERS+2);
gamedet.turn=avg;
}
