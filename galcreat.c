/* Galaxy creation module for Celestial Empire by Dougal Scott (c) 1989 
 * Usage: galcreat [gamenum] [protofilename]
 * The order of the parameters is irrelevent
 * If `gamenumber' is not defined on the command line then it will be taken from
 * the environment variable CELEMPGAME. If `protofilename' is not defined on the
 * command line then it will default to `./protofile'.
 * Note that the protofile's name cannot begin with a number.
 */

#include <math.h>
#include "def.h"
#include "galc.h"
#include "galcreat.h"
#include "planet.h"

#define NEWSHIPS	gamedet.ship.num*NUMPLAYERS+gamedet.ship2.num*NUMPLAYERS

/* TRGAL is the basic tracing, shows which procedures have been called, and
 * what arguments were passed
 */
#define TRGAL(x)	if(strstr(dbgstr,"GALC") || strstr(dbgstr,"galc")) x
/* TRGAL2 does tracing in more details. This includes functions that are called
 * for every planet. This also creates two files describing the planets and
 * ships of the galaxy.
 */
#define TRGAL2(x)	if(strstr(dbgstr,"GAL2") || strstr(dbgstr,"gal2")) x

#define RND(a)  abs(rand()%(a))
#define RPLAN   246
#define NUMSPEC NUMPLANETS-RPLAN
#define NAMESIZE    10

FILE *trns[NUMPLAYERS+1];
char *dbgstr;
FILE *pl;		/* Planet list file */
planet galaxy[NUMPLANETS]; /* Details of all the planets */
ship fleet[NUMSHIPS];	/* Details of all the ships */
game gamedet;
Ship shiptr;
Number gm,turn;
Number ecredit[NUMPLAYERS+1];
Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
Number score[NUMPLAYERS+1];	/* Score of each plr */
char name[NUMPLAYERS+1][NAMESIZE];
Number price[10];	/* Price of ore on Earth */
FILE *desc;		/* File that contains the text of the galaxy */
int desturn[NUMPLAYERS+1];

/*****************************************************************************/
void Init(Planet num)
/*****************************************************************************/
/* Initialize planet to zero */
{
    int count;

    givePlanetName(num);
    strcpy(galaxy[num].stndord,"");
    galaxy[num].pduleft=0;
    galaxy[num].owner= NEUTPLR;
    for(count=0;count<10;count++) {
        galaxy[num].ore[count]=0;
        galaxy[num].mine[count]=0;
        }
    galaxy[num].ind=galaxy[num].indleft=0;
    galaxy[num].pdu=0;
    for(count=0;count<4;count++)
        galaxy[num].link[count]=NP;
    for(int plr=0; plr<NUMPLAYERS; plr++) {
        galaxy[num].knows[plr] = 0;
    }
    galaxy[num].numlinks=0;
    galaxy[num].spec=0;
}

/*****************************************************************************/
void PrtDesc(Planet pln)
/*****************************************************************************/
/* Put the text description of the planet into the disc file */
{
    char	str[128];
    int	count;

    TRGAL2(printf("Saving details of planet %d\n",pln));

    sprintf(str,"Number:%d\tName:%-24sOwner:%d\tNumlinks:%d\n",pln,galaxy[pln].name,galaxy[pln].owner,galaxy[pln].numlinks);
    fprintf(desc,"%s",str);
    sprintf(str,"Links:%d\t%d\t%d\t%d\n",galaxy[pln].link[0],galaxy[pln].link[1],galaxy[pln].link[2],galaxy[pln].link[3]);
    fprintf(desc,"%s",str);
    sprintf(str,"Ind:%d\tPDU:%d\tIncome:%d\tSpec:%d\n",galaxy[pln].ind,
    galaxy[pln].pdu,galaxy[pln].income,galaxy[pln].spec);
    fprintf(desc,"%s",str);
    for(count=0;count<10;count+=2) {
        sprintf(str,"Type:%d\tMine:%d\tStorage:%d\t",count,galaxy[pln].mine[count], galaxy[pln].ore[count]);
        fprintf(desc,"%s",str);
        sprintf(str,"Type:%d\tMine:%d\tStorage:%d\n",count+1,galaxy[pln].mine[count+1], galaxy[pln].ore[count+1]);
        fprintf(desc,"%s",str);
	}
}

