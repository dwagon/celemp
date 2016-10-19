/* Fix the data structure of any misdemeanors */

#include "def.h"

FILE *trns[NUMPLAYERS+1];
Number score[NUMPLAYERS+1];
Number price[10];
char name[NUMPLAYERS+1][10];
game gamedet;
game ogamedet;
char *game_path;
Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
Number ecredit[NUMPLAYERS+1];
char *dbgstr;
planet galaxy[NUMPLANETS];
Ship shiptr;
ship fleet[NUMSHIPS];
int turn,gm;

int main(int argc,char **argv)
{
char *gmstr;
int count;

if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
	strcpy(dbgstr,"null");

if((game_path = getenv("CELEMPPATH")) == NULL) {
    fprintf(stderr,"set CELEMPPATH to the appropriate directory\n");
    exit(-1);
    }

if(argc>=2)
    gm=atoi(argv[1]);
else {
    if((gmstr = getenv("CELEMPGAME")) == NULL) {
        fprintf(stderr,"set CELEMPGAME to the appropriate game number\n");
        exit(-1);
        }
    gm=atoi(gmstr);
    }

printf("Reading galaxy...\n");
ReadGalflt();
printf("Fixing game structure...\n");
gamedet.winning=ogamedet.winning;
gamedet.score=ogamedet.score;
gamedet.planets=ogamedet.planets;

gamedet.home.ind=ogamedet.home.ind;
for(count=0;count<10;count++) {
	gamedet.home.ore[count]=ogamedet.home.ore[count];
	gamedet.home.mine[count]=ogamedet.home.mine[count];
	}
gamedet.home.pdu=ogamedet.home.pdu;
gamedet.home.spacemine=ogamedet.home.spacemine;
gamedet.home.deployed=ogamedet.home.deployed;

gamedet.earth.amnesty= ogamedet.earth.amnesty;
gamedet.earth.earthmult= ogamedet.earth.earthmult;
gamedet.earth.fbid= ogamedet.earth.fbid;
gamedet.earth.cbid= ogamedet.earth.cbid;
gamedet.earth.sbid= ogamedet.earth.sbid;
gamedet.earth.tbid= ogamedet.earth.tbid;
gamedet.earth.ind= ogamedet.earth.ind;
for(count=0;count<10;count++) {
	gamedet.earth.ore[count]=ogamedet.earth.ore[count];
	gamedet.earth.mine[count]=ogamedet.earth.mine[count];
	}
gamedet.earth.pdu= ogamedet.earth.pdu;
gamedet.earth.spacemine= ogamedet.earth.spacemine;
gamedet.earth.deployed= ogamedet.earth.deployed;

gamedet.gal.nomine= ogamedet.gal.nomine;
gamedet.gal.extramine= ogamedet.gal.extramine;
gamedet.gal.extraore= ogamedet.gal.extraore;
gamedet.gal.hasind= ogamedet.gal.hasind;
gamedet.gal.haspdu= ogamedet.gal.haspdu;

gamedet.ship.num= ogamedet.ship.num;
gamedet.ship.fight= ogamedet.ship.fight;
gamedet.ship.cargo= ogamedet.ship.cargo;
gamedet.ship.shield= ogamedet.ship.shield;
gamedet.ship.tractor= ogamedet.ship.tractor;
gamedet.ship.eff= ogamedet.ship.eff;

gamedet.income=ogamedet.income;

printf("Writing galaxy...\n");
WriteGalflt();
return(0);
}
