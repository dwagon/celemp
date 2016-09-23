/****************************************************************/
/* Celestial Empire by Zer Dwagon    TURN.C 					*/
/* Takes data structure as input and outputs all the players    */
/* turn sheets  - Copyright (c) 2016 Dougal Scott               */ 
/****************************************************************/

#include "def.h"
#include "turn.h"
#include "typname.h"
#include "allname.h"

FILE *trns[NUMPLAYERS+1];
planet galaxy[NUMPLANETS];
ship fleet[NUMSHIPS];
Number ecredit[NUMPLAYERS+1];
game gamedet;
Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
int turn,gm;
Ship shiptr;
int score[NUMPLAYERS+1];
char name[NUMPLAYERS+1][10];
char duedate[15];
int price[10];
char *dbgstr;
int earth;
int desturn[NUMPLAYERS+1];
int plrflag[NUMPLAYERS+1]={
	PLR0,
	PLR1,	PLR2,	PLR3,
	PLR4,	PLR5,	PLR6,
	PLR7,	PLR8,	PLR9	};
char *path;

/*****************************************************************************/
int main(int argc,char **argv)
/*****************************************************************************/
/* Do the lot */
{
Player plr;
char *gmstr;

if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
	dbgstr=(char *)"null";

if((path = getenv("CELEMPPATH")) == NULL) {
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

if(ReadGalflt()== -1) {
	fprintf(stderr,"Program terminated\n");
	exit(-1);
	}
printf("Celestial Empire Version %d.%d Turn sheet creation program\n",VERSION,PATCHLEVEL);
earth=LocateEarth();
if(argc==3) {
	if(PrintGalaxy(argv[2])==-1)
		fprintf(stderr,"Galaxy listing aborted due to file error\n");
	}
else {
	Init();
	for(plr=1;plr<NUMPLAYERS+1;plr++) {
		TRTUR(printf("Processing player%d\n",plr));
		Process(plr);
		}
	}
return(0);
}

/*****************************************************************************/
int PrintGalaxy(char *fname)
/*****************************************************************************/
/* Print the details of the entire galaxy */
{
FILE *gal;
int count;
char filename[128];
char str[128];

TRTUR(printf("PrintGalaxy\n"));
sprintf(filename,"%s",fname);
if((gal=fopen(filename,"w"))==NULL) {
	printf("Could not open %s for writing\n",filename);
	return(-1);
	}

sprintf(str,"                         CELESTIAL EMPIRE %d.%d\n",VERSION,PATCHLEVEL);
Prnt(str,gal);
Prnt("                                  by\n",gal);
Prnt("                             Zer Dwagon\n",gal);
sprintf(str,"\nGame:%d\tTurn:%d\n",gm,turn);
Prnt(str,gal);
TypeSummary(gal);
OwnerSummary(gal);
UnitSummary(gal);
WinningDetails(gal);
CostDetails(gal);
PlanetSummary(NEUTPLR,gal);
for(count=0;count<shiptr;count++)
	ChekShip(count,NEUTPLR,gal);
for(count=0;count<NUMPLANETS;count++) {
	DoPlanet(count,NEUTPLR,gal);
	Prnt("------------------------------------------------------------\n",gal);
	}
fclose(gal);
return(0);
}

/*****************************************************************************/
void Init(void)
/*****************************************************************************/
/* Ask for various details */
{
    TRTUR(printf("Init\n"));

    printf("Enter due date:");
    fgets(duedate, sizeof(duedate), stdin);
}

/*****************************************************************************/
void Prnt(const char *string,FILE *stream)
/*****************************************************************************/
/* Print string to disk file */
{
fprintf(stream,"%s",string);
}

/*****************************************************************************/
void Process(Player plr)
/*****************************************************************************/
/* Process each players turn */
{
char str[128];
FILE *outfile;
char filename[128];
int count;

TRTUR(printf("Process(plr:%d)\n",plr));

/* Open file for outputing information */
sprintf(filename,"%s%d/turn%d.%d",path,gm,turn,plr);
if((outfile=fopen(filename,"w"))==NULL) {
	fprintf(stderr,"Could not open %s for writing\n",filename);
	return;
	}

/*******  HEADINGS ***************************************************/
Headings(outfile,plr);
/*************** RESEARCH TURN WARNING *****************************/
if((turn-10)%4==0 && turn>9)
	ResOutput(plr,outfile);
if(((turn-10)%4==3 && turn>=9) || turn==9) {
	Prnt("        Next turn is a research turn (Not this one)!\n",outfile);	
	}

/*************** GLOBAL SHIP TYPE SUMMARY *****************************/
if(turn%2==0)
	TypeSummary(outfile);
/*************** SHIP SUMMARY ****************************************/
TRTUR(printf("Process:Ship Summary\n"));
Prnt("\nSUMMARY OF SHIPS\n",outfile);
for(count=0;count<shiptr;count++)
	ChekShip(count,plr,outfile);
/*************** PLANET SUMMARY ***********************************/
PlanetSummary(plr,outfile);
/*********** ALLIANCE STATUS **********************************/
Prnt("\nALLY STATUS\nEmpire Name         You are     They are\n",outfile);
for(count=1;count<NUMPLAYERS+1;count++) 
	if(count!=plr && (alliance[plr][count]!=NEUTRAL || alliance[count][plr]!=NEUTRAL)) {
		sprintf(str,"%-11s          : %-9s : %-9s\n",name[count],allname[alliance[plr][count]],allname[alliance[count][plr]]);
		Prnt(str,outfile);
		}
if(alliance[plr][plr]==ENEMY) {
	sprintf(str,"%-11s          :           : %-9s\n",name[count],allname[alliance[plr][plr]]);
	Prnt(str,outfile);
	}
/******** EARTH DETAILS ***************************************/
TRTUR(printf("Process:Earth Details\n"));
sprintf(str,"\n %d  ******** EARTH ********\n",earth+100);
Prnt(str,outfile);
if(turn<gamedet.earth.amnesty)
	Prnt("Earth amnesty in effect. No shots allowed.\n",outfile);
if(gamedet.earth.flag&WBUYALLORE)
	Prnt("Unlimited Selling",outfile);
if(gamedet.earth.flag&WBUY100ORE)
	Prnt("Limited Selling",outfile);
sprintf(str,"\nNEARBY PLANETS %d %d %d\n",galaxy[earth].link[0]+100,galaxy[earth].link[1]+100,galaxy[earth].link[2]+100);
Prnt(str,outfile);
Prnt("                   EARTH   Trading Prices\n",outfile);
Prnt("type     ",outfile);
for(count=0;count<10;count++) {
	sprintf(str,"    %d",count);
	Prnt(str,outfile);
	}
Prnt("\n",outfile);
Prnt("price    ",outfile);
for(count=0;count<10;count++) {
	sprintf(str,"%5d",price[count]);
	Prnt(str,outfile);
	}
Prnt("\n",outfile);
Prnt("amount   ",outfile);
for(count=0;count<10;count++) {
	sprintf(str,"%5d",galaxy[earth].ore[count]);
	Prnt(str,outfile);
	}
Prnt("\n",outfile);
DoShip(plr,earth,outfile);
Prnt("\n-----------------------------------------------------------\n",outfile);
/******** PLANET DETAILS **************************************/
TRTUR(printf("******** PLANET DETAILS *******\n"));
for(count=0;count<NUMPLANETS;count++)
	if(Interest(plr,count)==1) {
		DoPlanet(count,plr,outfile);
		Prnt("-----------------------------------------------------------\n",outfile);
		}

/******* TAIL NOTES ******************************************/
	CatMotd(outfile);
	CatSpec(outfile,plr);
	CatExhist(outfile,plr);
	if(((turn-11)%4==0) && (turn>10))
		ListRes(outfile);

fclose(outfile);
}

/*****************************************************************************/
void CatExhist(FILE *stream,Player plyr)
/*****************************************************************************/
/* Cat the execution history into the turn sheet */
{
FILE *exechist;
char filename[128];
char strng[128];

TRTUR(printf("CatExhist(stream,plyr:%d)\n",plyr));
sprintf(filename,"%s%d/exhist.%d",path,gm,plyr);
if((exechist = fopen(filename,"r")) == NULL) {
	fprintf(stderr,"CatExhist:Could not open exhist file:%s\n",filename);
	return;
	}
Prnt("\nCOMMAND HISTORY\n",stream);
for(;fgets(strng,256,exechist)!=NULL;) {
	fputs(strng,stream);
	}
fclose(exechist);
}

/*****************************************************************************/
void ListRes(FILE *stream)
/*****************************************************************************/
/* Give details of the research planets at the end of every research turn */
{
int count,count2;
char strng[128];

TRTUR(printf("ListRes(stream)\n"));
Prnt("\n\nSUMMARY OF RESEARCH PLANETS.\n",stream);
for(count=0;count<NUMPLANETS;count++) 
	if(IsResearch(count) && galaxy[count].owner!=NEUTPLR) {
		sprintf(strng,"%-3d: ",count+100);
		Prnt(strng,stream);
		for(count2=0;count2<4;count2++) {
			if(galaxy[count].link[count2]>=0) {
				sprintf(strng,"%-3d ",galaxy[count].link[count2]+100);
				Prnt(strng,stream);
				}
			else
				Prnt("    ",stream);
			}
			sprintf(strng,"%-20s %-10s\n",galaxy[count].name,name[galaxy[count].owner]);
			Prnt(strng,stream);
		}
return;
}

/*****************************************************************************/
void CatMotd(FILE *stream)
/*****************************************************************************/
/* Cat the messages of the day to the end of the file */
{
FILE *motd;
char filename[128];
char strng[128];

TRTUR(printf("CatMotd(stream)\n"));
sprintf(filename,"%s%d/motd",path,gm);
if((motd = fopen(filename,"r")) == NULL) {
	TRTUR(fprintf(stderr,"CatMotd:Could not open motd file:%s\n",filename));
	return;
	}
Prnt("\nGENERAL MESSAGES\n",stream);
for(;fgets(strng,80,motd)!=NULL;) {
	fputs(strng,stream);
	}
fputs("\n",stream);
fclose(motd);
return;
}

/*****************************************************************************/
void CatSpec(FILE *stream,Player plr)
/*****************************************************************************/
/* Cat any special messages to that player to the end of file */
{
FILE *spec;
char strng[256];
char filename[128];

TRTUR(printf("CatSpec(stream,plr:%d)\n",plr));
sprintf(filename,"%s%d/spec.%d",path,gm,plr);
if((spec=fopen(filename,"r"))==NULL) {
	TRTUR(fprintf(stderr,"CatSpec:Could not open spec file:%s\n",filename));
	return;
	}
Prnt("\nPERSONAL MESSAGES\n",stream);
printf("Reading player %d's message\n",plr);
for(;fgets(strng,256,spec)!=NULL;) {
	fputs(strng,stream);
	}
fputs("\n",stream);
fclose(spec);
return;
}

/*****************************************************************************/
int Interest(Player plr,Planet plan)
/*****************************************************************************/
/* Determine if the player has as interest in the planet, either has a ship
	over it or owns it, or scanned it that turn */
{
int r;

TRTUR2(printf("Interest(plr:%d, plan:%d)\n",plr,plan));
/* Check for ownership */
if(galaxy[plan].owner==plr)
	return(1);
if(galaxy[plan].scanned && galaxy[plan].scanned & plrflag[plr])
	return(1);
for(r=0;r<shiptr;r++)
	if(fleet[r].planet==plan && fleet[r].owner==plr)
		return(1);
return(0);
}

/*****************************************************************************/
void DoPlanet(Planet plan,Player plr,FILE *stream)
/*****************************************************************************/
/* Do the main part of the display */
{
char str[128];
int count;

TRTUR2(printf("DoPlanet(plan:%d, plr:%d)\n",plan,plr));
if(galaxy[plan].scanned!=0) {
	Prnt("              *******  PLANET SCANNED THIS TURN *******\n",stream);
	}
if(IsResearch(plan)) {
	sprintf(str,"                  ******* RESEARCH PLANET *******\n");
	Prnt(str,stream);
	}
TRTUR(printf("plan:%d\t owner:%d\n",plan,galaxy[plan].owner));
sprintf(str,"%3d %-9s:%-25s\n",plan+100,name[galaxy[plan].owner],galaxy[plan].name);
Prnt(str,stream);
Prnt("Nearby Planets   ",stream);
for(count=0;count<4;count++) 
	if(galaxy[plan].link[count]>=0) {
		sprintf(str,"%-4d ",galaxy[plan].link[count]+100);
		Prnt(str,stream);
	} else {
		Prnt("     ",stream);
		}
sprintf(str,"Industry= %-3d  PDU= %d(%d) Income= %-5d\n",galaxy[plan].ind,galaxy[plan].pdu,Pdus(galaxy[plan].pdu),galaxy[plan].income);
Prnt(str,stream);
sprintf(str,"Spacemines: Stored= %-5d Deployed= %-5d\n",galaxy[plan].spacemine,galaxy[plan].deployed);
Prnt(str,stream);
if(galaxy[plan].stndord[0]==0)
	sprintf(str,"Standing Order:None\n");
else 
	sprintf(str,"Standing Order:%d%s\n",plan+100,galaxy[plan].stndord);
Prnt(str,stream);
Prnt("     Mine Type              ",stream);
for(count=0;count<10;count++)
	if(galaxy[plan].mine[count]!=0 || galaxy[plan].ore[count]!=0) {
		sprintf(str,"%-5d",count);
		Prnt(str,stream);
		}
Prnt("\n     Amount stored          ",stream);
for(count=0;count<10;count++)
	if(galaxy[plan].mine[count]!=0 || galaxy[plan].ore[count]!=0) {
		sprintf(str,"%-5d",galaxy[plan].ore[count]);
		Prnt(str,stream);
		}
Prnt("\n     Production             ",stream);
for(count=0;count<10;count++)
	if(galaxy[plan].mine[count]!=0 || galaxy[plan].ore[count]!=0) {
		sprintf(str,"%-5d",galaxy[plan].mine[count]);
		Prnt(str,stream);
		}
Prnt("\n\n",stream);
DoShip(plr,plan,stream);
return;
}

/*****************************************************************************/
void DoShip(Player plr,Planet plan,FILE *stream)
/*****************************************************************************/
/* Display the ships above a certain planet */
{
    int count;

    TRTUR(printf("DoShip(plr:%d plan:%d)\n",plr,plan));
    for(count=0;count<shiptr;count++) {
        if(fleet[count].planet==plan) {
            if(fleet[count].owner==plr || plr==NEUTPLR || alliance[fleet[count].owner][plr]==ALLY) {
                DoFriend(count,stream);
            }
            else {
                DoEnemy(count,stream);
            }
        }
    }
    return;
}

/*****************************************************************************/
void DoEnemy(Ship shp,FILE *stream)
/*****************************************************************************/
/* Print out details of enemy ships */
{
char str[128];

TRTUR(printf("DoEnemy(ship:%d)\n",shp));
Prnt("- - - - - - - - - - - - - - - - - - - -\n",stream);
sprintf(str,"S%3d %-9s: %-24s: %s\n",shp+100,name[fleet[shp].owner],fleet[shp].name,stypes[fleet[shp].type]);
Prnt(str,stream);
TRTUR(printf("Finished DoEnemy()\n"));
return;
}

/*****************************************************************************/
void DoFriend(Ship shp,FILE *stream)
/*****************************************************************************/
/* Print details of friendly ships */
{
int count;
char str[128];

TRTUR(printf("DoFriend(ship:%d)\n",shp));
Prnt("- - - - - - - - - - - - - - - - - - - -\n",stream);
sprintf(str,"S%-3d %-9s: f=%-3d c=%-3d(%-3d) t=%-3d s=%d(%d)",shp+100,name[fleet[shp].owner],fleet[shp].fight,fleet[shp].cargo,fleet[shp].cargleft,fleet[shp].tractor,fleet[shp].shield,Shields(shp));
Prnt(str,stream);
sprintf(str," eff=%-1d(%d) shots=%d",fleet[shp].efficiency,EffEff(shp),Shots(shp,fleet[shp].fight));
Prnt(str,stream);
sprintf(str," %-24s\n",fleet[shp].name);
Prnt(str,stream);
if(fleet[shp].stndord[0]==0)
	sprintf(str,"          %-15s Standing Order:None\n",stypes[fleet[shp].type]);
else
	sprintf(str,"          %-15s Standing Order:S%d%s\n",stypes[fleet[shp].type],shp+100,fleet[shp].stndord);
Prnt(str,stream);
/* Print out cargo details */
Prnt("          ",stream);
if(fleet[shp].ind!=0) {
	sprintf(str,"I %-3d",fleet[shp].ind);
	Prnt(str,stream);
	}
if(fleet[shp].mines!=0) {
	sprintf(str,"M %-3d",fleet[shp].mines);
	Prnt(str,stream);
	}
if(fleet[shp].pdu!=0) {
	sprintf(str,"D %-3d",fleet[shp].pdu);
	Prnt(str,stream);
	}
if(fleet[shp].spacemines!=0) {
	sprintf(str,"SM %-3d",fleet[shp].spacemines);
	Prnt(str,stream);
	}
for(count=0;count<10;count++)
	if(fleet[shp].ore[count]!=0) {
		sprintf(str,"R%d %-3d",count,fleet[shp].ore[count]);
		Prnt(str,stream);
		}
Prnt("\n",stream);
return;
}

/*****************************************************************************/
int ChekPlan(Planet plan,Player plr,FILE *stream)
/*****************************************************************************/
/* Get details of the planet for the planet summary */
{
char str[128];
int count;

TRTUR2(printf("ChekPlan(plan:%d, plr:%d)\n",plan,plr));
if(galaxy[plan].owner!=plr && plr!=NEUTPLR)
	return(0);
sprintf(str,"%d    ",plan+100);
Prnt(str,stream);
for(count=0;count<10;count++) {
	if(galaxy[plan].mine[count]==0 && galaxy[plan].ore[count]==0) {
		sprintf(str,"  /   ");
		}
	else if(galaxy[plan].mine[count]==0) {
		sprintf(str,"  /%-3d",galaxy[plan].ore[count]);
		}
	else if(galaxy[plan].ore[count]==0) {
		sprintf(str,"%2d/   ",galaxy[plan].mine[count]);
		}
	else {
		sprintf(str,"%2d/%-3d",galaxy[plan].mine[count],galaxy[plan].ore[count]);
		}
	Prnt(str,stream);
	}
if(galaxy[plan].pdu==0)
	Prnt("  -  ",stream);
else {
	sprintf(str," %-4d",galaxy[plan].pdu);
	Prnt(str,stream);
	}
if(galaxy[plan].ind==0)
	Prnt(" -  \n",stream);
else {
	sprintf(str," %-4d\n",galaxy[plan].ind);
	Prnt(str,stream);
	}
return(1);
}

/*****************************************************************************/
void ChekTot(Player plr,FILE *stream)
/*****************************************************************************/
/* Print out total amounts of ore in players empire */
{
Amount tore[10],tmin[10],tind,tpdu;
int count,count2;
char str[128];

TRTUR(printf("ChekTot(plr:%d)\n",plr));
for(count=0;count<10;count++) {
	tore[count]=0;
	tmin[count]=0;
	}
tind=tpdu=0;

for(count=0;count<NUMPLANETS;count++)
	if(galaxy[count].owner==plr || plr==NEUTPLR) {
		for(count2=0;count2<10;count2++) {
			tore[count2]+=galaxy[count].ore[count2];
			tmin[count2]+=galaxy[count].mine[count2];
			}
		tpdu+=galaxy[count].pdu;
		tind+=galaxy[count].ind;
	}

for(count=0;count<shiptr;count++)
	if(fleet[count].owner==plr || plr==NEUTPLR) {
		for(count2=0;count2<10;count2++) {
			tore[count2]+=fleet[count].ore[count2];
			}
		tpdu+=fleet[count].pdu;
		tind+=fleet[count].ind;
	}

sprintf(str,"Total:");
Prnt(str,stream);
for(count=0;count<10;count++) {
	sprintf(str,"%5d/",tmin[count]);
	Prnt(str,stream);
	}
sprintf(str,"  %-4d",tpdu);
Prnt(str,stream);
sprintf(str,"  %-4d\n",tind);
Prnt(str,stream);
sprintf(str,"      ");
Prnt(str,stream);
for(count=0;count<10;count++) {
	sprintf(str,"%5d ",tore[count]);
	Prnt(str,stream);
	}
}

/*****************************************************************************/
void ChekShip(Ship shp,Player plr,FILE *stream)
/*****************************************************************************/
/* Get details of ship for ship summary */
{
char str[128];
int count;

if(fleet[shp].owner!=plr && plr!=NEUTPLR) 
	return;

TRTUR2(printf("ChekShip(ship:%d, plr:%d)\n",shp,plr));
/* Print ship details */
sprintf(str,"S%-3d f=%-3d c=%-3d(%-3d) t=%-3d s=%-3d(%d)",shp+100,fleet[shp].fight,fleet[shp].cargo,fleet[shp].cargleft,fleet[shp].tractor,fleet[shp].shield,Shields(shp));
Prnt(str,stream);
sprintf(str," eff=%-1d(%d) shots=%d",fleet[shp].efficiency,EffEff(shp),Shots(shp,fleet[shp].fight));
Prnt(str,stream);
sprintf(str,"       %s\n",fleet[shp].name);
Prnt(str,stream);
if(fleet[shp].stndord[0]==0)
	sprintf(str,"          Planet: %-3d %-15s Standing Order:None\n",fleet[shp].planet+100,stypes[fleet[shp].type]);
else
	sprintf(str,"          Planet: %-3d %-15s Standing Order:S%d%s\n",fleet[shp].planet+100,stypes[fleet[shp].type],shp+100,fleet[shp].stndord);
Prnt(str,stream);
/* Print out cargo details */
Prnt("          ",stream);
if(fleet[shp].ind!=0) {
	sprintf(str,"I %d ",fleet[shp].ind);
	Prnt(str,stream);
	}
if(fleet[shp].mines!=0) {
	sprintf(str,"M %d ",fleet[shp].mines);
	Prnt(str,stream);
	}
if(fleet[shp].pdu!=0) {
	sprintf(str,"D %d ",fleet[shp].pdu);
	Prnt(str,stream);
	}
if(fleet[shp].spacemines!=0) {
	sprintf(str,"SM %d ",fleet[shp].spacemines);
	Prnt(str,stream);
	}
for(count=0;count<10;count++)
	if(fleet[shp].ore[count]!=0) {
		sprintf(str,"R%d %d.",count,fleet[shp].ore[count]);
		Prnt(str,stream);
		}
Prnt("\n- - - - - - - - - - - - - - - - - - - -\n",stream);
return;
}

/*****************************************************************************/
void ResOutput(Player plr,FILE *stream)
/*****************************************************************************/
{
char str[128];
int nres,count;

TRTUR(printf("Process:Research turn\n"));
nres=NumRes(plr);
Prnt("\nRemember to give research orders this turn\n",stream);
sprintf(str,"You have %d research planet%c\n",nres,nres!=1?'s':' ');
Prnt(str,stream);
if(nres==0)
	return;
if(nres==1) {
	Prnt("It is ",stream);
	}
else {
	Prnt("They are ",stream);
	}
for(count=0;count<NUMPLANETS;count++) 
	if(IsResearch(count) && galaxy[count].owner==plr) {
		sprintf(str," %d",count+100);
		Prnt(str,stream);
	}
	Prnt(".\n",stream);
return;
}

/*****************************************************************************/
void TypeSummary(FILE *outfile)
/*****************************************************************************/
{
char str[128];
int x,types[NUMTYPES];

TRTUR(printf("TypeSummary\n"));
for(x=0;x<NUMTYPES;x++)		/* Clear the types */
	types[x]=0;

for(x=0;x<shiptr;x++)
	types[fleet[x].type]++;	/* Work out how many of each type */
	
Prnt("\nSUMMARY OF SHIP TYPES\n",outfile);
for(x=0;x<NUMTYPES-(NUMTYPES%2);x+=2) {
	sprintf(str,"%-15s %d\t\t%-15s %d\n",stypes[x],types[x],stypes[x+1],types[x+1]);
	Prnt(str,outfile);
	}
if(NUMTYPES%2!=0) {
	sprintf(str,"%-15s %d\n",stypes[NUMTYPES-1],types[NUMTYPES-1]);
	Prnt(str,outfile);
	}
Prnt("\n",outfile);
}

/*****************************************************************************/
void Headings(FILE *outfile,Player plr)
/*****************************************************************************/
{
char str[128];
int income=CalcPlrInc(plr);

TRTUR(printf("Headings\n"));
Prnt("\n",outfile);
sprintf(str,"                         CELESTIAL EMPIRE %d.%d\n",VERSION,PATCHLEVEL);
Prnt(str,outfile);
Prnt("                                  by\n",outfile);
Prnt("                             Zer Dwagon\n",outfile);
Prnt("\n",outfile);
/* Print out name of player and number */
sprintf(str,"        *********   %s   player number=%d     *********\n",name[plr],plr);
Prnt(str,outfile);
/* Print out score and gm and turn numbers */
sprintf(str,"        score=%d       turn number=%d        Game number=%d\n" ,score[plr],turn,gm);
Prnt(str,outfile);
/* Print out due date, and player scores */
sprintf(str,"        date due= before %-8s   player scores %6d %6d %6d\n" ,duedate,score[1],score[2],score[3]);
Prnt(str,outfile);
sprintf(str,"        your income=%4d                          %6d %6d %6d\n",income,score[4],score[5],score[6]);
Prnt(str,outfile);
sprintf(str,"        Earth credits=%4d  Credits:Score=%3d     %6d %6d %6d\n\n",ecredit[plr],gamedet.earth.earthmult,score[7],score[8],score[9]);
Prnt(str,outfile);
/* Print winning conditions */
Prnt("        Winning Conditions:  ",outfile);
if(gamedet.winning&WEARTH) {
	Prnt("Earth  ",outfile);
	}
if(gamedet.winning&WCREDIT) {
	sprintf(str,"Credits=%d  ",gamedet.credits);
	Prnt(str,outfile);
	}
if(gamedet.winning&WINCOME) {
	sprintf(str,"Income=%d  ",gamedet.income);
	Prnt(str,outfile);
	}
if(gamedet.winning&WSCORE) {
	sprintf(str,"Score=%d  ",gamedet.score);
	Prnt(str,outfile);
	}
if(gamedet.winning&WPLANETS) {
	sprintf(str,"Planets=%d  ",gamedet.planets);
	Prnt(str,outfile);
	}
if(gamedet.winning&WTURN) {
	sprintf(str,"Turn=%d  ",gamedet.turn);
	Prnt(str,outfile);
	if(!(gamedet.winning&WFIXTURN)) {
		sprintf(str,"Desired end turn=%d\n",desturn[plr]);
		Prnt(str,outfile);
		}
	}
sprintf(str,"\n        Mail commands to %s %s%d%s\n",NAME,ADDRESS,gm,HOST);
Prnt(str,outfile);
sprintf(str,"        Minimum bids: Cargo=%d Fighter=%d Shield=%d Tractor=%d\n",gamedet.earth.cbid,gamedet.earth.fbid,gamedet.earth.sbid,gamedet.earth.tbid);
Prnt(str,outfile);
sprintf(str,"        You have %d scans this turn\n",NumRes(plr)+1);
Prnt(str,outfile);
}

/*****************************************************************************/
void PlanetSummary(Player plr,FILE *outfile)
/*****************************************************************************/
{
char str[128];
int count,total=0;

TRTUR(printf("Process:Planet Summary\n"));
Prnt("\n\nSUMMARY OF PLANETS.\n",outfile);
Prnt("planet",outfile);
for(count=0;count<10;count++) {
	sprintf(str,"   %d  ",count);
	Prnt(str,outfile);
	}
Prnt("   PDU",outfile);
Prnt("  IND\n",outfile);
Prnt("------",outfile);
for(count=0;count<10;count++)
	Prnt("  --- ",outfile);
Prnt("   ---  ---\n",outfile);
for(count=0;count<NUMPLANETS;count++) {
	total+=ChekPlan(count,plr,outfile);
	}
ChekShipTot(plr,outfile);
Prnt("------",outfile);
for(count=0;count<10;count++)
	Prnt("  --- ",outfile);
Prnt("   ---  ---\n",outfile);
ChekTot(plr,outfile);
sprintf(str,"\n\ntotal number of planets owned = %d\n\n",total);
Prnt(str,outfile);
}

/*****************************************************************************/
void OwnerSummary(FILE *stream)
/*****************************************************************************/
/* Print ownership statistix to file stream */
{
int planown[11];	/* Who owns how many planets */
int indown[11];		/* Who owns how much industry */
int pduown[11];		/* Who owns how many pdus */
int resplan[11];	/* How many research planets */
int count,count2,totinc=0,totscore=0;

/* Clear arrays */
for(count=0;count<NUMPLAYERS+2;count++) {
	planown[count]=0;
	indown[count]=0;
	pduown[count]=0;
	resplan[count]=0;
	}

/* Calculate planet and industry numbers */
for(count=0;count<NUMPLAYERS+1;count++) {
	for(count2=0;count2<NUMPLANETS;count2++)
		if(galaxy[count2].owner==count) {
			if(IsResearch(count2)) 
				resplan[count]++;
			planown[count]++;
			indown[count]+=galaxy[count2].ind;
			pduown[count]+=galaxy[count2].pdu;
			}
	planown[10]+=planown[count];
	indown[10]+=indown[count];
	pduown[10]+=pduown[count];
	resplan[10]+=resplan[count];
	totinc+=CalcPlrInc(count);
	totscore+=score[count];
	}

/* Print planet numbers */
fprintf(stream,"OWNERSHIP STATISTIX\n");
fprintf(stream,"Empire     Plnets   ResPlan  Indust   Income   Score    PDUs\n");
for(count=0;count<NUMPLAYERS+1;count++) {
	fprintf(stream,"%-10s %-8d %-8d %-8d",name[count],planown[count],resplan[count],indown[count]);
	fprintf(stream," %-8d %-8d %-8d\n",CalcPlrInc(count),score[count],pduown[count]);
	}
fprintf(stream,"%-10s %-8d %-8d %-8d","Total",planown[10],resplan[10],indown[10]);
fprintf(stream," %-8d %-8d %-8d\n",totinc,totscore,pduown[10]);
}

/*****************************************************************************/
void UnitSummary(FILE *stream)
/*****************************************************************************/
/* Calculate how many ship units each player has */
{
int fgtown[11],crgown[11],shdown[11],tracown[11],shpown[11];
int count,count2;

for(count=0;count<11;count++) {
	fgtown[count]=0;
	crgown[count]=0;
	shdown[count]=0;
	tracown[count]=0;
	shpown[count]=0;
	}

for(count=0;count<NUMPLAYERS+1;count++) {
	for(count2=0;count2<shiptr;count2++) {
		if(fleet[count2].owner==count) {
			shpown[count]++;
			fgtown[count]+=fleet[count2].fight;
			crgown[count]+=fleet[count2].cargo;
			shdown[count]+=fleet[count2].shield;
			tracown[count]+=fleet[count2].tractor;
			}
		}
	shpown[10]+=shpown[count];
	fgtown[10]+=fgtown[count];
	crgown[10]+=crgown[count];
	shdown[10]+=shdown[count];
	tracown[10]+=tracown[count];
	}

/* Print unit numbers */
fprintf(stream,"\nSHIP STATISTIX\n");
fprintf(stream,"Empire     Ships    Fghtrs   Cargo    Shield   Tractor\n");
for(count=0;count<NUMPLAYERS+1;count++) {
	fprintf(stream,"%-10s %-8d %-8d %-8d",name[count],shpown[count],fgtown[count],crgown[count]);
	fprintf(stream," %-8d %-8d\n",shdown[count],tracown[count]);
	}
fprintf(stream,"%-10s %-8d %-8d %-8d","Total",shpown[10],fgtown[10],crgown[10]);
fprintf(stream," %-8d %-8d\n",shdown[10],tracown[10]);
}

/*****************************************************************************/
void WinningDetails(FILE *stream)
/*****************************************************************************/
/* Print the winning details, and all info from the game structure */
{
int cnt;

TRTUR(printf("WinningDetails()\n"));
fprintf(stream,"\nGAME DETAILS\n");

/* Print winning conditions */
fprintf(stream,"Winning Conditions:  ");
if(gamedet.winning&WEARTH) {
	fprintf(stream,"Earth  ");
	}
if(gamedet.winning&WSCORE) {
	fprintf(stream,"Score=%d  ",gamedet.score);
	}
if(gamedet.winning&WINCOME) {
	fprintf(stream,"Income=%d  ",gamedet.income);
	}
if(gamedet.winning&WPLANETS) {
	fprintf(stream,"Planets=%d",gamedet.planets);
	}

/* Print details of initial home planet */
fprintf(stream,"\nHome Details:\n");
fprintf(stream,"\tInd=%d  PDU=%d  Spacemines=%d  Deployed=%d\n",gamedet.home.ind,gamedet.home.pdu,gamedet.home.spacemine,gamedet.home.deployed);
fprintf(stream,"\tOre Type: ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",cnt);
	}
fprintf(stream,"\n\tAmount:   ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",gamedet.home.ore[cnt]);
	}
