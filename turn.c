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

/*****************************************************************************/
int main(int argc, char **argv)
/*****************************************************************************/
/* Do the lot */
{
    Player plr;
    char *gmstr;

    if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
        dbgstr=(char *)"null";

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
    FILE *output, *dotfile;
    int count;
    char filename[BUFSIZ];

    TRTUR(printf("PrintGalaxy\n"));
    sprintf(filename, "%s", fname);
    if((output=fopen(filename, "w"))==NULL) {
        printf("Could not open %s for writing\n", filename);
        return(-1);
        }

    sprintf(filename, "%s.dot", fname);
    if((dotfile=fopen(filename, "w"))==NULL) {
        printf("Could not open %s for writing\n", filename);
        return(-1);
        }
    fprintf(dotfile, "strict graph G {\n");
    TitlePage(output);

    fprintf(output, "\nGame:%d\tTurn:%d\n", gm, turn);
    TypeSummary(output);
    OwnerSummary(output);
    UnitSummary(output);
    WinningDetails(output);
    CostDetails(output);
    PlanetSummary(NEUTPLR, output);
    ShipSummary(NEUTPLR, output);

    fprintf(output, "\n");
    fprintf(output, "\\section*{Planets}\n");
    for(count=0; count<NUMPLANETS; count++) {
        GraphPlanet(count, NEUTPLR, dotfile);
        DoPlanet(count, NEUTPLR, output);
        }
    fprintf(output, "\\end{document}\n");
    fprintf(dotfile, "}\n");
    fclose(output);
    fclose(dotfile);
    return(0);
}

/*****************************************************************************/
void TitlePage(FILE *output)
/*****************************************************************************/
{
    fprintf(output, "\\documentclass{article}\n");
    fprintf(output, "\\usepackage{longtable, graphicx, epstopdf, changepage, a4wide}\n");
    fprintf(output, "\\title{Celestial Empire %d.%d Game %d}\n", VERSION, PATCHLEVEL, gm);
    fprintf(output, "\\author{Dougal Scott $<$dougal.scott@gmail.com$>$}\n");
    fprintf(output, "\\begin{document}\n");
    fprintf(output, "\\maketitle\n");
}

