/*
 * Program to edit the CELESTIAL EMPIRE data structures
 *
 * By Zer Dwagon (c) 2016 Dougal Scott
 *
 */

#include "def.h"
#include "edit.h"
#include "typname.h"
#include <string.h>

#define TREDIT(x)	if(strstr(dbgstr,"EDIT") || strstr(dbgstr,"edit")) x

/* Maximum/minimum that any units can be increase to */
#define MAXUNIT	250
#define MINUNIT	5

/* Research planet increase amounts */
#define TWINC(x)	x+=MAX(MINUNIT,(x*20)/100)
#define FFINC(x)	x+=MAX(MINUNIT,(x*50)/100)

char *dbgstr;
planet	galaxy[NUMPLANETS];
ship	fleet[NUMSHIPS];
Number	price[10];
Number 	ecredit[NUMPLAYERS+1];
game 	gamedet;
Flag	alliance[NUMPLAYERS+1][NUMPLAYERS+1];
Ship	shiptr;
int		turn,
		gm,
		score[NUMPLAYERS+1];
char	name[NUMPLAYERS+1][10];
char	prompt[20];
char 	*editor;
FILE	*trns[NUMPLAYERS+1];
static char	allsym[4]={'E','N','F','A'};
int 	desturn[NUMPLAYERS+1];

/******************************************************************************/
int main(int argc,char **argv)
/******************************************************************************/
{
    char *gmstr;
    char systr[BUFSIZ], tmpstr[BUFSIZ], srcfname[BUFSIZ];

    if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
        dbgstr=(char *)"null";

    if(argc==2) 
        gm=atoi(argv[1]);
    else {
        if((gmstr = getenv("CELEMPGAME")) == NULL) {
            fprintf(stderr,"set CELEMPGAME to the appropriate game number\n");
            exit(-1);
            }
        gm=atoi(gmstr);
        }

    if((editor = getenv("EDITOR")) == NULL) {
        strcpy(editor,"vi");
        }

    fprintf(stderr,"Reading in galaxy structure\n");
    if(ReadGalflt()==-1) {
        fprintf(stderr,"Program terminated\n");
        exit(-1);
        }

    sprintf(tmpstr,"/tmp/gfile%d.orgXXXXXX",gm);
    mktemp(tmpstr);
    FilePath("galfile.json", srcfname);
    sprintf(systr,"cp %s %s", srcfname, tmpstr);
    (void)system(systr);

    for(;;)
        MainMenu();

    return(0);
}

/*****************************************************************************/
void MainMenu(void)
/*****************************************************************************/
{
char buff[80];

printf("W..Write Datafile\t");
printf("R..Read Datafile\t");
printf("Z..New Ship\n");
printf("P..Planet Editing\t");
printf("S..Ship Editing\t");
printf("A..Alliance Editing\n");
printf("N..Name Editing\t");
printf("X..Player Editing\t");
printf("V..Verify Validity\n");
printf("C..Cost Editing\t");
printf("G..Game details\t\t");
printf("U..Unit Increase(RP)\n");
printf("Q..Quit Editing\n");

printf("Main Menu> ");
fgets(buff, sizeof(buff), stdin);
switch(buff[0]) {
	case 'c': case 'C':
				PriceEdit();
				break;
	case 'g': case 'G':
				GameEdit();
				break;
	case 'w': case 'W':
				printf("Saving....\n");
				WriteGalflt();
				break;
	case 'r': case 'R':
				ReadGalflt();
				break;
	case 'p': case 'P':
				PlanetEdit();
				break;
	case 's': case 'S':
				ShipEdit(-1);
				break;
	case 'a': case 'A':
				AllianceEdit();
				break;
	case 'n': case 'N':
				NameEdit();
				break;
	case 'x': case 'X':
				ScoreEdit();
				break;
	case 'u': case 'U':
				UnitIncrease();
				break;
	case 'v': case 'V':
				Validate();
				break;
	case 'q': 
				Validate();
				fflush(stdin);
				printf("Do you wish to save galaxy?");
				fflush(stdin);
				fgets(buff, sizeof(buff), stdin);
				if(buff[0]!='n' && buff[0]!='N') {
					printf("Saving....\n");
					WriteGalflt();
					}
	case 'Q':
				exit(0);
				break;
	case 'z': case 'Z':
				shiptr++;
				break;
	default:
				break;
	}
}

/*****************************************************************************/
void PlanetEdit(void)
/*****************************************************************************/
{
    Planet num;				/* Planet Number */
    char tmpstr[80];		/* Temporary file name */
    char systr[80];			/* System tmp string */
    FILE *ef;				/* Edit file */

    printf("Enter planet to edit (100-%d): ", NUMPLANETS+100);
    scanf("%hd", &num);
    if(num<100 || num>NUMPLANETS+100)
        return;
    num-=100;

    strcpy(tmpstr, "/tmp/ZDpeXXXXXX");
    mktemp(tmpstr);
    if((ef=fopen(tmpstr, "w"))==NULL) {
        fprintf(stderr, "Could not open %s for writing\n", tmpstr);
        return;
        }
    PrintPlanet(num, ef);
    fclose(ef);

    sprintf(systr, "%s %s", editor, tmpstr);
    (void)system(systr);

    AnalyzePlanet(tmpstr, num);
    remove(tmpstr);
}