/*****************************************************************************/
void SetHome(Player plyr, Planet plnt)
/*****************************************************************************/
/* Set up all the home planets */
{
    int x;

    galaxy[plnt].owner = plyr;
    for(int rtype=0; rtype<10; rtype++) {
        galaxy[plnt].ore[rtype] = gamedet.home.ore[rtype];
        galaxy[plnt].mine[rtype] = gamedet.home.mine[rtype];
    }
    galaxy[plnt].ind = galaxy[plnt].indleft = gamedet.home.ind;
    galaxy[plnt].numlinks = 4;
    galaxy[plnt].pdu = gamedet.home.pdu;
    galaxy[plnt].spec =  -plyr;
    galaxy[plnt].knows[plyr] = 1;
    /* Make sure that none of the `A' ring planets is defended, or has industry to
     * make life a bit fairer */
    for(x=0; x<4; x++) {
        galaxy[galaxy[plnt].link[x]].pdu = 0;
        galaxy[galaxy[plnt].link[x]].ind = 0;
	}
}

/*****************************************************************************/
void SetResch(Planet plnt)
/*****************************************************************************/
/* Set up all the research planets */
{
    TRGAL2(printf("galcreat:SetResch(plnt: %d)\n",plnt));

    galaxy[plnt].spec=RESEARCH;
}

/*****************************************************************************/
void SetEarth(Planet plnt)
/*****************************************************************************/
/* Set up details for the Earth */
{
    int count;

    strcpy(galaxy[plnt].name,"**** EARTH ****");
    galaxy[plnt].ind = galaxy[plnt].indleft = gamedet.earth.ind;
    galaxy[plnt].pdu = gamedet.earth.pdu;
    galaxy[plnt].spec = EARTH;
    for(count=0;count<10;count++) {
        galaxy[plnt].ore[count] = abs(rand()%gamedet.earth.ore[count]);
        price[count] = 33-(galaxy[plnt].ore[count]*3)/10;
        galaxy[plnt].mine[count] = gamedet.earth.mine[count];
	}
    for(int plr=0; plr<NUMPLAYERS+1; plr++) {
        galaxy[plnt].knows[plr] = 1;
    }
}

/*****************************************************************************/
int Normal(void)
/*****************************************************************************/
/* Determine a semi-normal destribution from 1 to 5 */
{
    int r=RND(100);

    if(r>93)
        return(5);
    if(r>87)
        return(4);
    if(r>75)
        return(3);
    if(r>50)
        return(2);
    return(1);
}

/*****************************************************************************/
void DistrMine(int nummin,Planet num)
/*****************************************************************************/
/* Distribute the mines onto the planet with a tendency to cluster  */
{
    int r,mins=nummin;

    TRGAL2(printf("# %d mines",mins));

    for(;mins>0;) {
        r=RND(10);
        if(galaxy[num].mine[r]==0) {
            if(RND(100)>85) {
                galaxy[num].mine[r]++;
                mins--;
            }
        } else {
            galaxy[num].mine[r]++;
            mins--;
        }
    }
}

/*****************************************************************************/
void SetPlan(Planet num)
/*****************************************************************************/
/* Set up all the normal planets */
{
    int count,nummin;

    TRGAL2(printf("SetPlanet(num: %d\t",num));

    if(RND(100)>(100-gamedet.gal.hasind))
        galaxy[num].ind=galaxy[num].indleft=Normal();
    TRGAL2(printf(" with %d ind",galaxy[num].ind));
    if(RND(100)>(100-gamedet.gal.haspdu))
        galaxy[num].pdu=Normal()*2;
    TRGAL2(printf(" & %d PDU",galaxy[num].pdu));
    if(RND(100)>(100-gamedet.gal.nomine))
        nummin=0;
    else if(RND(100)>(100-gamedet.gal.extramine))
        nummin=abs(Normal()*(RND(8)+1));
    else
        nummin=abs(Normal()*(RND(5)+1));
    DistrMine(nummin,num);
    for(count=0;count<10;count++)
        if(galaxy[num].mine[count]!=0) {
            if(RND(100)>(100-gamedet.gal.extraore)) {
                galaxy[num].ore[count]=galaxy[num].mine[count]+RND(5);
            }
            else {
                galaxy[num].ore[count]=RND(3);
            }
        }
    TRGAL2(printf(" Inc: %d\n",galaxy[num].income));
}