/*****************************************************************************/
void GraphPlanet(Planet plan, Player plr, FILE *dotfile)
/*****************************************************************************/
{
    fprintf(dotfile, "%d [ label=\"%d %s", plan+100, plan+100, galaxy[plan].name);
    if(IsResearch(plan)) {
        fprintf(dotfile, "(RP)");
    }
    fprintf(dotfile, "\";");
    if(galaxy[plan].owner != NEUTPLR) {
        fprintf(dotfile, "shape=\"rectangle\";");
    }
    if(galaxy[plan].spec == -plr) {
        fprintf(dotfile, "shape=\"square\"; color=\"dodgerblue3\"; style=\"filled\"");
    }
    else {
        if(galaxy[plan].spec < 0) {
            fprintf(dotfile, "shape=\"square\"; color=\"firebrick1\"; style=\"filled\"");
        }
    }
    if(plan==earth) {
        fprintf(dotfile, "shape=\"hexagon\"; color=\"chartreuse\"; style=\"filled\"");
    }

    // Colour up "A" ring planets as well
    for(int count=0; count<4; count++) {
        if(galaxy[galaxy[plan].link[count]].spec == -plr) {
            fprintf(dotfile, "color=\"dodgerblue1\"; style=\"filled\"");
        }
    }

    // Colour up Earth "A" ring planets as well
    for(int count=0; count<4; count++) {
        if(galaxy[plan].link[count] == earth) {
            fprintf(dotfile, "color=\"chartreuse3\"; style=\"filled\"");
        }
    }
    fprintf(dotfile, "];\n");

    // Planet links
    for(int count=0; count<4; count++) {
        if(galaxy[plan].link[count]>=0) {
            fprintf(dotfile, "%d -- %d;\n", plan+100, galaxy[plan].link[count]+100);
        }
    }

    // Ships
    for(int ship=0; ship < shiptr; ship++) {
        if (fleet[ship].planet == plan) {
            if (fleet[ship].owner == plr || plr == NEUTPLR) {
                fprintf(dotfile, "S%d [label=\"S%d\"; shape=\"hexagon\"; style=\"filled\";];\n", ship+100, ship+100);
                fprintf(dotfile, "S%d -- %d;\n", ship+100, plan+100);
            }
        else {
            fprintf(dotfile, "S%d [label=\"S%d\"; shape=\"hexagon\"; style=\"filled\"; color=\"firebrick2\";];\n", ship+100, ship+100);
            fprintf(dotfile, "S%d -- %d;\n", ship+100, plan+100);
            }
        }
    }
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
void Process(Player plr)
/*****************************************************************************/
/* Process each players turn */
{
    FILE *output, *dotfile;
    char filename[BUFSIZ], turnsheet[BUFSIZ];

    TRTUR(printf("Process(plr:%d)\n", plr));

    /* Open file for outputing information */
    sprintf(turnsheet, "turn%d.%d.tex", turn, plr);
    FilePath(turnsheet, filename);
    if((output=fopen(filename, "w"))==NULL) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        return;
	}

    /* Open file for outputing dot information */
    sprintf(turnsheet, "turn%d.%d.dot", turn, plr);
    FilePath(turnsheet, filename);
    if((dotfile=fopen(filename, "w"))==NULL) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        return;
	}

    /*******  HEADINGS ***************************************************/
    Headings(output, dotfile, plr);
    /*************** RESEARCH TURN WARNING *****************************/
    if((turn-10)%4==0 && turn>9) {
        ResOutput(plr, output);
    }
    if(((turn-10)%4==3 && turn>=9) || turn==9) {
        fprintf(output, " Next turn is a research turn (Not this one)!\n");	
	}

    /*************** GLOBAL SHIP TYPE SUMMARY *****************************/
    if(turn % 2 == 0) {
        TypeSummary(output);
    }

    ShipSummary(plr, output);
    PlanetMap(plr, output);
    PlanetSummary(plr, output);
    AllianceStatus(plr, output);
    EarthDetails(plr, output);

    /******** PLANET DETAILS **************************************/
    fprintf(output, "\n");
    fprintf(output, "\\section*{Planets}\n");
    for(int pln=0; pln<NUMPLANETS; pln++) {
        if(galaxy[pln].knows[plr]==1) {
            GraphPlanet(pln, plr, dotfile);
        }
        if(Interest(plr, pln)==1) {
            DoPlanet(pln, plr, output);
        }
    }

    /******* TAIL NOTES ******************************************/
    CatMotd(output);
    CatSpec(output, plr);
    CatExhist(output, plr);
    if(((turn-11)%4==0) && (turn>10))
        ListRes(output);

    fprintf(output, "\\end{document}\n");
    fprintf(dotfile, "}\n");
    fclose(dotfile);
    fclose(output);
}

/*****************************************************************************/
void ShipSummary(Player plr, FILE *output)
/*****************************************************************************/
{
    fprintf(output, "\n");
    fprintf(output, "\n\\section*{Summary of ships}\n");
    fprintf(output, "\\begin{longtable}{rllllll}\n");
    for(int count=0;count<shiptr;count++) {
        ChekShip(count, plr, output);
    }
    fprintf(output, "\\end{longtable}\n");
}

/*****************************************************************************/
void AllianceStatus(Player plr, FILE *output)
/*****************************************************************************/
{
    fprintf(output, "\n");
    fprintf(output, "\\section*{Ally Status}\n");
    fprintf(output, "\\begin{tabular}{lll}\n");
    fprintf(output, "Empire Name & You are & They are\\\\ \\hline\n");
    for(int count=1;count<NUMPLAYERS+1;count++) {
        if(count != plr && (alliance[plr][count] != NEUTRAL || alliance[count][plr] != NEUTRAL)) {
            fprintf(output, "%s & %s & %s \\\\\n", name[count], allname[alliance[plr][count]], allname[alliance[count][plr]]);
            }
        if(alliance[plr][plr]==ENEMY) {
            fprintf(output, "%s & & %s \\\\\n", name[count], allname[alliance[plr][plr]]);
        }
    }
    fprintf(output, "\\end{tabular}\n");
    fprintf(output, "\n");

}

