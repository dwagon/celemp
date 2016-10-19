/* MAIN.C of Celestial Empire by Dougal Scott (c) 2016 */

/* If you want to make changes to any of Celestial Empire, mail me
 * dwagon@pobox.com to discuss the new code, and if I like
 * it, I will put it in the official code. Also patches to get this code
 * working on other systems are always appreciated
 */

/* The CELEMPGAME environment variable should be set when running any of the
 * celemp programs, this tells the program what game you are talking about.
 * Alternatively you can specify the game number on the command line, which
 * takes precedence over teh environment variable
 */

/* The CELEMPPATH environment variable has to be set before running any of the
 * the celemp programs. It tells the programs where to find the datafiles for
 * a particular game, and where ot put the output files. The game number will
 * be appended to this variable to get the full path. Eg if the game was 1
 * and CELEMPPATH was set to /usr/games/celemp/data then the program will
 * try and put its data files in a directory called /usr/games/celemp/data1
 */

#include "def.h"
#include "cmnd.h"

#define TRMAIN(x)   if(strstr(dbgstr,"MAIN") || strstr(dbgstr,"main")) x;

#define ZX      (cmdarr[idx][idx2++])

FILE    *trns[NUMPLAYERS+1];        /* File to put commands executed */
FILE    *bidfp;                     /* Bid files */
planet  galaxy[NUMPLANETS];         /* Details on all the planets */
ship    fleet[NUMSHIPS];            /* Details on all the ships */
game    gamedet;                    /* Details of game */
Number  price[10];                  /* Price of ore on earth */
Number  sknd[NUMPLAYERS+1];         /* If the player has scanned this turn */
Number  ecredit[NUMPLAYERS+1];      /* Earth credit */
Flag    alliance[NUMPLAYERS+1][NUMPLAYERS+1];           /* Diplomatic matrix */
Ship    shiptr;                     /* Number of ships that exist */
int     cmdarr[NUMPLAYERS*80][50];  /* Each player can issue upto 80 cmds */
int     idx2,idx;                   /* Indicies into command array */
Player  plr;                    /* Which players commands being considered */
int     turn;                       /* What turn it is */
int     gm;                         /* What game it is */
int     score[NUMPLAYERS+1];        /* Players scores */
char    name[NUMPLAYERS+1][10];     /* Name of players empire */
int     desturn[NUMPLAYERS+1];      /* Desired game length */
char    *game_path;                     /* Path to find data files */
int     PlrFlag[]={                 /* Used for ORing a scan */
    PLR0,
    PLR1,   PLR2,   PLR3,
    PLR4,   PLR5,   PLR6,
    PLR7,   PLR8,   PLR9    };
int shipvict[NUMSHIPS][NUMPLAYERS+1][2];    /* Which players have done how
                                        much damage to a ship */
int changed[NUMPLAYERS+1][NUMPLAYERS+1];    /* Whether alliance has been changed
                                        this turn */
char *dbgstr;

/***************************************************************************/
void ProcessTurn(void)
/***************************************************************************/
/* Process the turn */
{
int tmp;
int dnatks=0;   /* Whether attacks have been resolved yet */
int dnearth=0;  /* Whether earth has rebuilt */

TRMAIN(printf("ProcessTurn\n"));

for(idx2=0,idx=0;idx<NUMPLAYERS*80;idx++,idx2=0) {
    plr=(Player)ZX;     /* First field is player number */
    tmp=ZX;             /* Second field is action */
    if(tmp>RSLVATT && dnatks==0) {
        EarthRetaliate();
        ResolveDamage();
        dnatks=1;
        }
    if(tmp>EARTHDEF && dnearth==0) {
        RebuildEarth();
        dnearth=1;
        }
    if(tmp>CONTSTART && tmp<CONTEND)
        DoEarth();
    else
        if(tmp!=NOOPERAT) {
            TRMAIN(printf("ProcessTurn:\tplr:%d, action:%d\n",plr,tmp));
            Execute(tmp);
            }
    }
return;
}