/*****************************************************************************/
void ShipEdit(Ship num)
/*****************************************************************************/
{
char tmpstr[80];		/* Temporary file name */
char systr[80];			/* System tmp string */
FILE *ef;				/* Edit file */

if(num<0) {
	printf("Enter ship to edit (100-%d): ",shiptr+99);
	scanf("%hd",&num);
	num-=100;
	if(num<0 || num>shiptr-1)
		return;
	}

strcpy(tmpstr,"/tmp/ZDseXXXXXX");
mktemp(tmpstr);
if((ef=fopen(tmpstr,"w"))==NULL) {
	fprintf(stderr,"Could not open %s for writing\n",tmpstr);
	return;
	}
PrintShip(num,ef);
fclose(ef);

sprintf(systr,"%s %s",editor,tmpstr);
(void)system(systr);

AnalyzeShip(tmpstr,num);
remove(tmpstr);
}
 
/*****************************************************************************/
void AllianceEdit(void)
/*****************************************************************************/
{
char tmpstr[80];		/* Temporary file name */
char systr[80];			/* System tmp string */
FILE *ef;				/* Edit file */

strcpy(tmpstr,"/tmp/ZDaeXXXXXX");
mktemp(tmpstr);
if((ef=fopen(tmpstr,"w"))==NULL) {
	fprintf(stderr,"Could not open %s for writing\n",tmpstr);
	return;
	}
PrintAlliance(ef);
fclose(ef);

sprintf(systr,"%s %s",editor,tmpstr);
(void)system(systr);

AnalyzeAlliance(tmpstr);
remove(tmpstr);
}

/*****************************************************************************/
void NameEdit(void)
/*****************************************************************************/
{
char tmpstr[80];		/* Temporary file name */
char systr[80];			/* System tmp string */
FILE *ef;				/* Edit file */

strcpy(tmpstr,"/tmp/ZDneXXXXXX");
mktemp(tmpstr);
if((ef=fopen(tmpstr,"w"))==NULL) {
	fprintf(stderr,"Could not open %s for writing\n",tmpstr);
	return;
	}
PrintName(ef);
fclose(ef);

sprintf(systr,"%s %s",editor,tmpstr);
(void)system(systr);

AnalyzeName(tmpstr);
remove(tmpstr);
}

/*****************************************************************************/
void ScoreEdit(void)
/*****************************************************************************/
{
char tmpstr[80];		/* Temporary file name */
char systr[80];			/* System tmp string */
FILE *ef;				/* Edit file */

strcpy(tmpstr,"/tmp/ZDxeXXXXXX");
mktemp(tmpstr);
if((ef=fopen(tmpstr,"w"))==NULL) {
	fprintf(stderr,"Could not open %s for writing\n",tmpstr);
	return;
	}
PrintScore(ef);
fclose(ef);

sprintf(systr,"%s %s",editor,tmpstr);
(void)system(systr);

AnalyzeScore(tmpstr);
remove(tmpstr);
}

/*****************************************************************************/
void PrintPlanet(Planet num,FILE *ef)
/*****************************************************************************/
{
    int x;

    fprintf(ef,"Planet %d details:\n",num+100);
    fprintf(ef,"Name:%-s\n",galaxy[num].name);
    fprintf(ef,"Owner:%-2d Income:%-5d Spec:%-5d\n",galaxy[num].owner,galaxy[num].income,galaxy[num].spec);
    fprintf(ef,"Industry:%-5d PDU:%-5d Spacemines:%-5d Deployed:%-5d\n",galaxy[num].ind,galaxy[num].pdu,galaxy[num].spacemine,galaxy[num].deployed);
    fprintf(ef,"Standing:%-10s\n",galaxy[num].stndord);
    fprintf(ef,"Links: %-4d %-4d %-4d %-4d\n",galaxy[num].link[0]+100,galaxy[num].link[1]+100,galaxy[num].link[2]+100,galaxy[num].link[3]+100);
    fprintf(ef,"Mine Type     ");
    for(x=0;x<10;x++) 
        fprintf(ef,"%-5d",x);
    fprintf(ef,"\nAmount Stored ");
    for(x=0;x<10;x++) 
        fprintf(ef,"%-5d",galaxy[num].ore[x]);
    fprintf(ef,"\nProduction    ");
    for(x=0;x<10;x++) 
        fprintf(ef,"%-5d",galaxy[num].mine[x]);
    fprintf(ef,"\n");
}

/*****************************************************************************/
void PrintShip(Ship num,FILE *ef)
/*****************************************************************************/
{
int x;

fprintf(ef,"Ship %d details:\n",num+100);
fprintf(ef,"Name:%s\nOwner:%-2d Type:%-2d %s\n",fleet[num].name,fleet[num].owner,fleet[num].type,stypes[fleet[num].type]);
fprintf(ef,"Fighter:%-4d Cargo:%-4d Shield:%-4d Tractor:%d\n",fleet[num].fight,fleet[num].cargo,fleet[num].shield,fleet[num].tractor);
fprintf(ef,"Planet:%-4d Cargoleft:%-4d Efficiency:%d\n",fleet[num].planet+100,fleet[num].cargleft,fleet[num].efficiency);
fprintf(ef,"Standing:%s\n",fleet[num].stndord);
fprintf(ef,"Mines:%-4d Industries:%-4d PDUs:%-4d Spacemines:%d\n",fleet[num].mines,fleet[num].ind,fleet[num].pdu,fleet[num].spacemines);
for(x=0;x<10;x++)
	fprintf(ef,"R%d:%-3d ",x,fleet[num].ore[x]);
fprintf(ef,"\n");
}

/*****************************************************************************/
void PrintAlliance(FILE *ef)
/*****************************************************************************/
{
int x,y;

fprintf(ef,"Alliance Details\n");
fprintf(ef,"ALLY:%c\tFRIEND:%c\tNEUTRAL:%c\tENEMY:%c\n",allsym[ALLY],allsym[FRIEND],allsym[NEUTRAL],allsym[ENEMY]);
fprintf(ef,"           ");
for(x=0;x<=NUMPLAYERS;x++)
	fprintf(ef,"%-3d ",x);
fprintf(ef,"\n");
for(x=0;x<=NUMPLAYERS;x++) {
	fprintf(ef,"%-9s  ",name[x]);
	for(y=0;y<=NUMPLAYERS;y++) {
		fprintf(ef,"%c   ",allsym[alliance[x][y]]);
		}
	fprintf(ef,"\n");
	}
}