/*****************************************************************************/
void InitDriver(char *protofile)
/*****************************************************************************/
{
/* Procedure to call all the other initialization procedures and do a bit of
work itself */
    int count;

    TRGAL(printf("Initializing score\n"));

    LoadProto(protofile);
    for(count=0;count<NUMPLAYERS+1;count++) {
        score[count]=0;
        ecredit[count]=0;
        desturn[count]=30;
        }
    shiptr=NEWSHIPS;
    TRGAL(printf("Initializing planets\n"));
    for(count=0;count<NUMPLANETS;count++)
        Init(count);
    InitAlliances();
}

/*****************************************************************************/
int main(int argc,char **argv)
/*****************************************************************************/
{
    int count,count2,r;
    char *gmstr;
    int trans[NUMPLANETS],used[NUMPLANETS],old[NUMPLANETS];
    char *protofile;

    /* trans:	trans[old planet]=new planet */
    /* used:	If new planet has been selected yet */
    /* old:		old[new planet]=old planet */

    printf("Celestial Empire\nGalactic Creation Program now operating\n");
    printf("Version:%d.%d\n",VERSION,PATCHLEVEL);

    if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
        dbgstr=(char *)"null";

    gm=-1;
    protofile=(char *)"./protofile";
    /* strcpy(protofile,"./protofile"); */

    for(r=1;r<argc;r++) {
        if(isdigit(argv[r][0]))
            gm=atoi(argv[r]);
        else
            strcpy(protofile,argv[r]);
        }
    if(gm<0) {
        if((gmstr = getenv("CELEMPGAME")) == NULL) {
            fprintf(stderr,"set CELEMPGAME to the appropriate game number\n");
            exit(-1);
            }
        gm=atoi(gmstr);
        }

    srand(gm);    /* Set the seed */
    turn=0;
    TRGAL(printf("\nIt is gm %d\n",gm));
    InitDriver(protofile);
    TRGAL(printf("Initializing main arrays\n"));
    for(count=0;count<NUMPLANETS;count++) {
        trans[count]=NP;
        old[count]=0;
        SetPlan(count);
        used[count]=0;
        }

    /* Set up placed planets */
    TRGAL(printf("Setting up the placed planets\n"));
    for(count=0;count<NUMPLANETS;count++)
        for(;;) {
            r=RND(NUMPLANETS);
            if(used[r]==0) {
                trans[count]=r;
                old[r]=count;
                used[r]=1;
                break;
                }
            }

    /* Install the links on the placed planets */
    TRGAL(printf("Installing the placed planets links\n"));
    for(r=0;r<NUMPLANETS;r++) {
        /* Get new planet number from old planet number */
        count=old[r];
        if(count!=UP) {
            for(count2=0;count2<4;count2++) {
                if(linkmap[count][count2]>=0) {
                    galaxy[r].link[count2]=trans[linkmap[count][count2]];
                    galaxy[r].numlinks++;
                } else 
                    galaxy[r].link[count2]=linkmap[count][count2];
                }
            }
        }

    /* Set up Earth */
    TRGAL(printf("Setting up Earth\n"));
    SetEarth(trans[ERTH]);
    TRGAL(printf("Earth: %d, New:%d\n",ERTH,trans[ERTH]));

    /* Set up the home planets */
    TRGAL(printf("Setting up the home planets\n"));
    for(count=1;count<NUMPLAYERS+1;count++) {
        TRGAL(printf("Home %d: %d, New:%d\n",count,home[count],trans[home[count]]));
        SetHome(count,trans[home[count]]);
        }

    /* Set up the research planets */
    TRGAL(printf("Setting up the research planets\n"));
    for(count=0;count<NUMRES;count++)
        SetResch(trans[res[count]]);

    /* Set up the ships on the home planets */
    TRGAL(printf("Setting up the ships in the home planets\n"));
    for(count=1;count<NUMPLAYERS+1;count++)
        Ships(count,trans[home[count]]);
    strcpy(name[NEUTPLR],"NEUTRAL");
    SortLinks();
    for(count=0;count<NUMPLANETS;count++)
        galaxy[count].income=CalcIncome(count);

    TRGAL2(WriteGalaxyInfo());
    TRGAL2(WriteShipInfo());
    WriteGalflt();
    printf("Finished.\n");
    return(0);
}