fprintf(stream,"\n\tProd:     ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",gamedet.home.mine[cnt]);
	}

/* Print details of initial Earth planet */
fprintf(stream,"\nEarth Details:\n");
fprintf(stream,"\tAmnesty=%d  Earthmult=%d  Bids: Fght=%d  Carg=%d  Shld=%d  Trac=%d\n",gamedet.earth.amnesty,gamedet.earth.earthmult,gamedet.earth.fbid,gamedet.earth.cbid,gamedet.earth.sbid,gamedet.earth.tbid);
fprintf(stream,"\tInd=%d  PDU=%d  Spacemines=%d  Deployed=%d\n",gamedet.earth.ind,gamedet.earth.pdu,gamedet.earth.spacemine,gamedet.earth.deployed);
fprintf(stream,"\tOre Type: ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",cnt);
	}
fprintf(stream,"\n\tAmount:   ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",gamedet.earth.ore[cnt]);
	}
fprintf(stream,"\n\tProd:     ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",gamedet.earth.mine[cnt]);
	}
fprintf(stream,"\n");
return;
}

/*****************************************************************************/
void CostDetails(FILE *stream)
/*****************************************************************************/
/* Print the Earth ore cost */
{
int cnt;

TRTUR(printf("CostDetails()\n"));
fprintf(stream,"\nORE PRICES\n");
fprintf(stream,"Ore Type: ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",cnt);
	}
