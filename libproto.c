/* Celestial Empire Proto file analysis library 
 * (c) 1992 Dougal Scott

$Header: /nelstaff/edp/dwagon/rfs/RCS/libproto.c,v 1.50 1993/10/20 03:56:53 dwagon Exp $
$Log: libproto.c,v $
 * Revision 1.50  1993/10/20  03:56:53  dwagon
 * Added turn winning condition
 *
 * Revision 1.49  1993/10/11  10:12:35  dwagon
 * Added Earth behaviour flags
 *
 * Revision 1.48  1993/09/16  04:54:58  dwagon
 * Added new winning condition: Earth Credits
 *
 * Revision 1.47  1993/07/15  06:44:37  dwagon
 * Added second initial ship type
 *
 * Revision 1.46  1993/05/24  04:11:21  dwagon
 * Added income as a winning condition to the protofile
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  13:57:16  dwagon
 * Initial RCS'd version
 *

Example proto file:
# Winning
Winning: Score=30000 Planets=64 Earth Income=3000 Credits=3000 Turn=30
# Earthmult
Earthmult=1
# Amnesty
Amnesty=15
# Home
Home: Ind=60 PDU=100 Spcmin=0 Deployed=0
# Earth Ore
EarthOre: 30 30 30 30 30 30 30 30 30 30
# Earth Mine
EarthMine: 10 10 10 10 10 10 10 10 10 10
# Home Ore
HomeOre: 100 30 30 20 25 15 15 15 50 50
# Home Mine
HomeMine: 5 3 3 1 2 1 1 0 0 0
# Earth
Earth: Ind=60 PDU=200 Spcmine=0 Deployed=0 Flag=Limited
# Earth Bids
EarthBid: C=1 S=2 T=2 F=3
# Galaxy setup details
Gal: Nomine=5 Extramine=5 Extraore=20 Hasind=10 Haspdu=5
# Initial Ship details
Ship: Num=5 Fight=1 Cargo=10 Shield=0 Tractor=0 Eff=0
Ship2: Num=5 Fight=1 Cargo=10 Shield=0 Tractor=0 Eff=0
*/

#define TRPROT(x)	if(strstr(dbgstr,"PROTO") || strstr(dbgstr,"proto")) x

#include "def.h"

extern game gamedet;
extern char *dbgstr;