/*****************************************************************************/
void EarthDetails(Player plr, FILE *output)
/*****************************************************************************/
{
    fprintf(output, "\n");
    fprintf(output, "\\section*{Earth}\n");
    fprintf(output, "\\begin{itemize}\n");
    fprintf(output, "\\item Planet: %d\n", earth+100);
    if(turn<gamedet.earth.amnesty)
        fprintf(output, "\\item Earth amnesty in effect. No shots allowed.\n");
    if(gamedet.earth.flag & WBUYALLORE)
        fprintf(output, "\\item Unlimited Selling\n");
    if(gamedet.earth.flag & WBUY100ORE)
        fprintf(output, "\\item Limited Selling\n");
    fprintf(output, "\\item Nearby planets: %d %d %d\n", galaxy[earth].link[0]+100, galaxy[earth].link[1]+100, galaxy[earth].link[2]+100);
    fprintf(output, "\\end{itemize}\n");
    fprintf(output, "\\subsection*{Earth Trading Prices}\n");
    fprintf(output, "\\begin{tabular}{rllllllllll}\n");
    fprintf(output, "type");
    for(int count=0;count<10;count++) {
        fprintf(output, " & %d", count);
        }
    fprintf(output, "\\\\\n");
    fprintf(output, "price");
    for(int count=0;count<10;count++) {
        fprintf(output, "& %d", price[count]);
        }
    fprintf(output, "\\\\\n");
    fprintf(output, "amount");
    for(int count=0;count<10;count++) {
        fprintf(output, " & %d", galaxy[earth].ore[count]);
        }
    fprintf(output, "\\\\\n");
    fprintf(output, "\\end{tabular}\n");
    DoShip(plr, earth, output);
}


/*****************************************************************************/
void CatExhist(FILE *output, Player plr)
/*****************************************************************************/
/* Cat the execution history into the turn sheet */
{
    FILE *exechist;
    char filename[BUFSIZ];
    char command[BUFSIZ];
    int commands = 0;

    PlrFile("exhist", plr, filename);
    if((exechist = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "CatExhist:Could not open exhist file: %s\n", filename);
        return;
        }
    fprintf(output, "\n");
    fprintf(output, "\\section*{Command history}\n");
    fprintf(output, "\\begin{itemize}\n");
    for(;fgets(command, sizeof(command), exechist)!=NULL;) {
        fprintf(output, "\\item %s", command);
        commands++;
        }
    if (commands == 0) {
        fprintf(output, "\\item No commands entered\n");
    }
    fprintf(output, "\\end{itemize}\n");
    fclose(exechist);
}

