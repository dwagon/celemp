/* JUMP.C for Celestial Empire by Dougal Scott */
/* This deals with all the ship movement stuff */
/* (c) 2016 Dougal Scott */

#include "def.h"

#define TRJUM(x)	if(strstr(dbgstr,"JUMP") || strstr(dbgstr,"jump")) x

extern FILE *trns[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern Player plr;
extern char *dbgstr;
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
int effuel[5][6]={				/* Fuel efficiency matrix */
	{2,		1, 	1, 	1, 	1, 	1},
	{8,		4, 	2, 	2, 	2, 	2},
	{18,	9, 	6, 	3, 	3, 	3},
	{32,	16,	12,	8, 	4, 	4},
	{50,	25,	20,	15,	10,	5}	};

/*****************************************************************************/
int ChekFuel(Ship shp,Number dist)
/*****************************************************************************/
/* Check that the ship has enough fuel to jump the required distance */
{
TRJUM(printf("jump:ChekFuel(shp: %d,dist: %d)\n",shp,dist));

if(EffEff(shp)<-1) {
	TRJUM(printf(" Too Big\n"));
	return(TOOBIG);
	}
if(fleet[shp].ore[0]<effuel[dist-1][EffEff(shp)+1]) {
	TRJUM(printf(" No Fuel (R0=%d<FuelUse[Eff=%d,Dist=%d]=%d)\n",fleet[shp].ore[0],EffEff(shp),dist,effuel[dist-1][EffEff(shp)+1]));
	return(NOFUEL);
	}
TRJUM(printf(" Enough Fuel\n"));
return(ENUFUEL);
}

/*****************************************************************************/
int ChekPath(Ship shp,Planet dest)
/*****************************************************************************/
/* Check to see if the destination is valid */
{
Number count;

TRJUM(printf("Checking path for ship %d to planet %d\n",shp,dest));

for(count=0;count<4;count++)
	if(galaxy[fleet[shp].planet].link[count]==dest)
		return(1);
return(0);
}

/*****************************************************************************/
void MoveTo(Ship shp, Planet pln, Flag dist, int segm)
/*****************************************************************************/
/* Move ship to planet and process all attacks on the ship and any ship that
 * it might be towing.
 */
{
    Ship towee;     /* What ship is being towed if any */

    fleet[shp].planet = pln;		/* Move ship to new planet */
    galaxy[pln].knows[fleet[shp].owner] = 1;
    if(galaxy[pln].deployed != 0) { /* If any spacemines on planet, attack ship */
        SpcminShp(shp);
    }
    if(dist > 1 && segm < dist)	{ /* If we jump thru this planet check for pdu attacks */
        if(galaxy[pln].owner != fleet[shp].owner && alliance[galaxy[pln].owner][fleet[shp].owner] <= NEUTRAL) {
            PduShp(pln, shp, galaxy[pln].pdu);
        }
    }

    fleet[shp].moved = 1;
    if(fleet[shp].engage < 0) {
        towee = -fleet[shp].engage-1;	/* What ship is being towed */
        fleet[towee].planet = pln;		/* Move towed ship */
        if(galaxy[pln].deployed != 0) {		/* Attack towed ship with spacemines */
            SpcminShp(towee);
        }
        if(dist > 1 && segm < dist)	{		/* Attack towed ship with PDUs */
            if(galaxy[pln].owner != fleet[towee].owner && alliance[galaxy[pln].owner][fleet[towee].owner] <= NEUTRAL) {
    /* Don't attack empty ships, this could give the owner of the hulk information
     * about where the ship was towed to, that they should not have. Thanks to
     * jasoncc@deakin.edu.au for pointing this out */
                if(!IsEmpty(towee))	{
                    PduShp(pln, towee, galaxy[pln].pdu);
                }
            }
        }
    }
}

/*****************************************************************************/
void Jump1(Ship shp,Planet dest1)
/*****************************************************************************/
/* Process jump of length 1     */
{
TRJUM(printf("Ship %d jumping to %d\n",shp,dest1));

fprintf(trns[plr],"S%dJ%d\t",shp+100,dest1+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if(fleet[shp].moved!=0) {
	fprintf(trns[plr],"Ship %d has already moved or fired\n",shp+100);
	return;
	}
if(fleet[shp].engage>0) {
	fprintf(trns[plr],"Ship %d is engaged by tractor beams\n",shp+100);
	return;
	}
switch(ChekFuel(shp,1)) {
	case TOOBIG:
		fprintf(trns[plr],"Ship %d is too big to jump\n",shp+100);
		return;
		break;
	case NOFUEL:
		fprintf(trns[plr],"Ship %d doesn't have enough fuel\n",shp+100);
		return;
		break;
	case ENUFUEL:
		break;
	default: fprintf(stderr,"SNAFU: ChekFuel returned unknown type\n");
		return;
	}
if(!ChekPath(shp,dest1)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest1+100);
	return;
	}
fleet[shp].ore[0]-=effuel[0][EffEff(shp)+1];
fleet[shp].cargleft+=effuel[0][EffEff(shp)+1];
MoveTo(shp,dest1,1,1);
fprintf(trns[plr],"S%dJ%d\n",shp+100,dest1+100);
}

/*****************************************************************************/
void Jump2(Ship shp,Planet dest1,Planet dest2)
/*****************************************************************************/
/* Process jump of length 2     */
{
TRJUM(printf("ship %d jumping to %d and %d\n",shp,dest1,dest2));

fprintf(trns[plr],"S%dJ%dJ%d\t",shp+100,dest1+100,dest2+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if(fleet[shp].engage>0) {
	fprintf(trns[plr],"Ship %d is engaged by tractor beams\n",shp+100);
	return;
	}
if(fleet[shp].moved!=0) {
	fprintf(trns[plr],"Ship %d has already moved or fired\n",shp+100);
	return;
	}
switch(ChekFuel(shp,2)) {
	case TOOBIG:
		fprintf(trns[plr],"Ship %d is too big to jump\n",shp+100);
		return;
		break;
	case NOFUEL:
		fprintf(trns[plr],"Ship %d doesn't have enough fuel\n",shp+100);
		return;
		break;
	case ENUFUEL:
		break;
	default: fprintf(stderr,"SNAFU: ChekFuel returned unknown type\n");
		return;
	}
if(!ChekPath(shp,dest1)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest1+100);
	return;
	}
fleet[shp].ore[0]-=effuel[1][EffEff(shp)+1];
fleet[shp].cargleft+=effuel[1][EffEff(shp)+1];
MoveTo(shp,dest1,2,1);
if(!ChekPath(shp,dest2)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest2+100);
	fprintf(trns[plr],"S%dJ%d\n",shp+100,dest1+100);
	return;
	}
MoveTo(shp,dest2,2,2);
fprintf(trns[plr],"S%dJ%dJ%d\n",shp+100,dest1+100,dest2+100);
}