/*****************************************************************************/
void LoadProto(char *protofile)
/*****************************************************************************/
/* Load the game details structure with info from the protofile */
{
FILE *pf;			/* File pointer to protofile */
char buff[80];		/* Input buffer */
char *tok;			/* Token under examination */
int line=0;			/* Line number of protofile being examined */

TRPROT(printf("LoadProto(protofile:%s)\n",protofile));
if((pf=fopen(protofile,"r"))==NULL) {
	fprintf(stderr,"Could not open %s for reading\n",protofile);
	exit(-1);
	}
gamedet.winning=0;
fgets(buff,80,pf);
while(!feof(pf)) {
	line++;
	while(buff[0]=='#') {		/* Ignore all lines that are comments */
		TRPROT(printf("# %s",buff));
		fgets(buff,80,pf);
		TRPROT(printf("Got comment line %d from protofile\n",line));
		line++;
		}
	TRPROT(printf("Got line %d from protofile\n",line));
	tok=strtok(buff,":=\n \t");
/* Get the Earth multiplication field */
	if(strcasecmp(tok,"Earthmult")==0) {
		TRPROT(printf("Analyzing Earth Multiplier number\n"));
		tok=strtok(NULL,"\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Earthmult should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.earthmult=atoi(tok);
		}
/* Get the amnesty turn number */
	if(strcasecmp(tok,"Amnesty")==0) {
		TRPROT(printf("Analyzing Earth Amnesty turn number\n"));
		tok=strtok(NULL,"\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Amnesty should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.amnesty=atoi(tok);
		}
	if(strcasecmp(tok,"Winning")==0)
		ProtoWinning(buff,tok,line);
	if(strcasecmp(tok,"Home")==0)
		ProtoHome(buff,tok,line);
	if(strcasecmp(tok,"HomeOre")==0)
		ProtoHomeOre(buff,tok,line);
	if(strcasecmp(tok,"HomeMine")==0)
		ProtoHomeMine(buff,tok,line);
	if(strcasecmp(tok,"Earth")==0)
		ProtoEarth(buff,tok,line);
	if(strcasecmp(tok,"EarthOre")==0)
		ProtoEarthOre(buff,tok,line);
	if(strcasecmp(tok,"EarthMine")==0)
		ProtoEarthMine(buff,tok,line);
	if(strcasecmp(tok,"EarthBid")==0)
		ProtoEarthBid(buff,tok,line);
	if(strcasecmp(tok,"Gal")==0)
		ProtoGal(buff,tok,line);
	if(strcasecmp(tok,"Ship1")==0)
		ProtoShip(buff,tok,line);
	if(strcasecmp(tok,"Ship2")==0)
		ProtoShip2(buff,tok,line);
	fgets(buff,80,pf);
	}
	fclose(pf);
	return;
}

/*****************************************************************************/
void ProtoWinning(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the winning conditions from the prototype file */
{
int flag=0;
char *tok2;

TRPROT(printf("ProtoWinning(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \n\t");tok!=NULL;tok=strtok(NULL,"= \n\t")) {
/* Is score part of the winning condition */
	if(strcasecmp(tok,"Score")==0) {
		TRPROT(printf("\tAnalyzing the winning score\n"));
		flag=1;
		tok=strtok(NULL,"= \n\t");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Score should be a number\n",line);
			exit(-1);
			}
		gamedet.score=atoi(tok);
		gamedet.winning|=WSCORE;
		TRPROT(printf("\t\tWinning score=%d\tFlag=%d\n",gamedet.score,gamedet.winning));
		}
/* Are planets part of the winning condition */
	if(strcasecmp(tok,"Planets")==0) {
		TRPROT(printf("\tAnalyzing the winning planets\n"));
		flag=1;
		tok=strtok(NULL,"= \n\t");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Planets should be a number\n",line);
			exit(-1);
			}
		gamedet.planets=atoi(tok);
		gamedet.winning|=WPLANETS;
		TRPROT(printf("\t\tWinning planets=%d\tFlag=%d\n",gamedet.planets,gamedet.winning));
		}
/* Is income part of the winning condition */
	if(strcasecmp(tok,"Income")==0) {
		TRPROT(printf("\tAnalyzing the winning income\n"));
		flag=1;
		tok=strtok(NULL,"= \n\t");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Income should be a number\n",line);
			exit(-1);
			}
		gamedet.income=atoi(tok);
		gamedet.winning|=WINCOME;
		TRPROT(printf("\t\tWinning income=%d\tFlag=%d\n",gamedet.income,gamedet.winning));
		}
/* Are earthcredits part of the winning condition */
	if(strcasecmp(tok,"Credits")==0) {
		TRPROT(printf("\tAnalyzing the winning credits\n"));
		flag=1;
		tok=strtok(NULL,"= \n\t");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Credits should be a number\n",line);
			exit(-1);
			}
		gamedet.credits=atoi(tok);
		gamedet.winning|=WCREDIT;
		TRPROT(printf("\t\tWinning credits=%d\tFlag=%d\n",gamedet.credits,gamedet.winning));
		}
/* Is there a turn limit on the game */
	if(strcasecmp(tok,"Turn")==0) {
		TRPROT(printf("\tAnalyzing the turn length restriction\n"));
		flag=1;
		gamedet.winning|=WTURN;
		tok2=strtok(NULL,"= \n\t");
		if(tok2==NULL) {
			TRPROT(printf("\t\tWinning turn variable"));
			gamedet.turn=30;
			}
		else {
			if(isdigit(tok2[0])) {
				gamedet.turn=atoi(tok);
				gamedet.winning|=WFIXTURN;
				TRPROT(printf("\t\tWinning turn=%d\n",gamedet.turn));
				}
			else
				fprintf(stderr,"Invalid turn type on line %d\n",line);
			}
		}
/* Is Earth part of the winning condition */
	if(strcasecmp(tok,"Earth")==0) {
		TRPROT(printf("\tAnalyzing the Earth winning condition\n"));
		flag=1;
		gamedet.winning|=WEARTH;
		TRPROT(printf("\t\tWinning condition=%d\n",gamedet.winning));
		}
	}