/*****************************************************************************/
void ListRes(FILE *output)
/*****************************************************************************/
/* Give details of the research planets at the end of every research turn */
{
int count, count2;

TRTUR(printf("ListRes(output)\n"));
fprintf(output, "\n\nSUMMARY OF RESEARCH PLANETS.\n");
for(count=0;count<NUMPLANETS;count++) 
	if(IsResearch(count) && galaxy[count].owner!=NEUTPLR) {
		fprintf(output, "%-3d: ", count+100);
		for(count2=0;count2<4;count2++) {
			if(galaxy[count].link[count2]>=0) {
				fprintf(output, "%-3d ", galaxy[count].link[count2]+100);
				}
			else
				fprintf(output, "    ");
			}
			fprintf(output, "%-20s %-10s\n", galaxy[count].name, name[galaxy[count].owner]);
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

    FilePath("motd", filename);
    if((motd = fopen(filename, "r")) == NULL) {
        TRTUR(fprintf(stderr, "CatMotd:Could not open motd file:%s\n", filename));
        return;
        }
    fprintf(output, "\n");
    fprintf(output, "\\section*{Messages}\n");
    fprintf(output, "\\begin{verbatim}\n");
    for(;fgets(strng, 80, motd)!=NULL;) {
        fputs(strng, output);
        }
    fprintf(output, "\\end{verbatim}\n");
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

    PlrFile("spec", plr, filename);

    if((spec=fopen(filename, "r"))==NULL) {
        TRTUR(fprintf(stderr, "CatSpec:Could not open spec file:%s\n", filename));
        return;
        }
    fprintf(output, "\nPERSONAL MESSAGES\n");
    printf("Reading player %d's message\n", plr);
    for(;fgets(strng, 256, spec) != NULL;) {
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
    int count;

    TRTUR2(printf("DoPlanet(plan:%d, plr:%d)\n", plan, plr));

    fprintf(output, "\\subsection*{%d %s", plan+100, galaxy[plan].name);
    if(IsResearch(plan)) {
        fprintf(output, " --- Research Planet");
        }
    fprintf(output, "}\n");
    fprintf(output, "\n");
    TRTUR(printf("plan:%d\t owner:%d\n", plan, galaxy[plan].owner));
    fprintf(output, "\\begin{tabular}{r|llll}\n");
    fprintf(output, "Owner & \\multicolumn{4}{l}{%s}\\\\\n", name[galaxy[plan].owner]);
    if(galaxy[plan].scanned!=0) {
        fprintf(output, "Scanned & \\multicolumn{4}{l}{Planet scanned this turn}\\\\\n");
    };
    fprintf(output, "Nearby Planets");
    for(count=0;count<4;count++) 
        if(galaxy[plan].link[count]>=0) {
            fprintf(output, "& %d", galaxy[plan].link[count]+100);
        } else {
            fprintf(output, " & ");
            }
    fprintf(output, "\\\\\n");
    fprintf(output, "Industry & Industry=%d & PDU=%d(%d) & Income=%d &\\\\\n", galaxy[plan].ind, galaxy[plan].pdu, Pdus(galaxy[plan].pdu), galaxy[plan].income);
    fprintf(output, "Spacemines & Stored=%d & Deployed=%d & \\\\\n", galaxy[plan].spacemine, galaxy[plan].deployed);
    fprintf(output, "Standing Order & ");
    if(galaxy[plan].stndord[0]==0)
        fprintf(output, "\\multicolumn{4}{l}{None}\\\\\n");
    else 
        fprintf(output, "\\multicolumn{4}{l}{%d%s}\\\\\n", plan+100, galaxy[plan].stndord);
    fprintf(output, "\\end{tabular}\n\n");

    fprintf(output, "\\begin{tabular}{r|cccccccccc}\n");
    fprintf(output, "Mine Type");
    for(count=0;count<10;count++) {
        fprintf(output, "& %d", count);
        }
    fprintf(output, "\\\\ \\hline \n");
    fprintf(output, "Amount stored");
    for(count=0;count<10;count++) {
        fprintf(output, "& %d", galaxy[plan].ore[count]);
		}
    fprintf(output, "\\\\\n");
    fprintf(output, "Production");
    for(count=0;count<10;count++) {
        fprintf(output, "& %d", galaxy[plan].mine[count]);
        }
    fprintf(output, "\\\\\n");
    fprintf(output, "\\end{tabular}\n");
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
    TRTUR(printf("DoEnemy(ship:%d)\n", shp));
    fprintf(output, "\n");
    fprintf(output, "\\frame{\\\n");
    fprintf(output, "\\begin{tabular}{rlll}\n");
    fprintf(output, "S%3d & %s & %s: & %s \\\\ \n", shp+100, name[fleet[shp].owner], fleet[shp].name, stypes[fleet[shp].type]);
    fprintf(output, "\\end{tabular}\n");
    fprintf(output, "}\n");
    return;
}

/*****************************************************************************/
void DoFriend(Ship shp, FILE *output)
/*****************************************************************************/
/* Print details of friendly ships */
{
    int count;
    int has_cargo = 0;

    TRTUR(printf("DoFriend(ship:%d)\n", shp));
    fprintf(output, "\n");
    fprintf(output, "\\frame{\\\n");
    fprintf(output, "\\begin{tabular}{rlll}\n");
    fprintf(output, "S%d & \\multicolumn{2}{l}{%s} & %s\\\\\n", shp+100, fleet[shp].name, stypes[fleet[shp].type]);
    fprintf(output, "%s & f=%d & t=%d & s=%d(%d)\\\\\n", name[fleet[shp].owner], fleet[shp].fight, fleet[shp].tractor, fleet[shp].shield, Shields(shp));
    fprintf(output, " & cargo=%d & cargoleft=%d & \\\\\n", fleet[shp].cargo, fleet[shp].cargleft);
    fprintf(output, " & eff=%d(%d) & shots=%d & \\\\\n", fleet[shp].efficiency, EffEff(shp), Shots(shp, fleet[shp].fight));

    fprintf(output, "Standing & \\multicolumn{3}{l}{");
    if(fleet[shp].stndord[0]==0) {
        fprintf(output, "None");
    }
    else {
        fprintf(output, "S%d%s", shp+100, fleet[shp].stndord);
    };
    fprintf(output, "}\\\\\n");

    /* Print out cargo details */
    fprintf(output, "Cargo & \\multicolumn{3}{l}{");
    if(fleet[shp].ind!=0) {
        fprintf(output, "Ind %d;", fleet[shp].ind);
        has_cargo = 1;
        }
    if(fleet[shp].mines!=0) {
        fprintf(output, "Mine %d;", fleet[shp].mines);
        has_cargo = 1;
        }
    if(fleet[shp].pdu!=0) {
        fprintf(output, "PDU %d;", fleet[shp].pdu);
        has_cargo = 1;
        }
    if(fleet[shp].spacemines!=0) {
        fprintf(output, "SpcMines %d;", fleet[shp].spacemines);
        has_cargo = 1;
        }
    for(count=0;count<10;count++)
        if(fleet[shp].ore[count]!=0) {
            fprintf(output, "R%d %d;", count, fleet[shp].ore[count]);
            has_cargo = 1;
            }
    if (has_cargo == 0) {
        fprintf(output, "None");
    };
    fprintf(output, "}\\\\\n");
    fprintf(output, "\\end{tabular}\n");
    fprintf(output, "}\n");
    return;
}

/*****************************************************************************/
int ChekPlan(Planet plan, Player plr, FILE *output)
/*****************************************************************************/
/* Get details of the planet for the planet summary */
{
    TRTUR2(printf("ChekPlan(plan:%d, plr:%d)\n", plan, plr));

    if(galaxy[plan].owner!=plr && plr!=NEUTPLR)
        return(0);

    fprintf(output, "%d &", plan+100);
    for(int rtype=0;rtype<10;rtype++) {
        if(galaxy[plan].mine[rtype]==0 && galaxy[plan].ore[rtype]==0) {
            fprintf(output, " & &");
            }
        else if(galaxy[plan].mine[rtype]==0) {
            fprintf(output, " &%d &", galaxy[plan].ore[rtype]);
            }
        else if(galaxy[plan].ore[rtype]==0) {
            fprintf(output, "%d& &", galaxy[plan].mine[rtype]);
            }
        else {
            fprintf(output, "%d&%d &", galaxy[plan].mine[rtype], galaxy[plan].ore[rtype]);
            }
        }
    if(galaxy[plan].pdu==0)
        fprintf(output, " --- &");
    else {
        fprintf(output, " %d &", galaxy[plan].pdu);
        }
    if(galaxy[plan].ind==0)
        fprintf(output, " --- \\\\\n");
    else {
        fprintf(output, " %d\\\\\n", galaxy[plan].ind);
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

    fprintf(output, "Total: &");
    for(count=0;count<10;count++) {
        fprintf(output, "%d & %d &", tmin[count], tore[count]);
        }
    fprintf(output, "%d & %d", tpdu, tind);
    fprintf(output, "\\\\ \\hline \n");
}

/*****************************************************************************/
void ChekShip(Ship shp, Player plr, FILE *output)
/*****************************************************************************/
/* Get details of ship for ship summary */
{
    int count;

    if(fleet[shp].owner!=plr && plr!=NEUTPLR) 
        return;

    TRTUR2(printf("ChekShip(ship:%d, plr:%d)\n", shp, plr));
    /* Print ship details */
    fprintf(output, "S%d & fighter=%d & cargo=%d(%d) & tractor=%d & shield=%d(%d)", shp+100, fleet[shp].fight, fleet[shp].cargo, fleet[shp].cargleft, fleet[shp].tractor, fleet[shp].shield, Shields(shp));
    fprintf(output, " & eff=%-1d(%d) & shots=%d", fleet[shp].efficiency, EffEff(shp), Shots(shp, fleet[shp].fight));
    fprintf(output, "%s\\\\\n", fleet[shp].name);
    fprintf(output, "& Planet: %d & \\multicolumn{2}{l}{%s} & Standing: & \\multicolumn{2}{l}{", fleet[shp].planet+100, stypes[fleet[shp].type]);
    if(fleet[shp].stndord[0]==0)
        fprintf(output, "None} \\\\\n");
    else
        fprintf(output, "S%d%s} \\\\\n", shp+100, fleet[shp].stndord);
    /* Print out cargo details */
    fprintf(output, "& \\multicolumn{6}{l}{");
    if(fleet[shp].ind!=0) {
        fprintf(output, "Ind=%d; ", fleet[shp].ind);
        }
    if(fleet[shp].mines!=0) {
        fprintf(output, "Mines=%d; ", fleet[shp].mines);
        }
    if(fleet[shp].pdu!=0) {
        fprintf(output, "PDU=%d; ", fleet[shp].pdu);
        }
    if(fleet[shp].spacemines!=0) {
        fprintf(output, "SM=%d; ", fleet[shp].spacemines);
        }
    for(count=0;count<10;count++)
        if(fleet[shp].ore[count]!=0) {
            fprintf(output, "Ore %d=%d;", count, fleet[shp].ore[count]);
            }
    fprintf(output, "}\\\\\n");
    fprintf(output, "\\\\\n");
    return;
}

/*****************************************************************************/
void ResOutput(Player plr, FILE *output)
/*****************************************************************************/
{
    int nres, count;

    TRTUR(printf("Process:Research turn\n"));
    nres=NumRes(plr);
    fprintf(output, "\nRemember to give research orders this turn\n");
    fprintf(output, "You have %d research planet%c\n", nres, nres!=1?'s':' ');
    if(nres==0)
        return;
    if(nres==1) {
        fprintf(output, "It is ");
        }
    else {
        fprintf(output, "They are ");
        }
    for(count=0;count<NUMPLANETS;count++) 
        if(IsResearch(count) && galaxy[count].owner==plr) {
            fprintf(output, " %d", count+100);
        }
        fprintf(output, ".\n");
    return;
}

/*****************************************************************************/
void TypeSummary(FILE *output)
/*****************************************************************************/
{
    int x, types[NUMTYPES];

    TRTUR(printf("TypeSummary\n"));
    for(x=0;x<NUMTYPES;x++)		/* Clear the types */
        types[x]=0;

    for(x=0;x<shiptr;x++)
        types[fleet[x].type]++;	/* Work out how many of each type */
        
    fprintf(output, "\n");
    fprintf(output, "\\section*{Summary of ship types}\n");
    fprintf(output, "\\begin{tabular}{lr|lr}\n");
    for(x=0;x<NUMTYPES-(NUMTYPES%2);x += 2) {
        fprintf(output, "%s & %d & %s & %d\\\\\n", stypes[x], types[x], stypes[x+1], types[x+1]);
        }
    if(NUMTYPES%2!=0) {
        fprintf(output, "%s & %d& &\n", stypes[NUMTYPES-1], types[NUMTYPES-1]);
        }
    fprintf(output, "\\end{tabular}\n\n");
}

/*****************************************************************************/
void Headings(FILE *output, FILE *dotfile, Player plr)
/*****************************************************************************/
{
    int income=CalcPlrInc(plr);

    TRTUR(printf("Headings\n"));
    TitlePage(output);
    /* Print out name of player and number */
    fprintf(output, "\n");
    fprintf(output, "\\section*{%s}\n", name[plr]);
    /* Print out score and gm and turn numbers */
    fprintf(output, "\\subsection*{Turn %d}\n", turn);
    fprintf(output, "\\begin{itemize}\n");
    fprintf(output, "\\item score=%d\n", score[plr]);
    /* Print out due date, and player scores */
    fprintf(output, "\\item date due= before %-8s\n" , duedate);
    fprintf(output, "\\item your income=%4d\n", income);
    fprintf(output, "\\item Earth credits=%4d \n", ecredit[plr]);
    fprintf(output, "\\item Credits:Score=%3d \n", gamedet.earth.earthmult);
    fprintf(output, "\\item You have %d scans this turn\n", NumRes(plr)+1);
    fprintf(output, "\\item \\begin{tabular}{c|c|c}\n");
    fprintf(output, "\\multicolumn{3}{c}{Player Scores}\\\\ \\hline \n");
    fprintf(output, "%6d & %6d & %6d\\\\ \n" , score[1], score[2], score[3]);
    fprintf(output, "%6d & %6d & %6d\\\\ \n", score[4], score[5], score[6]);
    fprintf(output, "%6d & %6d & %6d\\\\ \n", score[7], score[8], score[9]);
    fprintf(output, "\\end{tabular}\n\n");
    fprintf(output, "\\end{itemize}\n\n");

    /* Print winning conditions */
    fprintf(output, "\\subsection*{Winning Conditions}\n");
    fprintf(output, "\\begin{itemize}\n");
    if(gamedet.winning & WEARTH) {
        fprintf(output, "\\item Earth\n");
        }
    if(gamedet.winning & WCREDIT) {
        fprintf(output, "\\item Credits=%d\n", gamedet.credits);
        }
    if(gamedet.winning & WINCOME) {
        fprintf(output, "\\item Income=%d\n", gamedet.income);
        }
    if(gamedet.winning & WSCORE) {
        fprintf(output, "\\item Score=%d\n", gamedet.score);
        }
    if(gamedet.winning & WPLANETS) {
        fprintf(output, "\\item Planets=%d\n", gamedet.planets);
        }
    if(gamedet.winning & WTURN) {
        fprintf(output, "\\item Turn=%d\n", gamedet.turn);
        if(!(gamedet.winning & WFIXTURN)) {
            fprintf(output, "\\item Desired end turn=%d\n", desturn[plr]);
            }
        }
    fprintf(output, "\\end{itemize}\n");

    fprintf(output, "Minimum bids:\n");
    fprintf(output, "\\begin{itemize}\n");
    fprintf(output, "\\item Cargo=%d\n", gamedet.earth.cbid);
    fprintf(output, "\\item Fighter=%d\n", gamedet.earth.fbid);
    fprintf(output, "\\item Shield=%d\n", gamedet.earth.sbid);
    fprintf(output, "\\item Tractor=%d\n", gamedet.earth.tbid);
    fprintf(output, "\\end{itemize}\n");
    
    fprintf(dotfile, "strict graph G {\n");
}

/*****************************************************************************/
void PlanetMap(Player plr, FILE *output)
/*****************************************************************************/
{
    fprintf(output, "\\begin{adjustwidth}{-3cm}{3cm}\n");
    fprintf(output, "\\centerline{\\includegraphics[width=15cm]{turn%d.%d.eps}}\n", turn, plr);
    fprintf(output, "\\end{adjustwidth}\n");
}

/*****************************************************************************/
void PlanetSummary(Player plr, FILE *output)
/*****************************************************************************/
{
    int total=0;
    int rtype;

    fprintf(output, "\\section*{Summary of planets}\n");
    fprintf(output, "\\begin{longtable}{rc@{/}cc@{/}cc@{/}cc@{/}cc@{/}cc@{/}cc@{/}cc@{/}cc@{/}cc@{/}ccc}\n");
    fprintf(output, "planet &");
    for(rtype=0; rtype<10; rtype++) {
        fprintf(output, " \\multicolumn{2}{c}{%d} &", rtype);
        }
    fprintf(output, "PDU & IND \\\\ \\hline\n");
    fprintf(output, "\\endhead\n");
    for(int count=0; count<NUMPLANETS; count++) {
        total += ChekPlan(count, plr, output);
    }
    fprintf(output, "\\hline\n");
    ChekShipTot(plr, output);
    ChekTot(plr, output);
    fprintf(output, "\\end{longtable}\n");

    fprintf(output, "Total number of planets owned = %d\n", total);
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
			indown[count] += galaxy[count2].ind;
			pduown[count] += galaxy[count2].pdu;
			}
	planown[10] += planown[count];
	indown[10] += indown[count];
	pduown[10] += pduown[count];
	resplan[10] += resplan[count];
	totinc += CalcPlrInc(count);
	totscore += score[count];
	}

/* Print planet numbers */
fprintf(output, "\n");
fprintf(output, "\\section*{Ownership statistix}\n");
fprintf(output, "\\begin{tabular}{rllllll}\n");
fprintf(output, "Empire & Plnets & ResPlan & Indust & Income & Score & PDUs\\\\ \\hline \n");
for(count=0;count<NUMPLAYERS+1;count++) {
	fprintf(output, "%s & %d & %d & %d", name[count], planown[count], resplan[count], indown[count]);
	fprintf(output, "& %d & %d & %d \\\\ \n", CalcPlrInc(count), score[count], pduown[count]);
	}
fprintf(output, "\\hline\n");
fprintf(output, "%s & %d & %d & %d", "Total", planown[10], resplan[10], indown[10]);
fprintf(output, "& %d & %d & %d\\\\\n", totinc, totscore, pduown[10]);
fprintf(output, "\\end{tabular}\n");
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
                fgtown[count] += fleet[count2].fight;
                crgown[count] += fleet[count2].cargo;
                shdown[count] += fleet[count2].shield;
                tracown[count] += fleet[count2].tractor;
                }
            }
        shpown[10] += shpown[count];
        fgtown[10] += fgtown[count];
        crgown[10] += crgown[count];
        shdown[10] += shdown[count];
        tracown[10] += tracown[count];
        }

    /* Print unit numbers */
    fprintf(output, "\n");
    fprintf(output, "\\section*{Ship statistix}\n");
    fprintf(output, "\\begin{tabular}{rlllll}\\\n");
    fprintf(output, "Empire & Ships & Fghtrs & Cargo & Shield & Tractor\\\\ \\hline\n");
    for(count=0;count<NUMPLAYERS+1;count++) {
        fprintf(output, "%s & %d & %d & %d", name[count], shpown[count], fgtown[count], crgown[count]);
        fprintf(output, " & %d & %d \\\\ \n", shdown[count], tracown[count]);
        }
    fprintf(output, "\\hline\n");
    fprintf(output, "%s & %d & %d & %d", "Total", shpown[10], fgtown[10], crgown[10]);
    fprintf(output, " & %d & %d\n", shdown[10], tracown[10]);
    fprintf(output, "\\end{tabular}\n");
    fprintf(output, "\n");
}