/*****************************************************************************/
void PrintName(FILE *ef)
/*****************************************************************************/
{
int x;

fprintf(ef,"Name Details\n");
for(x=0;x<NUMPLAYERS+1;x++)
	fprintf(ef,"%-3d %s\n",x,name[x]);
}

/*****************************************************************************/
void PrintScore(FILE *ef)
/*****************************************************************************/
{
int x;

fprintf(ef,"Score Details\n");
fprintf(ef,"#   Score   ECredit DesTurn\n");
for(x=1;x<NUMPLAYERS+1;x++)
	fprintf(ef,"%-3d %-7d %-7d %-7d\n",x,score[x],ecredit[x],desturn[x]);
}

/*****************************************************************************/
void AnalyzePlanet(char filename[80],Planet num)
/*****************************************************************************/
{
FILE *ef;
char buff[80];
int confirm=0,x;
char *tok;
planet tmp;

/* File format
Planet 123 details:
Name:Appropinquare
Owner:5 Six        Income:30    Spec:0    
Industry:0     PDU:0     Spacemines:0     Deployed:0    
Links: 29  117  10  -1   Numlinks:3
Mine Type     0    1    2    3    4    5    6    7    8    9    
Amount Stored 7    0    0    0    2    9    4    0    0    0    
Production    2    0    0    0    2    4    2    0    0    0    
*/

bzero(&tmp,sizeof(planet));
if((ef=fopen(filename,"r"))==NULL) {
	fprintf(stderr,"Could not open %s for reading\n",filename);
	return;
	}
fgets(buff, sizeof(buff), ef);		/* Absorb first line */
fgets(buff, sizeof(buff), ef);		/* Get name line */
for(x=0;x<80;x++)		/* Strip CR */
	if(buff[x]=='\n') {
		buff[x]='\0';
		break;
		}
tok=strtok(buff,": \t");
strcpy(tmp.name,"\0");
while(tok!=NULL) {
	tok=strtok(NULL,": \t");
	if(tok==NULL)
		break;
	strncat(tmp.name," ",NAMESIZ);
	strncat(tmp.name,tok,NAMESIZ);
	}

fgets(buff,sizeof(buff),ef);		/* Get owner income spec line */
tok=strtok(buff,": \t");	/* Owner */
tmp.owner=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Income */
tok=strtok(NULL,": \t");	/* Inc # */
tok=strtok(NULL,": \t");	/* Spec */
tmp.spec=atoi(strtok(NULL,": \t"));

fgets(buff,sizeof(buff),ef);		/* Get the ind pdu space depl line */
tok=strtok(buff,": \t");	/* Industry */
tmp.ind=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* PDU */
tmp.pdu=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Spacemines */
tmp.spacemine=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Deployed */
tmp.deployed=atoi(strtok(NULL,": \t"));

fgets(buff,sizeof(buff),ef);		/* Get the Standing Orders line */
tok=strtok(buff,": \t");	/* Standing */
strcpy(tmp.stndord,strtok(NULL,": \t"));
if(tmp.stndord[0]=='\n')
	tmp.stndord[0]=0;

fgets(buff,sizeof(buff),ef);		/* Get the links line */
tok=strtok(buff,": \t");	/* Links */
tmp.link[0]=atoi(strtok(NULL,": \t"))-100;
tmp.link[1]=atoi(strtok(NULL,": \t"))-100;
tmp.link[2]=atoi(strtok(NULL,": \t"))-100;
tmp.link[3]=atoi(strtok(NULL,": \t"))-100;

fgets(buff,sizeof(buff),ef);		/* Ignore the mine type line */

fgets(buff,sizeof(buff),ef);		/* Amount stored */
tok=strtok(buff,": \t");	/* Amount */
tok=strtok(NULL,": \t");	/* Stored */
tmp.ore[0]=atoi(strtok(NULL,": \t")); tmp.ore[1]=atoi(strtok(NULL,": \t"));
tmp.ore[2]=atoi(strtok(NULL,": \t")); tmp.ore[3]=atoi(strtok(NULL,": \t"));
tmp.ore[4]=atoi(strtok(NULL,": \t")); tmp.ore[5]=atoi(strtok(NULL,": \t"));
tmp.ore[6]=atoi(strtok(NULL,": \t")); tmp.ore[7]=atoi(strtok(NULL,": \t"));
tmp.ore[8]=atoi(strtok(NULL,": \t")); tmp.ore[9]=atoi(strtok(NULL,": \t"));

fgets(buff,sizeof(buff),ef);		/* Production */
tok=strtok(buff,": \t");	/* Production */
tmp.mine[0]=atoi(strtok(NULL,": \t")); tmp.mine[1]=atoi(strtok(NULL,": \t"));
tmp.mine[2]=atoi(strtok(NULL,": \t")); tmp.mine[3]=atoi(strtok(NULL,": \t"));
tmp.mine[4]=atoi(strtok(NULL,": \t")); tmp.mine[5]=atoi(strtok(NULL,": \t"));
tmp.mine[6]=atoi(strtok(NULL,": \t")); tmp.mine[7]=atoi(strtok(NULL,": \t"));
tmp.mine[8]=atoi(strtok(NULL,": \t")); tmp.mine[9]=atoi(strtok(NULL,": \t"));

fclose(ef);

if(tmp.owner<0 || tmp.owner>9) {
	printf("Owner %d out of range(0-%d), restoring to original\n",tmp.owner,NUMPLAYERS);
	tmp.owner=galaxy[num].owner;
	confirm=1;
	}
if(tmp.ind<0) {
	printf("Industry %d: too low. Restoring to original\n",tmp.ind);
	tmp.ind=galaxy[num].ind;
	confirm=1;
	}
if(tmp.pdu<0) {
	printf("PDU %d: too low. Restoring to original\n",tmp.pdu);
	tmp.pdu=galaxy[num].pdu;
	confirm=1;
	}
if(tmp.spacemine<0) {
	printf("Spacemines %d: too low. Restoring to original\n",tmp.spacemine);
	tmp.spacemine=galaxy[num].spacemine;
	confirm=1;
	}
if(tmp.deployed<0) {
	printf("Deployed spacemines %d: too low. Restoring to orig\n",tmp.deployed);
	tmp.deployed=galaxy[num].deployed;
	confirm=1;
	}
for(x=0;x<4;x++)
	if(tmp.link[x]<-1 || tmp.link[x]>NUMPLANETS) {
		printf("Link %d %d. Out of Range (%d-%d). Restoring to orig\n",x,tmp.link[x],-1,NUMPLANETS);
		tmp.link[x]=galaxy[num].link[x];
		confirm=1;
		}
for(x=0;x<10;x++) {
	if(tmp.ore[x]<0) {
		printf("Ore %d %d out of range. Restoring to original\n",x,tmp.ore[x]);
		tmp.ore[x]=galaxy[num].ore[x];
		}
	if(tmp.mine[x]<0) {
		printf("Mine %d %d out of range. Restoring to orig\n",x,tmp.mine[x]);
		tmp.mine[x]=galaxy[num].mine[x];
		}
	}
tmp.income=20+5*tmp.ind;
for(x=0;x<10;x++)
	tmp.income+=tmp.mine[x];
printf("Planet %d details:\n",num+100);
printf("Name:%-*s\nOwner:%-2d %-9s Income:%-5d Spec:%-5d\n",NAMESIZ,tmp.name,tmp.owner,name[tmp.owner],tmp.income,tmp.spec);
printf("Industry:%-5d PDU:%-5d Spacemines:%-5d Deployed:%-5d\n",tmp.ind,tmp.pdu,tmp.spacemine,tmp.deployed);
printf("Standing:%-10s\n",tmp.stndord);
printf("Links: %-4d %-4d %-4d %-4d\n",tmp.link[0]+100,tmp.link[1]+100,tmp.link[2]+100,tmp.link[3]+100);
printf("Mine Type     ");
for(x=0;x<10;x++) 
	printf("%-5d",x);
printf("\nAmount Stored ");
for(x=0;x<10;x++) 
	printf("%-5d",tmp.ore[x]);
printf("\nProduction    ");
for(x=0;x<10;x++) 
	printf("%-5d",tmp.mine[x]);
printf("\n");

printf("Confirm the changes to planet?");
tmp.scanned=0;
fflush(stdin);
buff[0]=0;
while(buff[0]<'A')
	fgets(buff,sizeof(buff),stdin);
if(buff[0]=='n' || buff[0]=='N')
    printf("Planet change discarded\n");
else
	bcopy(&tmp,&galaxy[num],sizeof(tmp));
}