if(!flag) {
	fprintf(stderr,"Syntax error on line %d:\nWinning: must be followed by \
	one or more of `Score', `Planets', `Income', `Credits', `Turn' or\
	`Earth'\n",line);
	exit(-1);
	}
TRPROT(printf("Finished in ProtoWinning\n"));
return;
}

/*****************************************************************************/
void ProtoHome(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the Home planet details from the prototype file */
{
int flag=0;

TRPROT(printf("ProtoHome(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \t\n");tok!=NULL;tok=strtok(NULL,"= \t\n")) {
	/* Get the amount of industry on the home planet */
	if(strcasecmp(tok,"Ind")==0) {
		TRPROT(printf("\tAnalyzing the home industry\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Industry should be a number\n",line);
			exit(-1);
			}
		gamedet.home.ind=atoi(tok);
		TRPROT(printf("\t\tHome industry=%d\n",gamedet.home.ind));
		}
	/* Get the number of PDUs on the home planet initially */
	if(strcasecmp(tok,"PDU")==0) {
		TRPROT(printf("\tAnalyzing the home pdus\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: PDUs should be a number\n",line);
			exit(-1);
			}
		gamedet.home.pdu=atoi(tok);
		TRPROT(printf("\t\tHome PDUs=%d\n",gamedet.home.pdu));
		}
	/* Get the number of Spacemines on the home planet initially */
	if(strcasecmp(tok,"spcmin")==0) {
		TRPROT(printf("\tAnalyzing the home spacemines\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Spacemines should be a number\n",line);
			exit(-1);
			}
		gamedet.home.spacemine=atoi(tok);
		TRPROT(printf("\t\tHome spacemines=%d\n",gamedet.home.spacemine));
		}
	/* Get the number of deployed spacemines on the home planet initially */
	if(strcasecmp(tok,"deployed")==0) {
		TRPROT(printf("\tAnalyzing the home deployed spacemines\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Deployed should be a number\n",line);
			exit(-1);
			}
		gamedet.home.deployed=atoi(tok);
		TRPROT(printf("\t\tHome deployed=%d\n",gamedet.home.deployed));
		}
	} 
if(!flag) {
	fprintf(stderr,"Syntax error on line %d:\nHome: must be followed by one or more of `Ind', `PDU', `Spcmin' or `Deployed'\n",line);
	exit(-1);
	}
TRPROT(printf("Finished in ProtoHome\n"));
return;
}

/*****************************************************************************/
void ProtoEarth(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the Earth details from the prototype file */
{
int flag=0;

TRPROT(printf("ProtoEarth(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \t\n");tok!=NULL;tok=strtok(NULL,"= \t\n")) {
/* Get the amount of industry on Earth */
	if(strcasecmp(tok,"Ind")==0) {
		TRPROT(printf("Analyzing the Earth industry\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Industry should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.ind=atoi(tok);
		TRPROT(printf("earth.ind=%d\n",gamedet.earth.ind));
		}
	/* Get the number of PDUs on Earth initially */
	if(strcasecmp(tok,"PDU")==0) {
		TRPROT(printf("Analyzing the Earth PDUs\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: PDUs should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.pdu=atoi(tok);
		TRPROT(printf("earth.pdu=%d\n",gamedet.earth.pdu));
		}
	/* Get the number of Spacemines on Earth initially */
	if(strcasecmp(tok,"spcmin")==0) {
		TRPROT(printf("Analyzing the Earth spacemines\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Spacemines should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.spacemine=atoi(tok);
		TRPROT(printf("earth.spacemine=%d\n",gamedet.earth.spacemine));
		}
	/* Get the number of deployed spacemines on Earth initially */
	if(strcasecmp(tok,"deployed")==0) {
		TRPROT(printf("Analyzing the Earth deployed spacemines\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Deployed should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.deployed=atoi(tok);
		TRPROT(printf("earth.deployed=%d\n",gamedet.earth.deployed));
		}
	if(strcasecmp(tok,"flag")==0) {
		TRPROT(printf("Analyzing the Earth behaviour flag\n"));
		flag=1;
		tok=strtok(NULL,"= \t\n");
		gamedet.earth.flag=0;
		if(strcasecmp(tok,"limited")==0) 
			gamedet.earth.flag|=WBUY100ORE;
		else if(strcasecmp(tok,"unlimited")==0) 
			gamedet.earth.flag|=WBUYALLORE;
		else {
			fprintf(stderr,"Error in protofile line %d: Earth flag should be either \"Limited\" or \"Unlimited\"\n", line);
			exit(-1);
			}
		TRPROT(printf("earth.deployed=%d\n",gamedet.earth.deployed));
		}
	}
if(!flag) {
	fprintf(stderr,"Syntax error on line %d:\nEarth: must be followed by one or more of `Ind', `PDU', `Spcmin', `Deployed' or `Flag'\n",line);
	exit(-1);
	}
TRPROT(printf("Finished in ProtoEarth\n"));
return;
}

/*****************************************************************************/
void ProtoHomeOre(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the home planets ore from the prototype file */
{
int cnt;

TRPROT(printf("ProtoHomeOre(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(cnt=0;cnt<10;cnt++) {
	TRPROT(printf("Analyzing the home ore type %d\n",cnt));
	tok=strtok(NULL," \t\n");
	if(!isdigit(tok[0])) {
		fprintf(stderr,"Error in protofile line %d: Ore type %d not a number\n",line,cnt);
		exit(-1);
		}
	gamedet.home.ore[cnt]=atoi(tok);
	}
TRPROT(printf("Finished in ProtoHomeOre\n"));
return;
}

/*****************************************************************************/
void ProtoEarthOre(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the Earth ore from the prototype file */
{
int cnt;

TRPROT(printf("ProtoEarthOre(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(cnt=0;cnt<10;cnt++) {
	TRPROT(printf("Analyzing the Earth ore type %d\n",cnt));
	tok=strtok(NULL," \t\n");
	if(!isdigit(tok[0])) {
		fprintf(stderr,"Error in protofile line %d: Ore type %d not a number\n",line,cnt);
		exit(-1);
		}
	gamedet.earth.ore[cnt]=atoi(tok);
	}
TRPROT(printf("Finished in ProtoEarthOre\n"));
return;
}

/*****************************************************************************/
void ProtoHomeMine(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the home planet mines from the prototype file */
{
int cnt;

TRPROT(printf("ProtoHomeMine(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(cnt=0;cnt<10;cnt++) {
	TRPROT(printf("Analyzing the home mine type %d\n",cnt));
	tok=strtok(NULL," \t\n");
	if(!isdigit(tok[0])) {
		fprintf(stderr,"Error in protofile line %d: Mine type %d not a number\n",line,cnt);
		exit(-1);
		}
	gamedet.home.mine[cnt]=atoi(tok);
	}
TRPROT(printf("Finished in ProtoHomeMine\n"));
return;
}

/*****************************************************************************/
void ProtoEarthMine(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the Earth mines from the prototype file */
{
int cnt;

TRPROT(printf("ProtoEarthMine(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(cnt=0;cnt<10;cnt++) {
	TRPROT(printf("\tAnalyzing the Earth mine type %d\n",cnt));
	tok=strtok(NULL," \t\n");
	if(!isdigit(tok[0])) {
		fprintf(stderr,"Error in protofile line %d: Mine type %d not a number\n",line,cnt);
		exit(-1);
		}
	gamedet.earth.mine[cnt]=atoi(tok);
	TRPROT(printf("\t\tEarth.mine[%d]=%d\n",cnt,gamedet.earth.mine[cnt]));
	}
TRPROT(printf("Finished in ProtoEarthMine\n"));
return;
}

/*****************************************************************************/
void ProtoEarthBid(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the Earth bids from the prototype file */
{
TRPROT(printf("ProtoEarthBid(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \t\n");tok!=NULL;tok=strtok(NULL,"= \t\n")) {
	if(strcasecmp(tok,"C")==0) {
		TRPROT(printf("Analyzing the Earth Cargo Bid\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Cargo bid should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.cbid=atoi(tok);
		TRPROT(printf("earth.cbid=%d\n",gamedet.earth.cbid));
		}
	if(strcasecmp(tok,"F")==0) {
		TRPROT(printf("Analyzing the Earth Fighter Bid\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Fighter bid should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.fbid=atoi(tok);
		TRPROT(printf("earth.fbid=%d\n",gamedet.earth.fbid));
		}
	if(strcasecmp(tok,"T")==0) {
		TRPROT(printf("Analyzing the Earth Tractor Bid\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Tractor bid should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.tbid=atoi(tok);
		TRPROT(printf("earth.tbid=%d\n",gamedet.earth.tbid));
		}
	if(strcasecmp(tok,"S")==0) {
		TRPROT(printf("Analyzing the Earth Shield Bid\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Shield bid should be a number\n",line);
			exit(-1);
			}
		gamedet.earth.sbid=atoi(tok);
		TRPROT(printf("earth.sbid=%d\n",gamedet.earth.sbid));
		}
	}
return;
}

/*****************************************************************************/
void ProtoShip(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the initial ships from the prototype file */
{
TRPROT(printf("ProtoShip(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \t\n");tok!=NULL;tok=strtok(NULL,"= \t\n")) {

	if(strcasecmp(tok,"Num")==0) {
		TRPROT(printf("Analyzing the Ship numbers\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Ship number should be a number\n",line);
			exit(-1);
			}
		gamedet.ship.num=atoi(tok);
		TRPROT(printf("ship.num=%d\n",gamedet.ship.num));
		}

	if(strcasecmp(tok,"Fight")==0) {
		TRPROT(printf("Analyzing the inital Fighter units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Fight should be a number\n",line);
			exit(-1);
			}
		gamedet.ship.fight=atoi(tok);
		TRPROT(printf("ship.fight=%d\n",gamedet.ship.fight));
		}

	if(strcasecmp(tok,"Cargo")==0) {
		TRPROT(printf("Analyzing the initial Cargo units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Cargo should be a number\n",line);
			exit(-1);
			}
		gamedet.ship.cargo=atoi(tok);
		TRPROT(printf("ship.cargo=%d\n",gamedet.ship.cargo));
		}

	if(strcasecmp(tok,"Shield")==0) {
		TRPROT(printf("Analyzing the initial Shield units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Shield should be a number\n",line);
			exit(-1);
			}
		gamedet.ship.shield=atoi(tok);
		TRPROT(printf("ship.shield=%d\n",gamedet.ship.shield));
		}

	if(strcasecmp(tok,"Tractor")==0) {
		TRPROT(printf("Analyzing the initial Tractor units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Tractor should be a number\n",line);
			exit(-1);
			}
		gamedet.ship.tractor=atoi(tok);
		TRPROT(printf("ship.tractor=%d\n",gamedet.ship.tractor));
		}

	if(strcasecmp(tok,"Eff")==0) {
		TRPROT(printf("Analyzing the initial efficiency\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Eff should be a number\n",line);
			exit(-1);
			}
		gamedet.ship.eff=atoi(tok);
		TRPROT(printf("ship.eff=%d\n",gamedet.ship.eff));
		}

	}
return;
}

/*****************************************************************************/
void ProtoGal(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the Earth bids from the prototype file */
{
TRPROT(printf("ProtoGal(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \t\n");tok!=NULL;tok=strtok(NULL,"= \t\n")) {

	if(strcasecmp(tok,"Nomine")==0) {
		TRPROT(printf("Analyzing the nomine chance\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Nomine should be a number\n",line);
			exit(-1);
			}
		gamedet.gal.nomine=atoi(tok);
		TRPROT(printf("gal.nomine=%d\n",gamedet.gal.nomine));
		}

	if(strcasecmp(tok,"Extramine")==0) {
		TRPROT(printf("Analyzing the extramine chance\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Extramine should be a number\n",line);
			exit(-1);
			}
		gamedet.gal.extramine=atoi(tok);
		TRPROT(printf("gal.extramine=%d\n",gamedet.gal.extramine));
		}

	if(strcasecmp(tok,"Extraore")==0) {
		TRPROT(printf("Analyzing the Extraore chance\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Extraore should be a number\n",line);
			exit(-1);
			}
		gamedet.gal.extraore=atoi(tok);
		TRPROT(printf("gal.extraore=%d\n",gamedet.gal.extraore));
		}

	if(strcasecmp(tok,"Hasind")==0) {
		TRPROT(printf("Analyzing the hasind chance\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: hasind should be a number\n",line);
			exit(-1);
			}
		gamedet.gal.hasind=atoi(tok);
		TRPROT(printf("gal.hasind=%d\n",gamedet.gal.hasind));
		}

	if(strcasecmp(tok,"Haspdu")==0) {
		TRPROT(printf("Analyzing the haspdu chance\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: haspdu should be a number\n",line);
			exit(-1);
			}
		gamedet.gal.haspdu=atoi(tok);
		TRPROT(printf("gal.haspdu=%d\n",gamedet.gal.haspdu));
		}
	}

return;
}

/*****************************************************************************/
void ProtoShip2(char buff[80],char *tok,int line)
/*****************************************************************************/
/* Analyze the details of the second set of initial ships from the
 * prototype file */
{
TRPROT(printf("ProtoShip(buff:%s,tok:%s,line:%d)\n",buff,tok,line));
for(tok=strtok(NULL,"= \t\n");tok!=NULL;tok=strtok(NULL,"= \t\n")) {

	if(strcasecmp(tok,"Num")==0) {
		TRPROT(printf("Analyzing the Ship numbers\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Ship number should be a number\n",line);
			exit(-1);
			}
		gamedet.ship2.num=atoi(tok);
		TRPROT(printf("ship2.num=%d\n",gamedet.ship2.num));
		}

	if(strcasecmp(tok,"Fight")==0) {
		TRPROT(printf("Analyzing the inital Fighter units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Fight should be a number\n",line);
			exit(-1);
			}
		gamedet.ship2.fight=atoi(tok);
		TRPROT(printf("ship2.fight=%d\n",gamedet.ship2.fight));
		}

	if(strcasecmp(tok,"Cargo")==0) {
		TRPROT(printf("Analyzing the initial Cargo units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Cargo should be a number\n",line);
			exit(-1);
			}
		gamedet.ship2.cargo=atoi(tok);
		TRPROT(printf("ship2.cargo=%d\n",gamedet.ship2.cargo));
		}

	if(strcasecmp(tok,"Shield")==0) {
		TRPROT(printf("Analyzing the initial Shield units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Shield should be a number\n",line);
			exit(-1);
			}
		gamedet.ship2.shield=atoi(tok);
		TRPROT(printf("ship2.shield=%d\n",gamedet.ship2.shield));
		}

	if(strcasecmp(tok,"Tractor")==0) {
		TRPROT(printf("Analyzing the initial Tractor units\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Tractor should be a number\n",line);
			exit(-1);
			}
		gamedet.ship2.tractor=atoi(tok);
		TRPROT(printf("ship2.tractor=%d\n",gamedet.ship2.tractor));
		}

	if(strcasecmp(tok,"Eff")==0) {
		TRPROT(printf("Analyzing the initial efficiency\n"));
		tok=strtok(NULL,"= \t\n");
		if(!isdigit(tok[0])) {
			fprintf(stderr,"Error in protofile line %d: Eff should be a number\n",line);
			exit(-1);
			}
		gamedet.ship2.eff=atoi(tok);
		TRPROT(printf("ship2.eff=%d\n",gamedet.ship2.eff));
		}

	}
return;
}

