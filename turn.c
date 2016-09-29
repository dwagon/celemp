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
int turn, gm;
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
	PLR1, 	PLR2, 	PLR3,
	PLR4, 	PLR5, 	PLR6,
	PLR7, 	PLR8, 	PLR9	};
char *path;

/*****************************************************************************/
int main(int argc, char **argv)
/*****************************************************************************/
/* Do the lot */
{
Player plr;
char *gmstr;

if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
	dbgstr=(char *)"null";

if((path = getenv("CELEMPPATH")) == NULL) {
	fprintf(stderr, "set CELEMPPATH to the appropriate directory\n");
	exit(-1);
	}

if(argc>=2)
	gm=atoi(argv[1]);
else {
	if((gmstr = getenv("CELEMPGAME")) == NULL) {
		fprintf(stderr, "set CELEMPGAME to the appropriate game number\n");
		exit(-1);
		}
	gm=atoi(gmstr);
	}

if(ReadGalflt()== -1) {
	fprintf(stderr, "Program terminated\n");
	exit(-1);
	}
printf("Celestial Empire Version %d.%d Turn sheet creation program\n", VERSION, PATCHLEVEL);
earth=LocateEarth();
if(argc==3) {
	if(PrintGalaxy(argv[2])==-1)
		fprintf(stderr, "Galaxy listing aborted due to file error\n");
	}
else {
	Init();
	for(plr=1;plr<NUMPLAYERS+1;plr++) {
		TRTUR(printf("Processing player%d\n", plr));
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
FILE *outfile;
int count;
char filename[BUFSIZ];
char str[BUFSIZ];

TRTUR(printf("PrintGalaxy\n"));
sprintf(filename, "%s", fname);
if((outfile=fopen(filename, "w"))==NULL) {
	printf("Could not open %s for writing\n", filename);
	return(-1);
	}

Prnt("\\documentclass{article}\n", outfile);
Prnt("\\begin{document}\n", outfile);
sprintf(str, "                         CELESTIAL EMPIRE %d.%d\n", VERSION, PATCHLEVEL);
Prnt(str, outfile);
Prnt("                                  by\n", outfile);
Prnt("                             Zer Dwagon\n", outfile);
sprintf(str, "\nGame:%d\tTurn:%d\n", gm, turn);
Prnt(str, outfile);
TypeSummary(outfile);
OwnerSummary(outfile);
UnitSummary(outfile);
WinningDetails(outfile);
CostDetails(outfile);
PlanetSummary(NEUTPLR, outfile);
Prnt("\\begin{tabular}{rllllll}\n", outfile);
for(count=0;count<shiptr;count++) {
	ChekShip(count, NEUTPLR, outfile);
}
Prnt("\\end{tabular}\n", outfile);
Prnt("\\newpage\n", outfile);
Prnt("\\section*{Planets}\n", outfile);
for(count=0; count<NUMPLANETS; count++) {
	DoPlanet(count, NEUTPLR, outfile);
	}
Prnt("\\end{document}\n", outfile);
fclose(outfile);
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
void Prnt(const char *string, FILE *output)
/*****************************************************************************/
/* Print string to disk file */
{
    fprintf(output, "%s", string);
}

/*****************************************************************************/
void Process(Player plr)
/*****************************************************************************/
/* Process each players turn */
{
    char str[BUFSIZ];
    FILE *outfile;
    char filename[BUFSIZ];
    int count;

    TRTUR(printf("Process(plr:%d)\n", plr));

    /* Open file for outputing information */
    sprintf(filename, "%s%d/turn%d.%d.tex", path, gm, turn, plr);
    if((outfile=fopen(filename, "w"))==NULL) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        return;
	}

    /*******  HEADINGS ***************************************************/
    Headings(outfile, plr);
    /*************** RESEARCH TURN WARNING *****************************/
    if((turn-10)%4==0 && turn>9)
        ResOutput(plr, outfile);
    if(((turn-10)%4==3 && turn>=9) || turn==9) {
        Prnt("        Next turn is a research turn (Not this one)!\n", outfile);	
	}

    /*************** GLOBAL SHIP TYPE SUMMARY *****************************/
    if(turn % 2 == 0) {
        TypeSummary(outfile);
    }

    /*************** SHIP SUMMARY ****************************************/
    TRTUR(printf("Process:Ship Summary\n"));
    Prnt("\n\\section*{Summary of ships}\n", outfile);
    Prnt("\\begin{tabular}{rllllll}\n", outfile);
    for(count=0;count<shiptr;count++) {
        ChekShip(count, plr, outfile);
    }
    Prnt("\\end{tabular}\n", outfile);

    /*************** PLANET SUMMARY ***********************************/
    PlanetSummary(plr, outfile);

    /*********** ALLIANCE STATUS **********************************/
    Prnt("\\section*{Ally Status}\n", outfile);
    Prnt("\\begin{tabular}{lll}\n", outfile);
    Prnt("Empire Name & You are & They are\\\\ \\hline\n", outfile);
    for(count=1;count<NUMPLAYERS+1;count++) {
        if(count != plr && (alliance[plr][count] != NEUTRAL || alliance[count][plr] != NEUTRAL)) {
            sprintf(str, "%s & %s & %s \\\\\n", name[count], allname[alliance[plr][count]], allname[alliance[count][plr]]);
            Prnt(str, outfile);
            }
    }
    if(alliance[plr][plr]==ENEMY) {
        sprintf(str, "%s & & %s \\\\\n", name[count], allname[alliance[plr][plr]]);
        Prnt(str, outfile);
        }
    Prnt("\\end{tabular}\n", outfile);
    Prnt("\n", outfile);

    /******** EARTH DETAILS ***************************************/
    TRTUR(printf("Process:Earth Details\n"));
    Prnt("\\section*{Earth}\n", outfile);
    Prnt("\\begin{itemize}\n", outfile);
    if(turn<gamedet.earth.amnesty)
        Prnt("\\item Earth amnesty in effect. No shots allowed.\n", outfile);
    if(gamedet.earth.flag & WBUYALLORE)
        Prnt("\\item Unlimited Selling\n", outfile);
    if(gamedet.earth.flag & WBUY100ORE)
        Prnt("\\item Limited Selling\n", outfile);
    sprintf(str, "\\item Nearby planets: %d %d %d\n", galaxy[earth].link[0]+100, galaxy[earth].link[1]+100, galaxy[earth].link[2]+100);
    Prnt(str, outfile);
    Prnt("\\end{itemize}\n", outfile);
    Prnt("\\subsection*{Earth Trading Prices}\n", outfile);
    Prnt("\\begin{tabular}{rllllllllll}\n", outfile);
    Prnt("type", outfile);
    for(count=0;count<10;count++) {
        sprintf(str, " & %d", count);
        Prnt(str, outfile);
        }
    Prnt("\\\\\n", outfile);
    Prnt("price", outfile);
    for(count=0;count<10;count++) {
        sprintf(str, "& %d", price[count]);
        Prnt(str, outfile);
        }
    Prnt("\\\\\n", outfile);
    Prnt("amount", outfile);
    for(count=0;count<10;count++) {
        sprintf(str, " & %d", galaxy[earth].ore[count]);
        Prnt(str, outfile);
        }
    Prnt("\\\\\n", outfile);
    Prnt("\\end{tabular}\n", outfile);
    DoShip(plr, earth, outfile);

    /******** PLANET DETAILS **************************************/
    TRTUR(printf("******** PLANET DETAILS *******\n"));
    Prnt("\\newpage\n", outfile);
    Prnt("\\section*{Planets}\n", outfile);
    for(count=0;count<NUMPLANETS;count++)
        if(Interest(plr, count)==1) {
            DoPlanet(count, plr, outfile);
            }

    /******* TAIL NOTES ******************************************/
        CatMotd(outfile);
        CatSpec(outfile, plr);
        CatExhist(outfile, plr);
        if(((turn-11)%4==0) && (turn>10))
            ListRes(outfile);

    Prnt("\\end{document}\n", outfile);
    fclose(outfile);
}

/*****************************************************************************/
void CatExhist(FILE *output, Player plyr)
/*****************************************************************************/
/* Cat the execution history into the turn sheet */
{
    FILE *exechist;
    char filename[BUFSIZ];
    char command[BUFSIZ], buff[BUFSIZ];

    TRTUR(printf("CatExhist(plyr:%d)\n", plyr));
    sprintf(filename, "%s%d/exhist.%d", path, gm, plyr);
    if((exechist = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "CatExhist:Could not open exhist file: %s\n", filename);
        return;
        }
    Prnt("\\section*{Command history}\n", output);
    Prnt("\\begin{itemize}\n", output);
    for(;fgets(command, sizeof(command), exechist)!=NULL;) {
        sprintf(buff, "\\item %s", command);
        fputs(buff, output);
        }
    Prnt("\\end{itemize}\n", output);
    fclose(exechist);
}

/*****************************************************************************/
void ListRes(FILE *output)
/*****************************************************************************/
/* Give details of the research planets at the end of every research turn */
{
int count, count2;
char strng[BUFSIZ];

TRTUR(printf("ListRes(output)\n"));
Prnt("\n\nSUMMARY OF RESEARCH PLANETS.\n", output);
for(count=0;count<NUMPLANETS;count++) 
	if(IsResearch(count) && galaxy[count].owner!=NEUTPLR) {
		sprintf(strng, "%-3d: ", count+100);
		Prnt(strng, output);
		for(count2=0;count2<4;count2++) {
			if(galaxy[count].link[count2]>=0) {
				sprintf(strng, "%-3d ", galaxy[count].link[count2]+100);
				Prnt(strng, output);
				}
			else
				Prnt("    ", output);
			}
			sprintf(strng, "%-20s %-10s\n", galaxy[count].name, name[galaxy[count].owner]);
			Prnt(strng, output);
		}
return;
}

/*****************************************************************************/
void CatMotd(FILE *output)
/*****************************************************************************/
/* Cat the messages of the day to the end of the file */
{
FILE *motd;
char filename[BUFSIZ];
char strng[BUFSIZ];

TRTUR(printf("CatMotd(output)\n"));
sprintf(filename, "%s%d/motd", path, gm);
if((motd = fopen(filename, "r")) == NULL) {
	TRTUR(fprintf(stderr, "CatMotd:Could not open motd file:%s\n", filename));
	return;
	}
Prnt("\nGENERAL MESSAGES\n", output);
for(;fgets(strng, 80, motd)!=NULL;) {
	fputs(strng, output);
	}
fputs("\n", output);
fclose(motd);
return;
}

/*****************************************************************************/
void CatSpec(FILE *output, Player plr)
/*****************************************************************************/
/* Cat any special messages to that player to the end of file */
{
FILE *spec;
char strng[256];
char filename[BUFSIZ];

TRTUR(printf("CatSpec(output, plr:%d)\n", plr));
sprintf(filename, "%s%d/spec.%d", path, gm, plr);
if((spec=fopen(filename, "r"))==NULL) {
	TRTUR(fprintf(stderr, "CatSpec:Could not open spec file:%s\n", filename));
	return;
	}
Prnt("\nPERSONAL MESSAGES\n", output);
printf("Reading player %d's message\n", plr);
for(;fgets(strng, 256, spec)!=NULL;) {
	fputs(strng, output);
	}
fputs("\n", output);
fclose(spec);
return;
}

/*****************************************************************************/
int Interest(Player plr, Planet plan)
/*****************************************************************************/
/* Determine if the player has as interest in the planet, either has a ship
	over it or owns it, or scanned it that turn */
{
int r;

TRTUR2(printf("Interest(plr:%d, plan:%d)\n", plr, plan));
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
void DoPlanet(Planet plan, Player plr, FILE *output)
/*****************************************************************************/
/* Do the main part of the display */
{
    char str[BUFSIZ];
    int count;

    TRTUR2(printf("DoPlanet(plan:%d, plr:%d)\n", plan, plr));

    sprintf(str, "\\subsection*{%d %s", plan+100, galaxy[plan].name);
    Prnt(str, output);
    if(IsResearch(plan)) {
        Prnt(" --- Research Planet", output);
        }
    Prnt("}\n", output);
    if(galaxy[plan].scanned!=0) {
        Prnt("\\frame{Planet scanned this turn}\\ \n\n", output);
        }
    Prnt("\n", output);
    TRTUR(printf("plan:%d\t owner:%d\n", plan, galaxy[plan].owner));
    Prnt("\\begin{tabular}{r|llll}\n", output);
    sprintf(str, "Owner & \\multicolumn{4}{l}{%s}\\\\\n", name[galaxy[plan].owner]);
    Prnt(str, output);
    Prnt("Nearby Planets", output);
    for(count=0;count<4;count++) 
        if(galaxy[plan].link[count]>=0) {
            sprintf(str, "& %d", galaxy[plan].link[count]+100);
            Prnt(str, output);
        } else {
            Prnt(" & ", output);
            }
    Prnt("\\\\\n", output);
    sprintf(str, "Industry & Industry=%d & PDU=%d(%d) & Income=%d &\\\\\n", galaxy[plan].ind, galaxy[plan].pdu, Pdus(galaxy[plan].pdu), galaxy[plan].income);
    Prnt(str, output);
    sprintf(str, "Spacemines & Stored=%d & Deployed=%d & \\\\\n", galaxy[plan].spacemine, galaxy[plan].deployed);
    Prnt(str, output);
    Prnt("Standing Order & ", output);
    if(galaxy[plan].stndord[0]==0)
        sprintf(str, "\\multicolumn{4}{l}{None}\\\\\n");
    else 
        sprintf(str, "\\multicolumn{4}{l}{%d%s}\\\\\n", plan+100, galaxy[plan].stndord);
    Prnt(str, output);
    Prnt("\\end{tabular}\n\n", output);

    Prnt("\\begin{tabular}{r|cccccccccc}\n", output);
    Prnt("Mine Type", output);
    for(count=0;count<10;count++) {
        sprintf(str, "& %d", count);
        Prnt(str, output);
        }
    Prnt("\\\\ \\hline \n", output);
    Prnt("Amount stored", output);
    for(count=0;count<10;count++) {
        sprintf(str, "& %d", galaxy[plan].ore[count]);
		Prnt(str, output);
		}
    Prnt("\\\\\n", output);
    Prnt("Production", output);
    for(count=0;count<10;count++) {
        sprintf(str, "& %d", galaxy[plan].mine[count]);
        Prnt(str, output);
        }
    Prnt("\\\\\n", output);
    Prnt("\\end{tabular}\n", output);
    DoShip(plr, plan, output);
    return;
}

/*****************************************************************************/
void DoShip(Player plr, Planet plan, FILE *output)
/*****************************************************************************/
/* Display the ships above a certain planet */
{
    int count;

    TRTUR(printf("DoShip(plr:%d plan:%d)\n", plr, plan));
    for(count=0;count<shiptr;count++) {
        if(fleet[count].planet==plan) {
            if(fleet[count].owner==plr || plr==NEUTPLR || alliance[fleet[count].owner][plr]==ALLY) {
                DoFriend(count, output);
            }
            else {
                DoEnemy(count, output);
            }
        }
    }
    return;
}

/*****************************************************************************/
void DoEnemy(Ship shp, FILE *output)
/*****************************************************************************/
/* Print out details of enemy ships */
{
char str[BUFSIZ];

TRTUR(printf("DoEnemy(ship:%d)\n", shp));
sprintf(str, "S%3d %-9s: %-24s: %s\n", shp+100, name[fleet[shp].owner], fleet[shp].name, stypes[fleet[shp].type]);
Prnt(str, output);
TRTUR(printf("Finished DoEnemy()\n"));
return;
}

/*****************************************************************************/
void DoFriend(Ship shp, FILE *output)
/*****************************************************************************/
/* Print details of friendly ships */
{
    int count;
    char str[BUFSIZ];

    TRTUR(printf("DoFriend(ship:%d)\n", shp));
    Prnt("\n", output);
    Prnt("\\frame{\n", output);
    Prnt("\\begin{tabular}{rlll}\n", output);
    sprintf(str, "S%d & \\multicolumn{2}{l}{%s} & %s\\\\\n", shp+100, fleet[shp].name, stypes[fleet[shp].type]);
    Prnt(str, output);
    sprintf(str, "%s & f=%d & t=%d & s=%d(%d)\\\\\n", name[fleet[shp].owner], fleet[shp].fight, fleet[shp].tractor, fleet[shp].shield, Shields(shp));
    Prnt(str, output);
    sprintf(str, " & cargo=%d & cargoleft=%d & \\\\\n", fleet[shp].cargo, fleet[shp].cargleft);
    Prnt(str, output);
    sprintf(str, " & eff=%d(%d) & shots=%d & \\\\\n", fleet[shp].efficiency, EffEff(shp), Shots(shp, fleet[shp].fight));
    Prnt(str, output);

    Prnt("Standing & \\multicolumn{3}{l}{", output);
    if(fleet[shp].stndord[0]==0) {
        Prnt("None", output);
    }
    else {
        sprintf(str, "S%d%s", shp+100, fleet[shp].stndord);
        Prnt(str, output);
    };
    Prnt("}\\\\\n", output);

    /* Print out cargo details */
    Prnt("Cargo & \\multicolumn{3}{l}{", output);
    if(fleet[shp].ind!=0) {
        sprintf(str, "Ind %d", fleet[shp].ind);
        Prnt(str, output);
        }
    if(fleet[shp].mines!=0) {
        sprintf(str, "Mine %d", fleet[shp].mines);
        Prnt(str, output);
        }
    if(fleet[shp].pdu!=0) {
        sprintf(str, "PDU %d", fleet[shp].pdu);
        Prnt(str, output);
        }
    if(fleet[shp].spacemines!=0) {
        sprintf(str, "SpcMines %d", fleet[shp].spacemines);
        Prnt(str, output);
        }
    for(count=0;count<10;count++)
        if(fleet[shp].ore[count]!=0) {
            sprintf(str, "R%d %d", count, fleet[shp].ore[count]);
            Prnt(str, output);
            }
    Prnt("}\\\\\n", output);
    Prnt("\\end{tabular}\n", output);
    Prnt("}\n", output);
    return;
}

/*****************************************************************************/
int ChekPlan(Planet plan, Player plr, FILE *output)
/*****************************************************************************/
/* Get details of the planet for the planet summary */
{
    char str[BUFSIZ];
    int count;

    TRTUR2(printf("ChekPlan(plan:%d, plr:%d)\n", plan, plr));
    if(galaxy[plan].owner!=plr && plr!=NEUTPLR)
        return(0);
    sprintf(str, "%d &", plan+100);
    Prnt(str, output);
    for(count=0;count<10;count++) {
        if(galaxy[plan].mine[count]==0 && galaxy[plan].ore[count]==0) {
            sprintf(str, " / &");
            }
        else if(galaxy[plan].mine[count]==0) {
            sprintf(str, " /%d &", galaxy[plan].ore[count]);
            }
        else if(galaxy[plan].ore[count]==0) {
            sprintf(str, "%d/ &", galaxy[plan].mine[count]);
            }
        else {
            sprintf(str, "%d/%d &", galaxy[plan].mine[count], galaxy[plan].ore[count]);
            }
        Prnt(str, output);
        }
    if(galaxy[plan].pdu==0)
        Prnt(" --- &", output);
    else {
        sprintf(str, " %d &", galaxy[plan].pdu);
        Prnt(str, output);
        }
    if(galaxy[plan].ind==0)
        Prnt(" --- \\\\\n", output);
    else {
        sprintf(str, " %d\\\\\n", galaxy[plan].ind);
        Prnt(str, output);
        }
    return(1);
}

/*****************************************************************************/
void ChekTot(Player plr, FILE *output)
/*****************************************************************************/
/* Print out total amounts of ore in players empire */
{
    Amount tore[10], tmin[10], tind, tpdu;
    int count, count2;
    char str[BUFSIZ];

    TRTUR(printf("ChekTot(plr:%d)\n", plr));
    for(count=0;count<10;count++) {
        tore[count]=0;
        tmin[count]=0;
        }
    tind=tpdu=0;

    for(count=0;count<NUMPLANETS;count++)
        if(galaxy[count].owner==plr || plr==NEUTPLR) {
            for(count2=0; count2<10; count2++) {
                tore[count2] += galaxy[count].ore[count2];
                tmin[count2] += galaxy[count].mine[count2];
                }
            tpdu += galaxy[count].pdu;
            tind += galaxy[count].ind;
        }

    for(count=0; count<shiptr; count++)
        if(fleet[count].owner==plr || plr==NEUTPLR) {
            for(count2=0; count2<10; count2++) {
                tore[count2] += fleet[count].ore[count2];
                }
            tpdu += fleet[count].pdu;
            tind += fleet[count].ind;
        }

    sprintf(str, "Total: &");
    Prnt(str, output);
    for(count=0;count<10;count++) {
        sprintf(str, "%d/%d &", tmin[count], tore[count]);
        Prnt(str, output);
        }
    sprintf(str, "%d &", tpdu);
    Prnt(str, output);
    sprintf(str, "%d\\\\ \\hline \n", tind);
    Prnt(str, output);
}

/*****************************************************************************/
void ChekShip(Ship shp, Player plr, FILE *output)
/*****************************************************************************/
/* Get details of ship for ship summary */
{
    char str[BUFSIZ];
    int count;

    if(fleet[shp].owner!=plr && plr!=NEUTPLR) 
        return;

    TRTUR2(printf("ChekShip(ship:%d, plr:%d)\n", shp, plr));
    /* Print ship details */
    sprintf(str, "S%d & f=%d & c=%d(%d) & t=%d & s=%d(%d)", shp+100, fleet[shp].fight, fleet[shp].cargo, fleet[shp].cargleft, fleet[shp].tractor, fleet[shp].shield, Shields(shp));
    Prnt(str, output);
    sprintf(str, " & eff=%-1d(%d) & shots=%d", fleet[shp].efficiency, EffEff(shp), Shots(shp, fleet[shp].fight));
    Prnt(str, output);
    sprintf(str, "%s\\\\\n", fleet[shp].name);
    Prnt(str, output);
    sprintf(str, "& Planet: %d & \\multicolumn{3}{l}{%s} & Standing: & ", fleet[shp].planet+100, stypes[fleet[shp].type]);
    Prnt(str, output);
    if(fleet[shp].stndord[0]==0)
        sprintf(str, "None \\\\\n");
    else
        sprintf(str, "S%d%s \\\\\n", shp+100, fleet[shp].stndord);
    Prnt(str, output);
    /* Print out cargo details */
    Prnt("& \\multicolumn{6}{l}{", output);
    if(fleet[shp].ind!=0) {
        sprintf(str, "I %d ", fleet[shp].ind);
        Prnt(str, output);
        }
    if(fleet[shp].mines!=0) {
        sprintf(str, "M %d ", fleet[shp].mines);
        Prnt(str, output);
        }
    if(fleet[shp].pdu!=0) {
        sprintf(str, "D %d ", fleet[shp].pdu);
        Prnt(str, output);
        }
    if(fleet[shp].spacemines!=0) {
        sprintf(str, "SM %d ", fleet[shp].spacemines);
        Prnt(str, output);
        }
    for(count=0;count<10;count++)
        if(fleet[shp].ore[count]!=0) {
            sprintf(str, "R%d %d.", count, fleet[shp].ore[count]);
            Prnt(str, output);
            }
    Prnt("}\\\\\n", output);
    return;
}

/*****************************************************************************/
void ResOutput(Player plr, FILE *output)
/*****************************************************************************/
{
    char str[BUFSIZ];
    int nres, count;

    TRTUR(printf("Process:Research turn\n"));
    nres=NumRes(plr);
    Prnt("\nRemember to give research orders this turn\n", output);
    sprintf(str, "You have %d research planet%c\n", nres, nres!=1?'s':' ');
    Prnt(str, output);
    if(nres==0)
        return;
    if(nres==1) {
        Prnt("It is ", output);
        }
    else {
        Prnt("They are ", output);
        }
    for(count=0;count<NUMPLANETS;count++) 
        if(IsResearch(count) && galaxy[count].owner==plr) {
            sprintf(str, " %d", count+100);
            Prnt(str, output);
        }
        Prnt(".\n", output);
    return;
}

/*****************************************************************************/
void TypeSummary(FILE *outfile)
/*****************************************************************************/
{
    char str[BUFSIZ];
    int x, types[NUMTYPES];

    TRTUR(printf("TypeSummary\n"));
    for(x=0;x<NUMTYPES;x++)		/* Clear the types */
        types[x]=0;

    for(x=0;x<shiptr;x++)
        types[fleet[x].type]++;	/* Work out how many of each type */
        
    Prnt("\nSUMMARY OF SHIP TYPES\n", outfile);
    for(x=0;x<NUMTYPES-(NUMTYPES%2);x+=2) {
        sprintf(str, "%-15s %d\t\t%-15s %d\n", stypes[x], types[x], stypes[x+1], types[x+1]);
        Prnt(str, outfile);
        }
    if(NUMTYPES%2!=0) {
        sprintf(str, "%-15s %d\n", stypes[NUMTYPES-1], types[NUMTYPES-1]);
        Prnt(str, outfile);
        }
    Prnt("\n", outfile);
}

/*****************************************************************************/
void Headings(FILE *outfile, Player plr)
/*****************************************************************************/
{
char str[BUFSIZ];
int income=CalcPlrInc(plr);

TRTUR(printf("Headings\n"));
Prnt("\\documentclass{article}\n", outfile);
Prnt("\\begin{document}\n", outfile);
sprintf(str, "\\title{Celestial Empire %d.%d\\\\Game %d}\n", VERSION, PATCHLEVEL, gm);
Prnt(str, outfile);
Prnt("\\author{Dougal Scott <dougal.scott@gmail.com>}\n", outfile);
Prnt("\n", outfile);
/* Print out name of player and number */
sprintf(str, "\\large{%s turn %d}\n", name[plr], turn);
Prnt(str, outfile);
/* Print out score and gm and turn numbers */
Prnt("\\begin{itemize}\n", outfile);
sprintf(str, "\\item score=%d\n", score[plr]);
Prnt(str, outfile);
/* Print out due date, and player scores */
sprintf(str, "\\item date due= before %-8s\n" , duedate);
Prnt(str, outfile);
sprintf(str, "\\item your income=%4d\n", income);
Prnt(str, outfile);
sprintf(str, "\\item Earth credits=%4d \n", ecredit[plr]);
Prnt(str, outfile);
sprintf(str, "\\item Credits:Score=%3d \n", gamedet.earth.earthmult);
Prnt(str, outfile);
Prnt("\\end{itemize}\n", outfile);

Prnt("\\begin{tabular}{c|c|c}\n", outfile);
Prnt("\\multicolumn{3}{c}{Player Scores}\\\\ \n", outfile);
sprintf(str, "%6d & %6d & %6d\\\\ \n" , score[1], score[2], score[3]);
Prnt(str, outfile);
sprintf(str, "%6d & %6d & %6d\\\\ \n", score[4], score[5], score[6]);
Prnt(str, outfile);
sprintf(str, "%6d & %6d & %6d\\\\ \n", score[7], score[8], score[9]);
Prnt(str, outfile);
Prnt("\\end{tabular}\n", outfile);

/* Print winning conditions */
Prnt("Winning Conditions:  ", outfile);
Prnt("\\begin{itemize}\n", outfile);
if(gamedet.winning&WEARTH) {
	Prnt("\\item Earth\n", outfile);
	}
if(gamedet.winning&WCREDIT) {
	sprintf(str, "\\item Credits=%d\n", gamedet.credits);
	Prnt(str, outfile);
	}
if(gamedet.winning&WINCOME) {
	sprintf(str, "\\item Income=%d\n", gamedet.income);
	Prnt(str, outfile);
	}
if(gamedet.winning&WSCORE) {
	sprintf(str, "\\item Score=%d\n", gamedet.score);
	Prnt(str, outfile);
	}
if(gamedet.winning&WPLANETS) {
	sprintf(str, "\\item Planets=%d\n", gamedet.planets);
	Prnt(str, outfile);
	}
if(gamedet.winning&WTURN) {
	sprintf(str, "\\item Turn=%d\n", gamedet.turn);
	Prnt(str, outfile);
	if(!(gamedet.winning&WFIXTURN)) {
		sprintf(str, "\\item Desired end turn=%d\n", desturn[plr]);
		Prnt(str, outfile);
		}
	}
Prnt("\\end{itemize}\n", outfile);

sprintf(str, "        Minimum bids: Cargo=%d Fighter=%d Shield=%d Tractor=%d\n", gamedet.earth.cbid, gamedet.earth.fbid, gamedet.earth.sbid, gamedet.earth.tbid);
Prnt(str, outfile);
sprintf(str, "        You have %d scans this turn\n", NumRes(plr)+1);
Prnt(str, outfile);
Prnt("\\newpage\n", outfile);
}

/*****************************************************************************/
void PlanetSummary(Player plr, FILE *outfile)
/*****************************************************************************/
{
    char str[BUFSIZ];
    int count, total=0;

    TRTUR(printf("Process:Planet Summary\n"));
    Prnt("\\section*{Summary of planets}\n", outfile);
    Prnt("\\begin{tabular}{rcccccccccccc}\n", outfile);
    Prnt("planet &", outfile);
    for(count=0; count<10; count++) {
        sprintf(str, "%d &", count);
        Prnt(str, outfile);
        }
    Prnt("PDU &", outfile);
    Prnt("IND \\\\ \\hline\n", outfile);
    for(count=0; count<NUMPLANETS; count++) {
        total += ChekPlan(count, plr, outfile);
        }
    Prnt("\\hline\n", outfile);
    ChekShipTot(plr, outfile);
    ChekTot(plr, outfile);
    Prnt("\\end{tabular}\n", outfile);

    sprintf(str, "Total number of planets owned = %d\n", total);
    Prnt(str, outfile);
}

/*****************************************************************************/
void OwnerSummary(FILE *output)
/*****************************************************************************/
/* Print ownership statistix to file output */
{
int planown[11];	/* Who owns how many planets */
int indown[11];		/* Who owns how much industry */
int pduown[11];		/* Who owns how many pdus */
int resplan[11];	/* How many research planets */
int count, count2, totinc=0, totscore=0;

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
fprintf(output, "OWNERSHIP STATISTIX\n");
fprintf(output, "Empire     Plnets   ResPlan  Indust   Income   Score    PDUs\n");
for(count=0;count<NUMPLAYERS+1;count++) {
	fprintf(output, "%-10s %-8d %-8d %-8d", name[count], planown[count], resplan[count], indown[count]);
	fprintf(output, " %-8d %-8d %-8d\n", CalcPlrInc(count), score[count], pduown[count]);
	}
fprintf(output, "%-10s %-8d %-8d %-8d", "Total", planown[10], resplan[10], indown[10]);
fprintf(output, " %-8d %-8d %-8d\n", totinc, totscore, pduown[10]);
}

/*****************************************************************************/
void UnitSummary(FILE *output)
/*****************************************************************************/
/* Calculate how many ship units each player has */
{
int fgtown[11], crgown[11], shdown[11], tracown[11], shpown[11];
int count, count2;

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
fprintf(output, "\nSHIP STATISTIX\n");
fprintf(output, "Empire     Ships    Fghtrs   Cargo    Shield   Tractor\n");
for(count=0;count<NUMPLAYERS+1;count++) {
	fprintf(output, "%-10s %-8d %-8d %-8d", name[count], shpown[count], fgtown[count], crgown[count]);
	fprintf(output, " %-8d %-8d\n", shdown[count], tracown[count]);
	}
fprintf(output, "%-10s %-8d %-8d %-8d", "Total", shpown[10], fgtown[10], crgown[10]);
fprintf(output, " %-8d %-8d\n", shdown[10], tracown[10]);
}

/*****************************************************************************/
void WinningDetails(FILE *output)
/*****************************************************************************/
/* Print the winning details, and all info from the game structure */
{
int cnt;

TRTUR(printf("WinningDetails()\n"));
fprintf(output, "\nGAME DETAILS\n");

/* Print winning conditions */
fprintf(output, "Winning Conditions:  ");
if(gamedet.winning&WEARTH) {
	fprintf(output, "Earth  ");
	}
if(gamedet.winning&WSCORE) {
	fprintf(output, "Score=%d  ", gamedet.score);
	}
if(gamedet.winning&WINCOME) {
	fprintf(output, "Income=%d  ", gamedet.income);
	}
if(gamedet.winning&WPLANETS) {
	fprintf(output, "Planets=%d", gamedet.planets);
	}

/* Print details of initial home planet */
fprintf(output, "\nHome Details:\n");
fprintf(output, "\tInd=%d  PDU=%d  Spacemines=%d  Deployed=%d\n", gamedet.home.ind, gamedet.home.pdu, gamedet.home.spacemine, gamedet.home.deployed);
fprintf(output, "\tOre Type: ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", cnt);
	}
fprintf(output, "\n\tAmount:   ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", gamedet.home.ore[cnt]);
	}
fprintf(output, "\n\tProd:     ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", gamedet.home.mine[cnt]);
	}

/* Print details of initial Earth planet */
fprintf(output, "\nEarth Details:\n");
fprintf(output, "\tAmnesty=%d  Earthmult=%d  Bids: Fght=%d  Carg=%d  Shld=%d  Trac=%d\n", gamedet.earth.amnesty, gamedet.earth.earthmult, gamedet.earth.fbid, gamedet.earth.cbid, gamedet.earth.sbid, gamedet.earth.tbid);
fprintf(output, "\tInd=%d  PDU=%d  Spacemines=%d  Deployed=%d\n", gamedet.earth.ind, gamedet.earth.pdu, gamedet.earth.spacemine, gamedet.earth.deployed);
fprintf(output, "\tOre Type: ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", cnt);
	}