/*****************************************************************************/
void AnalyzeShip(char filename[80],Ship num)
/*****************************************************************************/
{
FILE *ef;
ship tmp;
char buff[80],*tok;
int x,confirm=0;

/* File format
Ship 113 details:
Name:Vaccuus
Owner:2  Three     Type:2  Medium Cargo   
Fighter:1    Cargo:10   Shield:0    Tractor:0   
Planet:15    Cargoleft:0    Efficiency:0   
Mines:0    Industries:0    PDUs:0    Spacemines:0   
R0: 0   R1: 0   R2: 0   R3: 0   R4: 0   R5: 0   R6: 0   R7: 0   R8: 0   R9: 0   
*/

bzero(&tmp,sizeof(ship));
if((ef=fopen(filename,"r"))==NULL) {
	fprintf(stderr,"Could not open %s for reading\n",filename);
	return;
	}

fgets(buff,sizeof(buff),ef);		/* Absorb first line */
fgets(buff,sizeof(buff),ef);		/* Get name line */
for(x=0;x<80;x++)		/* Strip CR */
	if(buff[x]=='\n') {
		buff[x]='\0';
		break;
		}
tok=strtok(buff,": \t");
strcpy(tmp.name,"\0");
while(tok!=NULL) {
	tok=strtok(NULL,": \t");
	if(tok==NULL)
		break;
	strncat(tmp.name," ",NAMESIZ);
	strncat(tmp.name,tok,NAMESIZ);
	}

fgets(buff,sizeof(buff),ef);		/* Get owner type */
tok=strtok(buff,": \t");	/* Owner */
tmp.owner=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Type */
tok=strtok(NULL,": \t");	/* Type # */

fgets(buff,sizeof(buff),ef);		/* Get the fighet cargo shield tractor line */
tok=strtok(buff,": \t");	/* Fighter */
tmp.fight=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Cargo */
tmp.cargo=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Shield */
tmp.shield=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Tractor */
tmp.tractor=atoi(strtok(NULL,": \t"));

fgets(buff,sizeof(buff),ef);		/* Get the planet carleft effic line */
tok=strtok(buff,": \t");	/* Planet */
tmp.planet=atoi(strtok(NULL,": \t"))-100;
tok=strtok(NULL,": \t");	/* Cargleft */
tok=strtok(NULL,": \t");	/* Cargleft # */
tok=strtok(NULL,": \t");	/* Efficiency */
tmp.efficiency=atoi(strtok(NULL,": \t"));

fgets(buff,sizeof(buff),ef);		/* Get the standing line */
tok=strtok(buff,": \t");	/* Standing */
strcpy(tmp.stndord,strtok(NULL,": \t"));
if(tmp.stndord[0]=='\n')
	tmp.stndord[0]=0;
#ifdef ABSORB
	fgets(buff,sizeof(buff),ef);		/* Absorb line */
#endif
fgets(buff,sizeof(buff),ef);		/* Get the Mines ind pdu spacemine line */
tok=strtok(buff,": \t");	/* Mines */
tmp.mines=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Industries */
tmp.ind=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* PDUs */
tmp.pdu=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* Spacemines */
tmp.spacemines=atoi(strtok(NULL,": \t"));

fgets(buff,sizeof(buff),ef);		/* Get the ore line */
tok=strtok(buff,": \t");	/* R0 */
tmp.ore[0]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R1 */
tmp.ore[1]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R2 */
tmp.ore[2]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R3 */
tmp.ore[3]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R4 */
tmp.ore[4]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R5 */
tmp.ore[5]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R6 */
tmp.ore[6]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R7 */
tmp.ore[7]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R8 */
tmp.ore[8]=atoi(strtok(NULL,": \t"));
tok=strtok(NULL,": \t");	/* R9 */
tmp.ore[9]=atoi(strtok(NULL,": \t"));
fclose(ef);

if(tmp.owner<0 || tmp.owner>9) {
	printf("Owner %d out of range(0-%d), restoring to original\n",tmp.owner,NUMPLAYERS);
	tmp.owner=fleet[num].owner;
	confirm=1;
	}
if(tmp.ind<0) {
	printf("Industry %d: too low. Restoring to original\n",tmp.ind);
	tmp.ind=fleet[num].ind;
	confirm=1;
	}
if(tmp.pdu<0) {
	printf("PDU %d: too low. Restoring to original\n",tmp.pdu);
	tmp.pdu=fleet[num].pdu;
	confirm=1;
	}
if(tmp.spacemines<0) {
	printf("Spacemines %d: too low. Restoring to original\n",tmp.spacemines);
	tmp.spacemines=fleet[num].spacemines;
	confirm=1;
	}
for(x=0;x<10;x++)
	if(tmp.ore[x]<0) {
		printf("Ore %d %d out of range. Restoring to original\n",x,tmp.ore[x]);
		tmp.ore[x]=fleet[num].ore[x];
		}

tmp.cargleft=tmp.cargo-20*tmp.mines-10*tmp.ind-2*tmp.pdu-tmp.spacemines;
for(x=0;x<10;x++)
	tmp.cargleft-=tmp.ore[x];
tmp.engage=0;
tmp.moved=0;
tmp.hits=0;
tmp.pduhits=0;
tmp.figleft=tmp.fight;

printf("Ship %d details:\n",num+100);
printf("Name:%s\nOwner:%-2d %-9s\n",tmp.name,tmp.owner,name[tmp.owner]);
printf("Fighter:%-4d Cargo:%-4d Shield:%-4d Tractor:%-4d\n",tmp.fight,tmp.cargo,tmp.shield,tmp.tractor);
printf("Planet:%-4d Cargoleft:%-4d Efficiency:%-4d\n",tmp.planet+100,tmp.cargleft,tmp.efficiency);
printf("Standing:%-10s\n",tmp.stndord);
printf("Mines:%-4d Industries:%-4d PDUs:%-4d Spacemines:%-4d\n",tmp.mines,tmp.ind,tmp.pdu,tmp.spacemines);
for(x=0;x<10;x++)
	printf("R%d:%-3d ",x,tmp.ore[x]);
printf("\n");

printf("Confirm the changes to ship?");
fflush(stdin);
buff[0]=0;
while(buff[0]<'A')
	fgets(buff,sizeof(buff),stdin);
if(buff[0]=='n' || buff[0]=='N')
    printf("Ship change discarded\n");
else {
	bcopy(&tmp,&fleet[num],sizeof(tmp));
	fleet[num].type=CalcType(num);
	}
}