/*****************************************************************************/
void InitAlliances(void)
/*****************************************************************************/
/* Initialize the alliances to all neutral */
{
int count,count2;

for(count=1;count<NUMPLAYERS+1;count++) 
	for(count2=1;count2<NUMPLAYERS+1;count2++)
		alliance[count][count2]=NEUTRAL;
}

/*****************************************************************************/
void Ships(Player plr,Planet plnt)
/*****************************************************************************/
/* Set up the ships of player plr on planet plnt */
{
    TRGAL(printf("galcreat:Ships(plr: %d, plnt: %d)\n",plr,plnt));
    ShipSet(plnt,plr);
    GetName(plr);	/* Ask administrator for player name */
    name[plr][9]=0;
}

/*****************************************************************************/
void WriteGalaxyInfo(void)
/*****************************************************************************/
{
    char fname[BUFSIZ];
    int count;

    FilePath("galinfo", fname);
    desc=fopen(fname, "w");
    if(desc==NULL) {
        printf("Warning: could not open %s for writing\n", fname);
        return;
        }
    printf("Writing to the first file\n");
    for(count=0;count<NUMPLANETS;count++)
        PrtDesc(count);
    fclose(desc);
}

/*****************************************************************************/
void WriteShipInfo(void)
/*****************************************************************************/
{
	int count;
	char fname[BUFSIZ];

    FilePath("fleetinfo", fname);
	if((desc=fopen(fname, "w")) == NULL) {
		printf("Warning: could not open %s for writing\n", fname);
		return;
	}
	for(count=0;count<shiptr;count++)
		PrtShip(count);
	fclose(desc);
}

/*****************************************************************************/
void GetName(Player plr)
/*****************************************************************************/
/* get the name of plr number plr */
{
    printf("Please Enter plr %d's name:",plr);
    fgets(name[plr], NAMESIZE, stdin);
    name[plr][strcspn(name[plr], "\n")] = 0;
}

/*****************************************************************************/
void PrtShip(Ship shp)
/*****************************************************************************/
/* Print details of ship to text file */
{
    char str[128];

    TRGAL2(printf("Writing details of ship %d\n",shp));

    sprintf(str,"Number:%d\tOwner:%d\tPlanet:%d\n",shp,fleet[shp].owner,fleet[shp].planet);
    fprintf(desc,"%s",str);
    sprintf(str,"Fight:%d\tCargo:%d\tType:%d\n",fleet[shp].fight,fleet[shp].cargo,fleet[shp].type);
    fprintf(desc,"%s",str);
}

/*****************************************************************************/
void ShipSet(Planet plan,Player plr)
/*****************************************************************************/
/* Set up all the ships on the home planet */
{
    int count,tmptr;
    static int snum[NUMSHIPS],sflag=0;

    TRGAL2(printf("galcreat:\tShipSet(plan:%d, plr:%d)\n",plan,plr));

    if(!sflag) {
        for(count=0;count<NUMSHIPS;count++)
            snum[count]=0;
        sflag=1;
    }

    for(count=0;count<gamedet.ship.num;count++) {
        for(tmptr=RND(NEWSHIPS);snum[tmptr]!=0;tmptr=RND(NEWSHIPS));
        snum[tmptr]=1;
        InitShip1(tmptr);
        fleet[tmptr].owner=plr;
        fleet[tmptr].planet=plan;
    }
    for(count=0;count<gamedet.ship2.num;count++) {
        for(tmptr=RND(NEWSHIPS);snum[tmptr]!=0;tmptr=RND(NEWSHIPS));
        snum[tmptr]=1;
        InitShip2(tmptr);
        fleet[tmptr].owner=plr;
        fleet[tmptr].planet=plan;
	}
}