fprintf(stream,"\nCost:     ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(stream,"%-3d ",price[cnt]);
	}
return;
}

/*****************************************************************************/
void ChekShipTot(Player plr,FILE *stream)
/*****************************************************************************/
/* Print the ore totals that are stored on ships */
{
char str[128];
int count,tmp,oretot[12];

TRTUR2(printf("ChekShipTot(plr:%d)\n",plr));

/* Clear total array */
for(tmp=0;tmp<12;tmp++)
	oretot[tmp]=0;

/* Add up all the ore, ind and pdu amounts */
for(count=0;count<shiptr;count++) {
	if(fleet[count].owner!=plr && plr!=NEUTPLR)
		continue;
	for(tmp=0;tmp<10;tmp++) {
		if(fleet[count].ore[tmp]>0)
			oretot[tmp]+=fleet[count].ore[tmp];
		}
	oretot[10]+=fleet[count].ind;
	oretot[11]+=fleet[count].pdu;
	}

/* Print out results */
sprintf(str,"Ships: ");
Prnt(str,stream);
for(tmp=0;tmp<10;tmp++) {
	if(oretot[tmp]>0) {
		sprintf(str,"  /%-3d",oretot[tmp]);
		Prnt(str,stream);
		}
	else {
		sprintf(str,"  /   ");
		Prnt(str,stream);
		}
	}
	sprintf(str,"  %-4d",oretot[11]);
	Prnt(str,stream);
	sprintf(str," %-4d\n",oretot[10]);
	Prnt(str,stream);
return;
}