/*****************************************************************************/
void AnalyzeAlliance(char filename[80])
/*****************************************************************************/
{
FILE *ef;
char buff[80],*tok;
int x,y;
Flag tmpall[NUMPLAYERS+1][NUMPLAYERS+1];

/* File format
Alliance Details
ALLY:A	FRIEND:F	NEUTRAL:N	ENEMY:E
     0   1   2   3   4   5   6   7   8   
0    N   N   N   N   N   N   N   N   N   
1    N   N   N   N   N   N   N   N   N   
2    N   N   N   N   N   N   N   N   N   
3    N   N   N   N   N   A   N   N   N   
4    N   F   N   N   N   N   N   N   N   
5    N   N   N   N   E   N   N   N   N   
6    N   N   N   N   N   N   N   N   N   
7    N   N   N   N   N   N   N   N   N   
8    N   N   N   N   N   N   N   N   N   
*/

if((ef=fopen(filename,"r"))==NULL) {
	fprintf(stderr,"Could not open %s for reading\n",filename);
	return;
	}
fgets(buff,sizeof(buff),ef);	/* Absorb first three lines */
fgets(buff,sizeof(buff),ef);
fgets(buff,sizeof(buff),ef);
printf("Alliance Details\n");
printf("ALLY:%d\tFRIEND:%d\tNEUTRAL:%d\tENEMY:%d\n",ALLY,FRIEND,NEUTRAL,ENEMY);
printf("     ");
for(x=0;x<=NUMPLAYERS;x++)
	printf("%-3d ",x);
printf("\n");
for(x=0;x<=NUMPLAYERS;x++) {
	fgets(buff,sizeof(buff),ef);
	tok=strtok(buff,": \t");
	printf("%-3d  ",x);
	for(y=0;y<=NUMPLAYERS;y++) {
		tmpall[x][y]=(int)strtok(NULL,": \t")[0];
		switch(tmpall[x][y]) {
			case 'A':	case 'a':
				tmpall[x][y]=ALLY;
				break;
			case 'F':	case 'f':
				tmpall[x][y]=FRIEND;
				break;
			case 'N':	case 'n':
				tmpall[x][y]=NEUTRAL;
				break;
			case 'E':	case 'e':
				tmpall[x][y]=ENEMY;
				break;
			default:
				fprintf(stderr,"Invalid alliance level\n");
				tmpall[x][y]=NEUTRAL;
				}
		printf("%-3c ",allsym[tmpall[x][y]]);
		}
	printf("\n");
	}
fclose(ef);

printf("Confirm the changes to alliance?");
fgets(buff, sizeof(buff), stdin);
if(buff[0]=='n' || buff[0]=='N')
	printf("Alliance changes discarded\n");
else
	bcopy(&tmpall,&alliance,sizeof(tmpall));
}