/*****************************************************************************/
void InitShip1(Ship shp)
/*****************************************************************************/
/* Initialize all the ships to default */
{
    int count;

    TRGAL2(printf("galcreat:InitShip(shp:%d)\n",shp));

    strcpy(fleet[shp].name,"");
    strcpy(fleet[shp].stndord,"");
    fleet[shp].figleft=gamedet.ship.fight;
    fleet[shp].fight=gamedet.ship.fight;
    fleet[shp].cargo=gamedet.ship.cargo;
    fleet[shp].tractor=gamedet.ship.tractor;
    fleet[shp].shield=gamedet.ship.shield;
    fleet[shp].type=CalcType(shp);
    for(count=0;count<10;count++)
        fleet[shp].ore[count]=0;
    fleet[shp].ind=0;
    fleet[shp].mines=0;
    fleet[shp].moved=0;
    fleet[shp].engage=0;
    fleet[shp].pdu=0;
    fleet[shp].cargleft=gamedet.ship.cargo;
    fleet[shp].hits=0;
    fleet[shp].pduhits=0;
    fleet[shp].efficiency=gamedet.ship.eff;
}

/*****************************************************************************/
void InitShip2(Ship shp)
/*****************************************************************************/
/* Initialize all the ships to default */
{
    int count;

    TRGAL2(printf("galcreat:InitShip(shp:%d)\n",shp));

    strcpy(fleet[shp].name,"");
    strcpy(fleet[shp].stndord,"");
    fleet[shp].figleft=gamedet.ship2.fight;
    fleet[shp].fight=gamedet.ship2.fight;
    fleet[shp].cargo=gamedet.ship2.cargo;
    fleet[shp].tractor=gamedet.ship2.tractor;
    fleet[shp].shield=gamedet.ship2.shield;
    fleet[shp].type=CalcType(shp);
    for(count=0;count<10;count++) {
        fleet[shp].ore[count]=0;
    }
    fleet[shp].ind=0;
    fleet[shp].mines=0;
    fleet[shp].moved=0;
    fleet[shp].engage=0;
    fleet[shp].pdu=0;
    fleet[shp].cargleft=gamedet.ship2.cargo;
    fleet[shp].hits=0;
    fleet[shp].pduhits=0;
    fleet[shp].efficiency=gamedet.ship2.eff;
}

/*****************************************************************************/
int LinkCmp(const void *a, const void *b)
/*****************************************************************************/
/* Compare links for qsort */
{
    return(*(Link *)a-*(Link *)b);
}

/*****************************************************************************/
void SortLinks(void)
/*****************************************************************************/
/* Sort the planetary links into ascending order */
{
    int count,count2;

    for(count=0;count<NUMPLANETS;count++) {
        for(count2=0;count2<4;count2++)	/* Move non existant links to the back */
            if(galaxy[count].link[count2]<0)
                galaxy[count].link[count2]=300;
        qsort(galaxy[count].link, 4, sizeof(Link), LinkCmp);
        for(count2=0;count2<4;count2++) /* Bring non existant links back */
            if(galaxy[count].link[count2]==300)
                galaxy[count].link[count2]= -1;
    }
}

/*****************************************************************************/
void givePlanetName(Planet num)
/*****************************************************************************/
/* Name the planet from the list of planet names */
{
    int i;

    TRGAL2(printf("galcreat:givePlanetName(num:%d)\n",num));

    for(i=RND(NUMNAMES);planetlist[i][0]==0;i=RND(NUMNAMES));
    strcpy(galaxy[num].name,planetlist[i]);
    planetlist[i][0]=0;
    return;
}