/*****************************************************************************/
void Jump3(Ship shp,Planet dest1,Planet dest2,Planet dest3)
/*****************************************************************************/
/* Process jump of length 3     */
{
TRJUM(printf("Ship %d jumping to %d, %d and %d\n",shp,dest1,dest2,dest3));

fprintf(trns[plr],"S%dJ%dJ%dJ%d\t",shp+100,dest1+100,dest2+100,dest3+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if(fleet[shp].engage>0) {
	fprintf(trns[plr],"Ship %d is engaged by tractor beams\n",shp+100);
	return;
	}
if(fleet[shp].moved!=0) {
	fprintf(trns[plr],"Ship %d has already moved or fired\n",shp+100);
	return;
	}
switch(ChekFuel(shp,3)) {
	case TOOBIG:
		fprintf(trns[plr],"Ship %d is too big to jump\n",shp+100);
		return;
		break;
	case NOFUEL:
		fprintf(trns[plr],"Ship %d doesn't have enough fuel\n",shp+100);
		return;
		break;
	case ENUFUEL:
		break;
	default: fprintf(stderr,"SNAFU: ChekFuel returned unknown type\n");
		return;
	}
if(!ChekPath(shp,dest1)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest1+100);
	return;
	}
fleet[shp].ore[0]-=effuel[2][EffEff(shp)+1];
fleet[shp].cargleft+=effuel[2][EffEff(shp)+1];

MoveTo(shp,dest1,3,1);					/* Jump 1 */
if(!ChekPath(shp,dest2)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest2+100);
	fprintf(trns[plr],"S%dJ%d\n",shp+100,dest1+100);
	return;
	}
MoveTo(shp,dest2,3,2);					/* Jump 2 */
if(!ChekPath(shp,dest3)) { 
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest3+100);
	fprintf(trns[plr],"S%dJ%dJ%d\n",shp+100,dest1+100,dest2+100);
	return;
	}
MoveTo(shp,dest3,3,3);					/* Jump 3 */
fprintf(trns[plr],"S%dJ%dJ%dJ%d\n",shp+100,dest1+100,dest2+100,dest3+100);
}