/*****************************************************************************/
void AnalyzeName(char filename[80])
/*****************************************************************************/
{
FILE *ef;
char buff[80],*tok;
int x;
char tmpname[NUMPLAYERS+1][10];

/* File format
Name Details
0   One       
1   Two       
2   Three     
3   Four      
4   Five      
5   Six       
6   Seven     
7   Eight     
8   Nine      
9	NEUTRAL
*/

printf("Plr Orig      New\n");
printf("------------------------\n");
if((ef=fopen(filename,"r"))==NULL) {
	fprintf(stderr,"Could not open %s for reading\n",filename);
	return;
	}
fgets(buff,sizeof(buff),ef);		/* Absorb first line */

bzero(&tmpname,sizeof(tmpname));
for(x=0;x<NUMPLAYERS+1;x++) {
	fgets(buff,sizeof(buff),ef);
	tok=strtok(buff,": \t\n");
	while(tok!=NULL) {
		tok=strtok(NULL,": \t\n");
		if(tok==NULL)
			break;
		strncat(tmpname[x],tok,NAMESIZ);
		strncat(tmpname[x]," ",NAMESIZ);
		}
	tmpname[x][9]=0;

	printf("%-3d : %-10s : %-10s\n",x,name[x],tmpname[x]);
	}
fclose(ef);

printf("Confirm the changes to names?");
fgets(buff, sizeof(buff), stdin);
if(buff[0]=='n' || buff[0]=='N')
	printf("Name change discarded\n");
else
	bcopy(&tmpname,&name,sizeof(tmpname));
}

/*****************************************************************************/
void AnalyzeScore(char filename[80])
/*****************************************************************************/
{
FILE *ef;
char buff[80],*tok;
int x;
int tmpscore[NUMPLAYERS+1],tmpcred[NUMPLAYERS+1],tmpturn[NUMPLAYERS+1];

/* File format
Score Details
#   Score   Ecredits  DesTurn
0   1445   	1234      30
1   2054   	1334      30
2   2328   	1254      30
3   2326   	1634      30
4   2070   	7234      30
5   2311   	1934      30
6   2201   	1204      30
7   2339   	1231      30
8   2384   	1224      30
*/

printf("Plr OrigS NewSc OCred NCred OTurn NTurn\n");
if((ef=fopen(filename,"r"))==NULL) {
	fprintf(stderr,"Could not open %s for reading\n",filename);
	return;
	}

fgets(buff,sizeof(buff),ef);		/* Absorb first line */
fgets(buff,sizeof(buff),ef);		/* Absorb second line */

for(x=1;x<NUMPLAYERS+1;x++) {
	fgets(buff,sizeof(buff),ef);
	tok=strtok(buff,": \t");
	tmpscore[x]=atoi(strtok(NULL,": \t"));
	tmpcred[x]=atoi(strtok(NULL,": \t"));
	tmpturn[x]=atoi(strtok(NULL,": \t"));
	printf("%-3d %-5d %-5d %-5d %-5d %-5d %-5d\n",x,score[x],tmpscore[x],ecredit[x],tmpcred[x],desturn[x],tmpturn[x]);
	}
fclose(ef);

printf("Confirm the changes to player details?");
fgets(buff, sizeof(buff), stdin);
if(buff[0]=='n' || buff[0]=='N')
	printf("Score change discarded\n");
else
	for(x=1;x<NUMPLAYERS+1;x++) {
		score[x]=tmpscore[x];
		ecredit[x]=tmpcred[x];
		desturn[x]=tmpturn[x];
		}
}