fprintf(output, "\n\tAmount:   ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", gamedet.earth.ore[cnt]);
	}
fprintf(output, "\n\tProd:     ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", gamedet.earth.mine[cnt]);
	}
fprintf(output, "\n");
return;
}

/*****************************************************************************/
void CostDetails(FILE *output)
/*****************************************************************************/
/* Print the Earth ore cost */
{
int cnt;

TRTUR(printf("CostDetails()\n"));
fprintf(output, "\nORE PRICES\n");
fprintf(output, "Ore Type: ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", cnt);
	}
fprintf(output, "\nCost:     ");
for(cnt=0;cnt<10;cnt++) {
	fprintf(output, "%-3d ", price[cnt]);
	}
return;
}

/*****************************************************************************/
void ChekShipTot(Player plr, FILE *output)
/*****************************************************************************/
/* Print the ore totals that are stored on ships */
{
char str[BUFSIZ];
int count, tmp, oretot[12];

TRTUR2(printf("ChekShipTot(plr:%d)\n", plr));

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
sprintf(str, "Ships: &");
Prnt(str, output);
for(tmp=0;tmp<10;tmp++) {
	if(oretot[tmp]>0) {
		sprintf(str, " /%d &", oretot[tmp]);
		Prnt(str, output);
		}
	else {
		sprintf(str, " / &");
		Prnt(str, output);
		}
	}
	sprintf(str, " %d&", oretot[11]);
	Prnt(str, output);
	sprintf(str, " %d\\\\\n", oretot[10]);
	Prnt(str, output);
return;
}