/*****************************************************************************/
void Jump4(Ship shp,Planet dest1,Planet dest2,Planet dest3,Planet dest4)
/*****************************************************************************/
/* Process jump of length 4     */
{
TRJUM(printf("Ship %d jumping to %d, %d, %d and %d\n",shp,dest1,dest2,dest3,dest4));

fprintf(trns[plr],"S%dJ%dJ%dJ%dJ%d\t",shp+100,dest1+100,dest2+100,dest3+100,dest4+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if(fleet[shp].moved!=0) {
	fprintf(trns[plr],"Ship %d has already moved or fired\n",shp+100);
	return;
	}
if(fleet[shp].engage>0) {
	fprintf(trns[plr],"Ship %d is engaged by tractor beams\n",shp+100);
	return;
	}
switch(ChekFuel(shp,4)) {
	case TOOBIG:
		fprintf(trns[plr],"Ship %d is too big to jump\n",shp+100);
		return;
		break;
	case NOFUEL:
		fprintf(trns[plr],"Ship %d doesn't have enough fuel\n",shp+100);
		return;
		break;
	case ENUFUEL:
		break;
	default: fprintf(stderr,"SNAFU: ChekFuel returned unknown type\n");
		return;
	}
if(!ChekPath(shp,dest1)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest1+100);
	return;
	}
fleet[shp].ore[0]-=effuel[3][EffEff(shp)+1];
fleet[shp].cargleft+=effuel[3][EffEff(shp)+1];
MoveTo(shp,dest1,4,1);					/* Jump 1 */
if(!ChekPath(shp,dest2)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest2+100);
	fprintf(trns[plr],"S%dJ%d\n",shp+100,dest1+100);
	return;
	}
MoveTo(shp,dest2,4,2);					/* Jump 2 */
if(!ChekPath(shp,dest3)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest3+100);
	fprintf(trns[plr],"S%dJ%dJ%d\n",shp+100,dest1+100,dest2+100);
	return;
	}
MoveTo(shp,dest3,4,3);					/* Jump 3 */
if(!ChekPath(shp,dest4)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest4+100);
	fprintf(trns[plr],"S%dJ%dJ%dJ%d\n",shp+100,dest1+100,dest2+100,dest3+100);
	return;
	}
MoveTo(shp,dest4,4,4);					/* Jump 4 */
fprintf(trns[plr],"S%dJ%dJ%dJ%dJ%d\n",shp+100,dest1+100,dest2+100,dest3+100,dest4+100);
}

/*****************************************************************************/
void Jump5(Ship shp,Planet dest1,Planet dest2,Planet dest3,Planet dest4,Planet
dest5)
/*****************************************************************************/
/* Process jump of length 5     */
{
TRJUM(printf("Ship %d jumping to %d, %d, %d, %d and %d\n",shp,dest1,dest2,dest3,dest4,dest5));

fprintf(trns[plr],"S%dJ%dJ%dJ%dJ%dJ%d\t",shp+100,dest1+100,dest2+100,dest3+100,dest4+100,dest5+100);
if(fleet[shp].owner!=plr) {
	fprintf(trns[plr],"You do not own ship %d\n",shp+100);
	return;
	}
if(fleet[shp].engage>0) {
	fprintf(trns[plr],"Ship %d is engaged by tractor beams\n",shp+100);
	return;
	}
if(fleet[shp].moved!=0) {
	fprintf(trns[plr],"Ship %d has already moved or fired\n",shp+100);
	return;
	}
switch(ChekFuel(shp,5)) {
	case TOOBIG:
		fprintf(trns[plr],"Ship %d is too big to jump\n",shp+100);
		return;
		break;
	case NOFUEL:
		fprintf(trns[plr],"Ship %d doesn't have enough fuel\n",shp+100);
		return;
		break;
	case ENUFUEL:
		break;
	default: fprintf(stderr,"SNAFU: ChekFuel returned unknown type\n");
		return;
	}
if(!ChekPath(shp,dest1)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest1+100);
	return;
	}
fleet[shp].ore[0]-=effuel[4][EffEff(shp)+1];
fleet[shp].cargleft+=effuel[4][EffEff(shp)+1];
MoveTo(shp,dest1,5,1);					/* Jump 1 */
if(!ChekPath(shp,dest2)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\n",shp+100,dest2+100);
	fprintf(trns[plr],"S%dJ%d\n",shp+100,dest1+100);
	return;
	}
MoveTo(shp,dest2,5,2);					/* Jump 2 */
if(!ChekPath(shp,dest3)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest3+100);
	fprintf(trns[plr],"S%dJ%dJ%d\n",shp+100,dest1+100,dest2+100);
	return;
	}
MoveTo(shp,dest3,5,3);					/* Jump 3 */
if(!ChekPath(shp,dest4)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest4+100);
	fprintf(trns[plr],"S%dJ%dJ%dJ%d\n",shp+100,dest1+100,dest2+100,dest3+100);
	return;
	}
MoveTo(shp,dest4,5,4);					/* Jump 4 */
if(!ChekPath(shp,dest5)) {
	fprintf(trns[plr],"Ship %d cannot jump to %d\t",shp+100,dest5+100);
	fprintf(trns[plr],"S%dJ%dJ%dJ%dJ%d\n",shp+100,dest1+100,dest2+100,dest3+100,dest4+100);
	return;
	}
MoveTo(shp,dest5,5,5);					/* Jump 5 */
fprintf(trns[plr],"S%dJ%dJ%dJ%dJ%dJ%d\n",shp+100,dest1+100,dest2+100,dest3+100,dest4+100,dest5+100);
}