/*****************************************************************************/
void Validate(void)
/*****************************************************************************/
{
int x,y;
Units cgl;
Number inc;

/* Validate ships */
printf("Validating ships\n");
/* Validate cargleft */
TREDIT(printf("Validating cargo :"));
for(x=0;x<shiptr;x++) {
	TREDIT(printf("."));
	if(fleet[x].ind<0) {
		fprintf(stderr,"Ship %d Industry %d\n",x,fleet[x].ind);
		continue;
		}
	if(fleet[x].mines<0) {
		fprintf(stderr,"Ship %d Mines %d\n",x,fleet[x].mines);
		continue;
		}
	if(fleet[x].pdu<0) {
		fprintf(stderr,"Ship %d PDUs %d\n",x,fleet[x].pdu);
		continue;
		}
	if(fleet[x].spacemines<0) {
		fprintf(stderr,"Ship %d Spacemines %d\n",x,fleet[x].spacemines);
		continue;
		}
	inc=CalcType(x);
	if(fleet[x].type!=inc) {
		fprintf(stderr,"Ship %d type mismatch Is:%s Was:%s\n",x+100,stypes[inc],stypes[fleet[x].type]);
		fleet[x].type=inc;
		}
	cgl=fleet[x].cargo;
	cgl-=fleet[x].mines*20;
	cgl-=fleet[x].ind*10;
	cgl-=fleet[x].pdu*2;
	cgl-=fleet[x].spacemines;
	for(y=0;y<10;y++) {
		cgl-=fleet[x].ore[y];
		if(fleet[x].ore[y]<0) {
			fprintf(stderr,"Ship %d Ore[%d] %d\n",x,y,fleet[x].ore[y]);
			continue;
			}
		}
	if(fleet[x].cargleft!=cgl) {
		printf("Cargoleft incompatable:%d: Calculated:%d Stored:%d\n",x,cgl,fleet[x].cargleft);
		fleet[x].cargleft=cgl;
		if(cgl<0) {
			printf("Too much cargo...\nPlease edit ship\n");
			getchar();
			ShipEdit(x);
			Validate();
			}
		}
	}
TREDIT(printf("\n"));

/* Validate planets */
printf("Validating planets\n");
/* Validate income */
TREDIT(printf("Validating income :"));
for(x=0;x<NUMPLANETS;x++) {
	TREDIT(printf("."));
	if(galaxy[x].owner>9) {
		fprintf(stderr,"Owner out of range planet:%d owner:%d\n",x,galaxy[x].owner);
		galaxy[x].owner=9;
		}
	inc=20+galaxy[x].ind*5;
	for(y=0;y<10;y++)
		inc+=galaxy[x].mine[y];
	if(galaxy[x].income!=inc) {
		printf("Incomes incompatable:%d: Calculated:%d Stored:%d\n",x,inc,galaxy[x].income);
		galaxy[x].income=inc;
		}
	}
TREDIT(printf("\n"));

/* Validate links */
TREDIT(printf("Validating links:"));
for(x=0;x<NUMPLANETS;x++) {
	TREDIT(printf("."));
	for(y=0;y<4;y++)
		if(galaxy[x].link[y]>=0)
			if(ValidateLink(x,galaxy[x].link[y]))
				printf("Invalid link:%d(%d):Link:%d\n",x,y,galaxy[x].link[y]);
	}
TREDIT(printf("\n"));
}

/*****************************************************************************/
int ValidateLink(Planet pln,Planet lnk)
/*****************************************************************************/
{
int x;

TREDIT(printf("ValidateLink(pln:%d,lnk:%d)\n",pln,lnk));

for(x=0;x<4;x++)
	if(galaxy[lnk].link[x]==pln)
		return(0);
return(1);
}

/*****************************************************************************/
void PriceEdit(void)
/*****************************************************************************/
{
}

/*****************************************************************************/
void GameEdit(void)
/*****************************************************************************/
/* Edit the winning conditions and the initial game details */
{
char tmpstr[80];		/* Temporary file name */
char systr[80];			/* System tmp string */
FILE *ef;				/* Edit file */

TREDIT(printf("GameEdit()\n"));
strcpy(tmpstr,"/tmp/ZDgeXXXXXX");
mktemp(tmpstr);
if((ef=fopen(tmpstr,"w"))==NULL) {
	fprintf(stderr,"Could not open %s for writing\n",tmpstr);
	return;
	}
PrintGame(ef);
fclose(ef);

sprintf(systr,"%s %s",editor,tmpstr);
(void)system(systr);

LoadProto(tmpstr);
remove(tmpstr);
}

/*****************************************************************************/
void PrintGame(FILE *file)
/*****************************************************************************/
/* Print the details of the game out to a file */
{
int count;

TREDIT(printf("PrintGame()\n"));
/*  Example file
Winning: Score=30000 Planets=64 Earth Income=3000 Credits=1500 Turn
Earthmult=1
Amnesty=15
Home: Ind=60 PDU=100 Spcmin=0 Deployed=0
HomeOre: 100 30 30 20 25 15 15 15 50 50
HomeMine: 5 3 3 1 2 1 1 0 0 0
Earth: Ind=60 PDU=200 Spcmine=0 Deployed=0 Flag=Limited
EarthOre: 30 30 30 30 30 30 30 30 30 30
EarthMine: 10 10 10 10 10 10 10 10 10 10
EarthBid: C=1 S=2 T=2 F=3
*/
fprintf(file,"Winning: ");
if(gamedet.winning&WPLANETS)
	fprintf(file,"Planets=%d ",gamedet.planets);
if(gamedet.winning&WSCORE)
	fprintf(file,"Score=%d ",gamedet.score);
if(gamedet.winning&WINCOME)
	fprintf(file,"Income=%d ",gamedet.income);
if(gamedet.winning&WCREDIT)
	fprintf(file,"Credits=%d ",gamedet.credits);
if(gamedet.winning&WEARTH)
	fprintf(file,"Earth");
if(gamedet.winning&WTURN)
	fprintf(file,"Turn");
if(gamedet.winning&WFIXTURN)
	fprintf(file,"=%d",gamedet.turn);
fprintf(file,"\nEarthmult=%d\n",gamedet.earth.earthmult);
fprintf(file,"Amnesty=%d\n",gamedet.earth.amnesty);
fprintf(file,"Home: Ind=%d PDU=%d Spcmin=%d Deployed=%d\n",gamedet.home.ind,gamedet.home.pdu,gamedet.home.spacemine,gamedet.home.deployed);
fprintf(file,"HomeOre: ");
for(count=0;count<10;count++) 
	fprintf(file,"%d ",gamedet.home.ore[count]);
fprintf(file,"\nHomeMine: ");
for(count=0;count<10;count++) 
	fprintf(file,"%d ",gamedet.home.mine[count]);
fprintf(file,"\nEarth: Ind=%d PDU=%d Spcmin=%d Deployed=%d Flag=",gamedet.earth.ind,gamedet.earth.pdu,gamedet.earth.spacemine,gamedet.earth.deployed);
if(gamedet.earth.flag&WBUYALLORE)
	fprintf(file,"Unlimited");
else 
	fprintf(file,"Limited");
fprintf(file,"\nEarthOre: ");
for(count=0;count<10;count++) 
	fprintf(file,"%d ",gamedet.earth.ore[count]);
fprintf(file,"\nEarthMine: ");
for(count=0;count<10;count++) 
	fprintf(file,"%d ",gamedet.earth.mine[count]);
fprintf(file,"\nEarthBid: C=%d S=%d T=%d F=%d\n",gamedet.earth.cbid,gamedet.earth.sbid,gamedet.earth.tbid,gamedet.earth.fbid);
fprintf(file,"Gal: Nomine=%d Extramine=%d Extraore=%d Hasind=%d Haspdu=%d\n",gamedet.gal.nomine,gamedet.gal.extramine,gamedet.gal.extraore,gamedet.gal.hasind,gamedet.gal.haspdu);
fprintf(file,"Ship1: Num=%d Fight=%d Cargo=%d Shield=%d Tractor=%d Eff=%d\n",gamedet.ship.num,gamedet.ship.fight,gamedet.ship.cargo,gamedet.ship.shield,gamedet.ship.tractor,gamedet.ship.eff);
fprintf(file,"Ship2: Num=%d Fight=%d Cargo=%d Shield=%d Tractor=%d Eff=%d\n",gamedet.ship2.num,gamedet.ship2.fight,gamedet.ship2.cargo,gamedet.ship2.shield,gamedet.ship2.tractor,gamedet.ship2.eff);
return;
}