/*****************************************************************************/
void WinningDetails(FILE *output)
/*****************************************************************************/
/* Print the winning details, and all info from the game structure */
{
    int cnt;

    TRTUR(printf("WinningDetails()\n"));
    fprintf(output, "\n\\section*{Game details}\n");

    /* Print winning conditions */
    fprintf(output, "\\subsection*{Winning Conditions}\n");
    if(gamedet.winning & WEARTH) {
        fprintf(output, "Earth  ");
        }
    if(gamedet.winning & WSCORE) {
        fprintf(output, "Score=%d  ", gamedet.score);
        }
    if(gamedet.winning & WINCOME) {
        fprintf(output, "Income=%d  ", gamedet.income);
        }
    if(gamedet.winning & WPLANETS) {
        fprintf(output, "Planets=%d", gamedet.planets);
        }
    fprintf(output, "\n");

    /* Print details of initial home planet */
    fprintf(output, "\\subsection*{Home Details}\n");
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
    fprintf(output, "\n");

    /* Print details of initial Earth planet */
    fprintf(output, "\\subsection*{Earth Details}\n");
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
    fprintf(output, "\\subsection*{Ore prices}\n");
    fprintf(output, "\\begin{tabular}{r|cccccccccc}\n");
    fprintf(output, "Ore Type: ");
    for(cnt=0;cnt<10;cnt++) {
        fprintf(output, " & %3d", cnt);
        }
    fprintf(output, "\\\\ \n");
    fprintf(output, "Cost: ");
    for(cnt=0;cnt<10;cnt++) {
        fprintf(output, " & %d", price[cnt]);
        }
    fprintf(output, "\\\\ \n");
    fprintf(output, "\\end{tabular}\n\n");
    return;
}

/*****************************************************************************/
void ChekShipTot(Player plr, FILE *output)
/*****************************************************************************/
/* Print the ore totals that are stored on ships */
{
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
                oretot[tmp] += fleet[count].ore[tmp];
            }
        oretot[10] += fleet[count].ind;
        oretot[11] += fleet[count].pdu;
        }

    /* Print out results */
    fprintf(output, "Ships: &");
    for(tmp=0;tmp<10;tmp++) {
        if(oretot[tmp]>0) {
            fprintf(output, " & %d &", oretot[tmp]);
            }
        else {
            fprintf(output, " & &");
            }
        }
    fprintf(output, " %d &", oretot[11]);
    fprintf(output, " %d\\\\\n", oretot[10]);
    return;
}

/* EOF */