/***************************************************************************/
void Execute(int cmd)
/***************************************************************************/
/* Execute commands */
{
int sbj,i,plnt,ore[13],shp;
char tmpname[30];

TRMAIN(printf("Execute(cmd: %d)\n",cmd));

switch(cmd) {

    case NOOPERAT:  /* Do nothing */
        TRMAIN(printf("Execute:\tNOOPERAT\n"));
        break;  /* Do nothing: */

    case GAMELEN:   /* Change the players desired game length */
        TRMAIN(printf("Execute:\tGAMELEN\n"));
        desturn[plr]=ZX;
        fprintf(trns[plr],"TURN%d",desturn[plr]);
        if(gamedet.winning&WFIXTURN)
            fprintf(trns[plr],"Game length fixed");
        fprintf(trns[plr],"\n");
        break;

    case NAMESHIP:  /* Name a ship */
        TRMAIN(printf("Execute:\tNAMESHIP\n"));
        sbj=(Ship)ZX;
        for(i=0;i<30;tmpname[i++]=ZX);
        NameShip(sbj,tmpname);
        break;  /* Name ship: ship,tmpname */

    case NAMEPLAN:  /* Name a planet */
        TRMAIN(printf("Execute:\tNAMEPLAN\n"));
        sbj=(Planet)ZX;
        for(i=0;i<30;tmpname[i++]=ZX);
        NamePlanet(sbj,tmpname);
        break;  /* name planet: planet,tmpname */

    case GIFTSHIP:  /* Give a ship to another player */
        GiftShip((Ship)ZX,(Player)ZX);
        break;  /* Gift ship: ship,plr */

    case GIFTPLAN:  /* Give a planet to another player */
        GiftPlan((Planet)ZX,(Player)ZX);    /* Gift planet: plan,plr */
        break;

    case UNLODALL:  /* Unload all ore from a ship */
        UnlAll((Ship)ZX);
        break;  /* Unload all:sbj */

    case UNLODIND:  /* Unload industry from a ship */
        UnlInd((Ship)ZX,(Amount)ZX);
        break;  /* Unload industry:sbj,amnt */

    case UNLODDEF:  /* Unload PDUs from a ship */
        UnlDef((Ship)ZX,(Amount)ZX);
        break;  /* Unload defence:sbj,amnt */

    case UNLODORE:  /* Unload ore from a ship */
        UnlOre((Oretype)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Unload ore:type,sbj,amnt */

    case UNLODMIN:  /* Unload mines from a ship */
        UnlMin((Oretype)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Unload mine:type,sbj,amnt */

    case UNLODSPM:  /* Unload spacemines */
        UnlSpcmin((Ship)ZX,(Amount)ZX);
        break;

    case LOADPRI:   /* Load all the ore but do priority load first */
        shp=ZX;
        ore[0]=ZX; ore[1]=ZX; ore[2]=ZX; ore[3]=ZX;
        ore[4]=ZX; ore[5]=ZX; ore[6]=ZX; ore[7]=ZX;
        ore[8]=ZX; ore[9]=ZX; ore[10]=ZX; ore[11]=ZX;
        ore[12]=ZX;
        LoadPri((Ship)shp,ore);
        break;

    case LOADALL:   /* Load all the ore that can fit */
        LoadAll((Ship)ZX);
        break;  /* Load all:sbj */

    case LOADORE:   /* Load ore onto a ship */
        LoadOre((Oretype)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Load ore:type,sbj,amnt */

    case LOADMIN:   /* Load mine onto ship */
        LoadMin((Oretype)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Load mine :type,sbj,amnt  */

    case LOADDEF:   /* Load pdu's onto a ship */
        LoadDef((Ship)ZX,(Amount)ZX);
        break;  /* Load pdu :shp,amt */

    case LOADIND:   /* Load industry onto a ship */
        LoadInd((Ship)ZX,(Amount)ZX);
        break;  /* Load industry:sbj,amt */

    case LOADSPM:   /* Load spacemines */
        LoadSpcmine((Ship)ZX,(Amount)ZX);
        break;

    case TENDALL:   /* Tend all the ore that can fit */
        TendAll((Ship)ZX,(Ship)ZX);
        break;  /* Tend all:sbj, vict */

    case TENDORE:   /* Tend ore onto a ship */
        TendOre((Oretype)ZX,(Ship)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Tend ore:type,sbj,vict,amnt */

    case TENDMIN:   /* Tend mine onto ship */
        TendMin((Ship)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Tend mine :sbj,vict,amnt  */

    case TENDDEF:   /* Tend pdu's onto a ship */
        TendDef((Ship)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Tend pdu :shp,vict,amt */

    case TENDIND:   /* Tend industry onto a ship */
        TendInd((Ship)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Tend industry:sbj,vict,amt */

    case TENDSPM:   /* Tend spacemines */
        TendSpcmine((Ship)ZX,(Ship)ZX,(Amount)ZX);
        break;

    case SELLALL:   /* Sell all ore on Earth */
        SellOre((Ship)ZX,(Amount)(-1),(Oretype)(-1));
        break;

    case SELLORE:   /* Sell ore on earth */
        SellOre((Ship)ZX,(Amount)ZX,(Oretype)ZX);
        break;  /* Sell ore type at Earth:sbj,amt,type */

    case BUYORE:    /* Buy ore from Earth */
        BuyOre((Ship)ZX,(Amount)ZX,(Oretype)ZX);
        break;  /* Buy ore on Earth:sbj,amt,type */

    case CONTCARG:  /* Contract for cargo units */
        ContCarg((Ship)ZX,(Amount)ZX,(Number)ZX);
        break;  /* Contract to build cargo units:sbj,amt,bid */

    case CONTFGHT:  /* Contract for fighter units */
        ContFght((Ship)ZX,(Amount)ZX,(Number)ZX);
        break;  /* Contract to build fighter units:sbj,amt,bid */

    case CONTTRAC:  /* Contract for tractor units */
        ContTrac((Ship)ZX,(Amount)ZX,(Number)ZX);
        break;  /* Contract to build tractor units:sbj,amt,bid */

    case CONTSHLD:  /* Contract for shield units */
        ContShld((Ship)ZX,(Amount)ZX,(Number)ZX);
        break;  /* Contract to build shield units:sbj,amt,bid */

    case ENGTRAC:   /* Engage tractor units */
        EngageTractor((Ship)ZX,(Ship)ZX);
        break;/* Engage tractor beams:sbj,vict */

    case PLANATT:   /* Attack a ship from a planet */
        PlanetAttack((Planet)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Planet attack ship:sbj,vict */
    
    case PLANSPM:   /* Attack spacemines from a planet */
        PlanetSpcmin((Planet)ZX,(Amount)ZX);
        break;  /* Planet attack spacemines: sbj */

    case ATTKSHP:   /* Attack a ship from a ship */
        ShpShp((Ship)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Ship vs Ship combat:sbj,vict  */

    case ATTKDEF:   /* Attack PDU's from a ship */
        ShpPdu((Ship)ZX,(Amount)ZX);
        break;  /* Ship attack PDU:sbj      */

    case ATTKIND:   /* Attack industry from a ship */
        ShpInd((Ship)ZX,(Amount)ZX);
        break;  /* Ship attack industry */

    case ATTKORE:   /* Attack ore from a ship */
        ShpOre((Oretype)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Ship attack ore:type,sbj */

    case ATTKMIN:   /* Attack mines from a ship */
        ShpMin((Oretype)ZX,(Ship)ZX,(Amount)ZX);
        break;  /* Ship attack mine:type,sbj */

    case ATTKSPM:   /* Attack spacemines from a ship */
        ShpSpcmin((Ship)ZX,(Amount)ZX);
        break;

    case BUILDSPM:  /* Build spacemines */
        BuildSpcmines((Planet)ZX,(Amount)ZX,(Oretype)ZX);
        break;  /* Build spacemines:plan,amt,type */

    case BUILDDEF:  /* Build PDU's */
        BuildDef((Planet)ZX,(Amount)ZX);
        break;  /* Build defence:sbj,amt */

    case BUILDIND:  /* Build industry */
        BuildInd((Planet)ZX,(Amount)ZX);
        break;  /* Build industry:sbj,amt */

    case BUILDMIN:  /* Build mines */
        BuildMine((Planet)ZX,(Oretype)ZX,(Amount)ZX);
        break;  /* Build mine:sbj,type,amt */

    case UNBLDCAR:  /* UnBuild cargo units onto a ship */
        UnbldCargo((Ship)ZX,(Amount)ZX);
        break;  /* UnBuild cargo units:sbj,amt */

    case UNBLDFGT:  /* UnBuild fighter units onto a ship */
        UnbldFight((Ship)ZX,(Amount)ZX);
        break;  /* UnBuild fighter units:sbj,amt  */

    case UNBLDTRC:  /* UnBuild tractor units onto a ship */
        UnbldTrac((Ship)ZX,(Amount)ZX);
        break;  /* UnBuild tractor units:sbj,amt */

    case UNBLDSLD:  /* UnBuild shield units */
        UnbldShield((Ship)ZX,(Amount)ZX);
        break;  /* UnBuild shield units:sbj,amt */

    case BUILDCAR:  /* Build cargo units onto a ship */
        BuildCargo((Ship)ZX,(Amount)ZX);
        break;  /* Build cargo units:sbj,amt */

    case BUILDFGT:  /* Build fighter units onto a ship */
        BuildFight((Ship)ZX,(Amount)ZX);
        break;  /* Build fighter units:sbj,amt  */

    case BUILDTRC:  /* Build tractor units onto a ship */
        BuildTrac((Ship)ZX,(Amount)ZX);
        break;  /* Build tractor units:sbj,amt */

    case BUILDSLD:  /* Build shield units */
        BuildShield((Ship)ZX,(Amount)ZX);
        break;  /* Build shield units:sbj,amt */

    case UNDEPL:    /* Retrieve spacemines */
        Undeploy((Ship)ZX,(Amount)ZX);
        break;

    case DEPLPLN:   /* Deploy planet */
        DeployPlnt((Planet)ZX,(Amount)ZX);
        break;

    case DEPLSHP:   /* Deploy ship */
        DeployShp((Ship)ZX,(Amount)ZX);
        break;

    case JUMP1:
        Jump1((Ship)ZX,(Planet)ZX);
        break;  /* Jump distance 1:sbj,dest1 */

    case JUMP2:
        Jump2((Ship)ZX,(Planet)ZX,(Planet)ZX);
        break;  /* Jump distance 2:sbj,dest1,dest2 */

    case JUMP3:
        Jump3((Ship)ZX,(Planet)ZX,(Planet)ZX,(Planet)ZX);
        break;  /* Jump distance 3:sbj,dest1,dest2,dest3 */

    case JUMP4:
        Jump4((Ship)ZX,(Planet)ZX,(Planet)ZX,(Planet)ZX,(Planet)ZX);
        break;  /* Jump distance 4:sbj,dest1,dest2,dest3,dest4 */

    case JUMP5:
        Jump5((Ship)ZX,(Planet)ZX,(Planet)ZX,(Planet)ZX,(Planet)ZX,(Planet)ZX);
        break;  /* Jump distance 5:sbj,dest1,dest2,dest3,dest4,dest5 */

    case BUILDHYP:
        BuildHyp((Ship)ZX,(Number)ZX,(Amount)ZX,(Amount)ZX,(Amount)ZX,(Amount)ZX);
        break;  /* Build Hyperdrive:sbj,amt,fgt,crg,trac,shld */

    case TRANSORE:
        Transmute((Planet)ZX,(Amount)ZX,(Oretype)ZX,(Oretype)ZX);
        break;  /* Transmute ore */

    case SCAN:
        plnt=ZX;
        if(sknd[plr] != 0) {
            galaxy[plnt].scanned |= PlrFlag[plr];
            galaxy[plnt].knows[plr] = 1;
            fprintf(trns[plr],"SCAN%d\n", plnt+100);
            sknd[plr]--;
            }
        else {
            fprintf(stderr, "Execute:Plr %d has scanned this turn\n", plr);
            fprintf(trns[plr], "Already scanned this turn: scan %d\n", plnt+100);
            }
        break;  /* Scan planet: sbj */

    case ALLNCUP:
        i=(Player)ZX;
        fprintf(trns[plr],"+%s\t",name[i]);
        if(i==plr) {
            fprintf(stderr,"Execute:Plr %d cant up status with self\n",plr);
            return;
            }
        if(i==NEUTPLR) {
            fprintf(stderr,"Execute:Plr %d cant up status with neutral\n",plr);
            return;
            }
        if(changed[plr][i]!=0) {
            fprintf(trns[plr],"Already changed alliance status this turn\n");
            return;
            }
        if(alliance[plr][i]!=ALLY) {
            changed[plr][i]++;
            alliance[plr][i]++;
            fprintf(trns[plr],"+%s\n",name[i]);
            }
        else {
            fprintf(stderr,"Execute:Plr %d already an ally\n",i);
            fprintf(trns[plr],"Player %s already an ally\n",name[i]);
            }
        break;  /* Increase alliance:plr  */

    case ALLNCDN:
        i=(Player)ZX;
        fprintf(trns[plr],"-%s\t",name[i]);
        if(i==plr) {
            fprintf(stderr,"Execute:Plr %d cant down status with self\n",plr);
            return;
            }
        if(i==NEUTPLR) {
            fprintf(stderr,"Execute:Plr %d cant up status with neutral\n",plr);
            return;
            }
        if(changed[plr][i]!=0) {
            fprintf(trns[plr],"Already changed alliance status this turn\n");
            return;
            }
        if(alliance[plr][i]!=ENEMY) {
            fprintf(trns[plr],"-%s\n",name[i]);
            changed[plr][i]++;
            alliance[plr][i]--;
            }
        else {
            fprintf(stderr,"Execute:Plr %d already an enemy\n",i);
            fprintf(trns[plr],"Player %s already an enemy\n",name[i]);
            }
        break;  /* Decrease alliance:plr */
    default:
        fprintf(stderr,"Serious Error, unknown command:%4d Plr:%4d\n",cmd,plr);
    }
return;
}
    
/***************************************************************************/
int main(int argc,char **argv)
/***************************************************************************/
{
char *gmstr;
char str[BUFSIZ];

printf("Celestial Empire Version:%d.%d\n",VERSION,PATCHLEVEL);

if((dbgstr = getenv("CELEMPDEBUG")) == NULL ) { 
    dbgstr=(char *)"null";
    }

if((game_path = getenv("CELEMPPATH")) == NULL) {
    fprintf(stderr,"set CELEMPPATH to the appropriate path\n");
    exit(-1);
    }

if(argc==2)
    gm=atoi(argv[1]);
else {
    if((gmstr = getenv("CELEMPGAME")) == NULL) {
        fprintf(stderr,"set CELEMPGAME to the appropriate game number\n");
        exit(-1);
        }
    gm=atoi(gmstr);
    }

TRMAIN(printf("Reading in galaxy structure\n"));
if(ReadGalflt()==-1) {
    fprintf(stderr,"Program terminated\n");
    exit(-1);
    }

TRMAIN(printf("Opening execution histories\n"));
if(OpenExhist("a")==-1) {
    fprintf(stderr,"Program terminated\n");
    exit(-1);
    }

sprintf(str,"%s%d/bids",game_path,gm);
if((bidfp=fopen(str,"r"))==NULL) {
    fprintf(stderr,"Could not open file %s for reading\n",str);
    exit(-1);
    }

TRMAIN(printf("Reading in turn commands\n"));
if(ReadInTurn()==-1) {
    fprintf(stderr,"Program terminated\n");
    exit(-1);
    }

TRMAIN(printf("Backing up galaxy structure\n"));
sprintf(str,"cp %s%d/galfile %s%d/galfile.%d",game_path,gm,game_path,gm,turn);
(void)system(str);

TRMAIN(printf("Reseting structures\n"));
ResetScan();
CalcEarthDmg();

TRMAIN(printf("Processing turn\n"));
ProcessTurn();

TRMAIN(printf("End of turn sequence\n"));
EndOfTurn();

TRMAIN(printf("Writing galaxy structure\n"));
WriteGalflt();

TRMAIN(printf("Closing execution histories\n"));
CloseExhist();
fclose(bidfp);

return(0);
}

/***************************************************************************/
void ResetScan(void)
/***************************************************************************/
/* Reset the scanned flag and lots of other stuff*/
{
int count,x;

/* Give each player their minimum 1 scan */
TRMAIN(printf("ResetScan()\n"));
for(count=1;count<NUMPLAYERS+1;count++) {
    sknd[count]=1;
    for(x=0;x<NUMPLAYERS+1;x++)
        changed[x][count]=0;
    }

TRMAIN(printf("resetting planets\n"));
for(count=0;count<NUMPLANETS;count++) {
    /* Clear the scanned flag on every planet */
    galaxy[count].indleft=galaxy[count].ind;
    galaxy[count].scanned=0;
    galaxy[count].pduleft=galaxy[count].pdu;
    /* For every research planet a player has increase their scans by 1 */
    if(IsResearch(count))
        sknd[galaxy[count].owner]++;
    }

TRMAIN(printf("resetting ships\n"));
for(count=0;count<shiptr;count++) {
    fleet[count].engage=0;
    fleet[count].moved=0;
    fleet[count].hits=0;
    fleet[count].pduhits=0;
    fleet[count].figleft=fleet[count].fight;
    fleet[count].cargleft=fleet[count].cargo-fleet[count].mines*20;
    fleet[count].cargleft-=fleet[count].ind*10;
    fleet[count].cargleft-=fleet[count].pdu*2;
    fleet[count].cargleft-=fleet[count].spacemines;
    for(x=0;x<NUMPLAYERS+1;x++)
        shipvict[count][x][SHIP]=shipvict[count][x][PDU]=0;
    for(x=0;x<10;x++)
        fleet[count].cargleft-=fleet[count].ore[x];
    }
return;
}

/*****************************************************************************/
int ReadInTurn(void)
/*****************************************************************************/
/* Read the commands of the turn into memeory   */
{
char str[124];
FILE *infile;
int tmp,c1,c2;

TRMAIN(printf("ReadInTurn(%d)\n",gm));
TRMAIN(printf("Reading in turn\n"));
sprintf(str,"%s%d/cmdout",game_path,gm);
if((infile=fopen(str,"r"))==NULL) {
    fprintf(stderr,"ReadInTurn: Unable to open %s for reading\n",str);
    return(-1);
    }

for(c1=c2=0;;) {
    if(fscanf(infile,"%d",&tmp)==EOF)
        break;
    if(tmp==ENDCOMMAND) {
        c1++;
        c2=0;
        }
    else
        cmdarr[c1][c2++]=tmp;
    }
fclose(infile);
return(0);
}