/*****************************************************************************/
void UnitIncrease()
/*****************************************************************************/
/* Do research planet based unit increases */
{
char buff[80];
int num,ret=0;
int origc,origf,origt,origs,origeff,nowc,nowf,nowt,nows,noweff;

TREDIT(printf("UnitIncrease()\n"));
printf("Enter ship number (100-%d):",shiptr+99);
scanf("%d",&num);
num-=100;
if(num<0 || num>shiptr-1) {
	printf("Ship number out of range\n");
	return;
	}

nowc=origc=fleet[num].cargo;
nowf=origf=fleet[num].fight;
nowt=origt=fleet[num].tractor;
nows=origs=fleet[num].shield;
noweff=origeff=fleet[num].efficiency;

printf("Lower case increases by 20%%. Uppercase increases by 50%%\n");
printf("F..Fighter Increase\tC..Cargo Increase\n");
printf("S..Shield Increase\tT..Tractor Increase\n");
printf("E..Efficiency Increase\tU..Undo Changes\n");
printf("R..Return to main menu\n");
while(ret==0) {
	printf("Increase Menu> ");
	fgets(buff, sizeof(buff), stdin);
	switch(buff[0]) {
		case 'c':
			TWINC(nowc);
			break;
		case 'C':
			FFINC(nowc);
			break;
		case 'e':
		case 'E':
			noweff++;
			break;
		case 'f':
			TWINC(nowf);
			break;
		case 'F':
			FFINC(nowf);
			break;
		case 'r':
		case 'R':
			ret=1;
			break;
		case 's':
			TWINC(nows);
			break;
		case 'S':
			FFINC(nows);
			break;
		case 't':
			TWINC(nowt);
			break;
		case 'T':
			FFINC(nowt);
			break;
		case 'u':
		case 'U':
			nowc=origc;
			nowt=origt;
			nowf=origf;
			nows=origs;
			noweff=origeff;
			break;
		default:
			break;
		}
	if(nowc>2*origc && origc>10) {
		printf("Cannot more than double units in a turn\n");
		nowc=origc*2;
		}
	if(nowc>MAXUNIT) {
		printf("Cannot increase units to more than %d\n",MAXUNIT);
		nowc=MAXUNIT;
		}
	if(nowf>2*origf && origf>10) {
		printf("Cannot more than double units in a turn\n");
		nowf=origf*2;
		}
	if(nowf>MAXUNIT) {
		printf("Cannot increase units to more than %d\n",MAXUNIT);
		nowf=MAXUNIT;
		}
	if(nowt>2*origt && origt>10) {
		printf("Cannot more than double units in a turn\n");
		nowt=origt*2;
		}
	if(nowt>MAXUNIT) {
		printf("Cannot increase units to more than %d\n",MAXUNIT);
		nowt=MAXUNIT;
		}
	if(nows>2*origs && origs>10) {
		printf("Cannot more than double units in a turn\n");
		nows=origs*2;
		}
	if(nows>MAXUNIT) {
		printf("Cannot increase units to more than %d\n",MAXUNIT);
		nows=MAXUNIT;
		}
	printf("Fighter(Orig:%d,Now:%d)\tCargo(Orig:%d,Now:%d)\n",origf,nowf,origc,nowc);
	printf("Shield(Orig:%d,Now:%d)\tTractor(Orig:%d,Now:%d)\n",origs,nows,origt,nowt);
	printf("Efficiency(Orig:%d,Now:%d)\n",origeff,noweff);
	}
fleet[num].cargo=nowc;
fleet[num].cargleft+=nowc-fleet[num].cargo;
fleet[num].fight=nowf;
fleet[num].tractor=nowt;
fleet[num].shield=nows;
fleet[num].efficiency=noweff;
fleet[num].type=CalcType(num);
return;
}
