/* TRANS.C      This takes in the files containing the text cmnds    */
/* and converts them into numerical lists as a disk file        */
/* (c) 2016 Dougal Scott */

/* Details: This program takes the files plr[0-8] which contain the players
 * moves, one per line and parses them outputing the following files:
 * bids - which contains a list of all the bids for earth industry sorted in
 *        descending bid amount. The file is ASCII with 5 numbers per bid
 *        "ship number" "type c,f,t,s" "amount to bld" "bid per ind" "plr"
 * cmdout - The actual list of commands sorted into command sequence, one
 *          command per line. The command is just made up of a sequence of
 *          numbers with the following format:
 *          "plr #" "cmnd #" ["Operand"] ... ["Operand"] -100
 * exhist.[1-9] - The execution history files, when a command is mis-
 *                interpreted, the details are put in these files, to be
 *                subsequently put in the turns sheets command history by
 *                the `turn' program.
 * sord.[1-9] - Standing order files. Initially the galaxy and fleet 
 *              structures are searched for standing orders, which are
 *              written to the sord file, which are then parsed in the
 *              same way the plr files are. These are deleted after use
 *              unless TRTRN debugging is turned on.
 * transout - When OUTTR is turned on, this file contains the commands that
 *            have been parsed in pseudo english language.
 * motd - File that broadcasts are put into for use with turn
 * spec.[1-9] - Files that personal messages are put into for each player
 */

/* Temporary files that are used by trans:
 * /tmp/ZDBXXXXXX is used to store the bids before they are sorted
 * /tmp/ZDTXXXXXX is used to store the cmnds before they are sorted
 */

#include "def.h"
#include "trans.h"
#include "cmnd.h"
#include "allname.h"

Number score[NUMPLAYERS+1];
Number ecredit[NUMPLAYERS+1];
planet galaxy[NUMPLANETS];
game gamedet;
ship fleet[NUMSHIPS];
int turn,gm;
Ship shiptr;
Number price[10];
Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
char *dbgstr;

int 	cmdarr[NUMPLAYERS+1][LENGTH];
FILE    *outfile;      	/* Pointer to in and outfiles   */
FILE    *trns[NUMPLAYERS+1];  /* File to put commands executed */
FILE	*sord[NUMPLAYERS+1];	/* File to put standing orders */
int     idx;  			/* Index to the next blank command space in cmdarr    */
Player 	plr;    		/* Current plr       */
char    cmnd[255];    	/* Current line being parsed    */
int     sbj;        	/* Subject being considered     */
FILE    *df,*nf;		/* File desc for debug type */
FILE    *bidfp;			/* File containing all the bids */
char 	name[NUMPLAYERS+1][10];	/* Name array */
char 	temp[80],temp2[80];
char 	*path;
int		desturn[NUMPLAYERS+1];

/*****************************************************************************/
int main(int argc, char **argv)
/*****************************************************************************/
/* Do the lot */
{
    char *gmstr;
    char str[124];

    printf("Celestial Empire Version %d.%d Turn translation program\n",VERSION,PATCHLEVEL);
    if((dbgstr = getenv("CELEMPDEBUG")) == NULL )
        dbgstr=(char *)"null";

    if((path = getenv("CELEMPPATH")) == NULL) {
        fprintf(stderr,"set CELEMPPATH to the appropriate directory\n");
        return(-1);
        }

    if(argc==2) {
        gm=atoi(argv[1]);
    }
    else {
        if((gmstr = getenv("CELEMPGAME")) == NULL) {
            fprintf(stderr,"set CELEMPGAME to the appropriate number or spacify on command list\n");
            return(-1);
            }
        gm=atoi(gmstr);
        }

    TRTRN(printf("Reading database...\n"));
    ReadGalflt();

    TRTRN(printf("Opening execution histories\n"));
    if(OpenExhist("w")==-1) {
        fprintf(stderr,"Program terminated\n");
        exit(-1);
        }

    TRTRN(printf("Processing old standing orders\n"));
    OpenSord("w");
    WriteSord();
    CloseSord();
    OpenBid();
        
    TRTRN(printf("Translation Program Running...\n"));
    InitArray();
    OUTTR(OpenDebug());
    ReadIn();
    OUTTR(fclose(df));

    TRTRN(printf("Closing execution histories\n"));
    CloseExhist();

    TRTRN(printf("Writing database...\n"));
    WriteGalflt();
    CloseBid();

    TRTRN(printf("Dumping cmdarr to disk\n"));
    CmdDump();
    TRTRN(printf("Sorting array\n"));
    sprintf(str,"sort -b +1n -o %s%d/cmdout %s",path,gm,temp);
    (void)system(str);
    TRTRN(printf("Sorting bids\n"));
    sprintf(str,"sort -b +3nr -o %s%d/bids %s",path,gm,temp2);
    (void)system(str);
    TRTRN(printf("Finished sorting\n"));
    TRTRN(if(0))
    remove(temp);
    TRTRN(if(0))
    remove(temp2);
    return(0);
}

/*****************************************************************************/
void WriteSord(void)
/*****************************************************************************/
/* Write the standing order for each planet and ship to the sord file */
{
    int x;

    for(x=0;x<NUMPLANETS;x++)
        if(galaxy[x].stndord[0]!=0 && galaxy[x].owner!=NEUTPLR)
            fprintf(sord[galaxy[x].owner],"%d%s\n",x+100,galaxy[x].stndord);
    for(x=0;x<shiptr;x++)
        if(fleet[x].stndord[0]!=0 && fleet[x].owner!=NEUTPLR)
            fprintf(sord[fleet[x].owner],"S%d%s\n",x+100,fleet[x].stndord);
    return;
}

/*****************************************************************************/
void OpenDebug(void)
/*****************************************************************************/
{
    char str[80];

    TRTRN(printf("OpenDebug\n"));
    sprintf(str,"%s%d/transout",path,gm);
    if((df=fopen(str,"w"))==NULL) {
        fprintf(stderr,"Couldn't open the debug file %s\n",str);
        exit(0);
        }
    return;
}

/*****************************************************************************/
void InitArray(void)
/*****************************************************************************/
{
    int count1,count2;

    TRTRN(printf("InitArray\n"));
    /* Clear the command array */
    for(count1=0;count1<NUMPLAYERS+1;count1++)
        for(count2=0;count2<LENGTH;count2++)
            cmdarr[count1][count2]= BLANK;
    return;
}

/*****************************************************************************/
void ReadIn(void)
/*****************************************************************************/
/* Read in the information from the plr number 'plr's file   */
{
    char infname[80];
    FILE *infile;
    char *flag;

    TRTRN(printf("ReadIn()\n"));

    /* Open the file containing all the plrs cmnd strings     */
    for(plr=1;plr<NUMPLAYERS+1;plr++) {
    /* Get new orders */
        OUTTR(fprintf(df,"** PLR %d **\n",plr));
        fprintf(stdout,"Processing player %d\n",plr);
        idx=0;
        sprintf(infname,"%s%d/plr%d",path,gm,plr);
        if ((infile = fopen(infname,"r")) == NULL) {
            fprintf(stderr,"ERROR! Cannot open %s for reading.\n",infname);
            }
        else {
            /* Read in and pass to parse the strings from the file  */
            flag=(char *)1;
            while (flag!=NULL) {
                flag=fgets(cmnd,255,infile);
                if(flag==NULL)
                    break;
                Parse();
                Prnt(ENDCOMMAND);
                OUTTR(fflush(df));
                }
            fclose(infile);
            }
        OUTTR(fprintf(df,"* STAND PLR %d *\n",plr));
        sprintf(infname,"%s%d/sord.%d",path,gm,plr);
        if((infile=fopen(infname,"r"))==NULL) {
            fprintf(stderr,"ERROR! Cannot open %s for reading.\n",infname);
            continue;
            }

        /* Read in and pass to parse the strings from the file */
        flag=(char *)1;
        while(flag!=NULL) {
            flag=fgets(cmnd,255,infile);
            if(flag==NULL)
                break;
            Parse();
            Prnt(ENDCOMMAND);
            OUTTR(fflush(df));
        }

        /* Close the file */
        fclose(infile);
        }
    return;
}

/*****************************************************************************/
void Prnt(int numb)
/*****************************************************************************/
/* Print the cmnds resultant numbers  */
/* and stick them into the array        */
{
    TRTRN2(printf("Prnt(numb: %4d) idx:%4d\n",numb,idx));

    cmdarr[plr][idx++]=numb;
}

/*****************************************************************************/
void Parse(void)
/*****************************************************************************/
/* Parse the cmnd string into its various sections and add   */
/* them to the data structures  */
{
    char *p;
    int i;

    TRTRN(printf("Parse(cmnd[0]=%c)\n",cmnd[0]));
    OUTTR(fprintf(df,"%s:",cmnd));

    /* Strip any trailing comments */
    if((p=strchr(cmnd,';'))!=NULL)
        p=NULL;		/* Truncates from ';' onwards */

    for(i=strlen(cmnd);isspace(cmnd[i]);i--)	/* Strip trailing whitespace */
        cmnd[i]=0;

    switch(cmnd[0]) {
        case ' ':	break;
        case ';':	break;
        case '+':
        case '-': 	ChangeAlliance();
                    break;
        case '{':	Broadcast();
                    break;
        case '&':	Personal();
                    break;
        case '(':	AllMsg();
                    break;
        case 'O':
        case 'o':
                    SetSord();
                    break;
        case 'X':
        case 'x':
                    ClearSord();
                    break;
        case 'S':
        case 's':	if((cmnd[1]=='c' || cmnd[1]=='C' )
                       && (cmnd[2]=='a' || cmnd[2]=='A')
                       && (cmnd[3]=='n' || cmnd[3]=='N'))
                        Scan();
                    else {
                        sscanf(&cmnd[1],"%d",&sbj);
                        ShpPar();
                        }
                    break;
        case 't':
        case 'T':	if((cmnd[1]=='u' || cmnd[1]=='U')
                       && (cmnd[2]=='r' || cmnd[2]=='R')
                       && (cmnd[3]=='n' || cmnd[3]=='N'))
                        GameLength();
                    else
                        fprintf(trns[plr],"Unknown command:%s\n",cmnd);
                    break;
        case '1':
        case '2':
        case '3': 	sscanf(&cmnd[0],"%d",&sbj);
                    PlnPar();
                    break;
        case '\0':
        case '\n':
                    break;
        default:
            fprintf(trns[plr],"Unknown command:%s\n",cmnd);
            fprintf(stderr,"Unknown activity:(%d)%s\n",cmnd[0],cmnd);
        }
    ValidCheck();
}

/*****************************************************************************/
void SetSord()
/*****************************************************************************/
/* Set a standing order */
{
    TRTRN(printf("SetSord()\n"));
    cmnd[strlen(cmnd)]=0;
    if(cmnd[1]=='s' || cmnd[1]=='S') {
        sscanf(&cmnd[2],"%d",&sbj);
        sbj-=100;
        ShipSord(); } 
    else {
        sscanf(&cmnd[1],"%d",&sbj);
        sbj-=100;
        PlanSord();
        }
    return;
}

/*****************************************************************************/
void ShipSord()
/*****************************************************************************/
/* Set a ships standing order */
{
    int x;

    TRTRN(printf("ShipSord()\n"));
    if(sbj<0 || sbj>shiptr) {
        fprintf(trns[plr],"%s: Invalid ship:%d\n",cmnd,sbj+100);
        return;
        }
    if(plr!=fleet[sbj].owner) {
        fprintf(trns[plr],"%s: Not your ship:%d\n",cmnd,sbj+100);
        return;
        }
    for(x=0;x<10 && cmnd[x+5]!=0;x++)
        fleet[sbj].stndord[x]=cmnd[x+5];
    fleet[sbj].stndord[x]=0;
    fprintf(trns[plr],"OS%d%s\n",sbj+100,fleet[sbj].stndord);
    return;
}

/*****************************************************************************/
void PlanSord()
/*****************************************************************************/
/* Set a planets standing order */
{
    int x;

    TRTRN(printf("PlanSord()\n"));
    if(sbj<0 || sbj>NUMPLANETS) {
        fprintf(trns[plr],"%s: Invalid planet:%d\n",cmnd,sbj+100);
        return;
        }
    if(plr!=galaxy[sbj].owner) {
        fprintf(trns[plr],"%s: Not your planet:%d\n",cmnd,sbj+100);
        return;
        }
    for(x=0;x<10 && cmnd[x+4]!=0 && cmnd[x+4]!='\n';x++)
        galaxy[sbj].stndord[x]=cmnd[x+4];
    galaxy[sbj].stndord[x]=0;
    fprintf(trns[plr],"O%d%s\n",sbj+100,galaxy[sbj].stndord);
    return;
}

/*****************************************************************************/
void ClearSord()
/*****************************************************************************/
/* Clear a standing order */
{
    TRTRN(printf("ClearSord()\n"));
    if(cmnd[1]=='s' || cmnd[1]=='S') {
        sscanf(&cmnd[2],"%d",&sbj);
        sbj-=100;
        ClearShipSord();
        }
    else {
        sscanf(&cmnd[1],"%d",&sbj);
        sbj-=100;
        ClearPlanSord();
        }
    return;
}

/*****************************************************************************/
void ClearShipSord()
/*****************************************************************************/
/* Clear a ships standing order */
{
    TRTRN(printf("ClearShipSord()\n"));
    if(sbj<0 || sbj>shiptr) {
        fprintf(trns[plr],"%s: Invalid ship %d\n",cmnd,sbj);
        return;
        }
    if(plr!=fleet[sbj].owner) {
        fprintf(trns[plr],"%s: Not your ship %d\n",cmnd,sbj);
        return;
        }
    fleet[sbj].stndord[0]=0;
    fprintf(trns[plr],"XS%d\n",sbj+100);
    return;
}

/*****************************************************************************/
void ClearPlanSord()
/*****************************************************************************/
/* Clear a planets standing order */
{
    TRTRN(printf("ClearPlanSord()\n"));
    if(sbj<0 || sbj>NUMPLANETS) {
        fprintf(trns[plr],"%s: Invalid planet %d\n",cmnd,sbj);
        return;
        }
    if(plr!=galaxy[sbj].owner) {
        fprintf(trns[plr],"%s: Not your planet %d\n",cmnd,sbj);
        return;
        }
    galaxy[sbj].stndord[0]=0;
    fprintf(trns[plr],"X%d\n",sbj+100);
    return;
}

/*****************************************************************************/
void Broadcast()
/*****************************************************************************/
/* Send a broadcast message to every player */
{
    FILE *fp;
    char str[80];
    int x;

    TRTRN(printf("Broadcast()\n"));
    sprintf(str,"%s%d/motd",path,gm);
    if((fp=fopen(str,"a"))==NULL) {
        fprintf(stderr,"Broadcast:Could not open file %s for appending\n",str);
        return;
        }
    fprintf(fp,"%s:",name[plr]);
    for(x=1;cmnd[x]!='\n';x++) 
        if(!(cmnd[x]=='}' && cmnd[x+1]=='\n'))
            fprintf(fp,"%c",cmnd[x]);
    fprintf(fp,"\n");
    fclose(fp);
    return;
}

/*****************************************************************************/
void Personal(void)
/*****************************************************************************/
/* Send a personal message to a player */
{
    FILE *fp;
    char str[80],victname[10];
    int x,rec;

    TRTRN(printf("Personal()\n"));

    for(x=0;cmnd[x+1]!=' ' && x<10;x++)
        victname[x]=cmnd[x+1];
    for(;x<10;x++)
        victname[x]='\0';
    for(rec= -1,x=0;x<10;x++)
        if(strcasecmp(victname,name[x])==0)
            rec=x;
    if(rec<0)
        rec=UserSelect(victname);
    sprintf(str,"%s%d/spec.%d",path,gm,rec);
    if((fp=fopen(str,"a"))==NULL) {
        fprintf(stderr,"Personal:Could not open file %s for apending\n",str);
        return;
        }
    fprintf(fp,"From %s:",name[plr]);
    for(x=strlen(victname)+1;cmnd[x]!='\n';x++) 
        if(!(cmnd[x]=='&' && cmnd[x+1]=='\n'))
            fprintf(fp,"%c",cmnd[x]);
    fprintf(fp,"\n");
    fclose(fp);
    return;
}

/*****************************************************************************/
void AllMsg(void)
/*****************************************************************************/
/* Send a message to an alliance level */
{
    FILE *fp,*tf;
    char tmpname[80],str[80],victname[10];
    int x,rec;

    TRTRN(printf("AllMsg()\n"));

    /* Get the alliance level from the message */
    for(x=0;cmnd[x+1]!=' ' && x<10;x++)
        victname[x]=cmnd[x+1];
    for(;x<10;x++)
        victname[x]='\0';

    /* Compare alliance level with the list of alliance levels */
    for(rec= -1,x=0;x<10;x++)
        if(strcasecmp(allname[x],victname)==0)
            rec=x;
    /* If the computer can't work out which level it is ask the administrator */
    if(rec<0)
        rec=AllSelect(victname);

    /* Save the message into a temporary file */
    strcpy(tmpname,"/tmp/ZDAMXXXXXX");
    mktemp(tmpname);
    if((tf=fopen(tmpname,"w"))==NULL) {
        fprintf(stderr,"Could not open %s for writing\n",tmpname);
        return;
        }
    for(x=strlen(victname)+1;cmnd[x]!='\n';x++) 
        fprintf(tf,"%c",cmnd[x]);
    fprintf(tf,"\n");
    fclose(tf);

    /* For every player whom you have at that level send message */
    for(x=0;x<NUMPLAYERS+1;x++)
        if(alliance[x][plr]==rec) {
            sprintf(str,"%s%d/spec.%d",path,gm,rec);
            if((fp=fopen(str,"a"))==NULL) {
                fprintf(stderr,"AllMsg:Could not open file %s for apending\n",str);
                return;
                }
            if((tf=fopen(tmpname,"r"))==NULL) {
                fprintf(stderr,"AllMsg:Couldn't open %s for reading\n",tmpname);
                return;
                }
            fprintf(fp,"From %s:",name[plr]);
            for(;fgets(str,80,tf)!=NULL;)
                fputs(str,fp);
            fprintf(fp,"\n");
            fclose(fp);
            fclose(tf);
            }
    return;
}

/*****************************************************************************/
void Scan(void)
/*****************************************************************************/
/* process the scan command */
{
    int num;

    TRTRN(printf("Scan()\n"));
    sscanf(&cmnd[4],"%d",&num);
    if(num<100 || num > 356) {
        OUTTR(fprintf(df,"Planet %d not scanned\n",num));
        fprintf(trns[plr],"Planet out of range:%s\n",cmnd);
        return;
        }
        
    Prnt(SCAN);
    Prnt(num-100);
    OUTTR(fprintf(df,"Scanning planet %d\n",num));
    return;
}

/*****************************************************************************/
void ChangeAlliance(void)
/*****************************************************************************/
/* Change the formal alliance */
{
    char victname[10];
    int x,rec;

    TRTRN(printf("ChangeAlliance()\n"));

    /* Copy char by char from cmnd into victname */
    for(x=0;cmnd[x+1]!='\0' && cmnd[x+1]!='\n' && x<10;x++)
        victname[x]=cmnd[x+1];

    /* Fill rest of name with nulls */
    for(;x<10;x++)
        victname[x]='\0';

    /* See if any playernames match */
    for(rec= -1,x=0;x<10;x++)
        if(strcasecmp(victname,name[x])==0)
            rec=x;

    /* If no match received then ask administrator to look */
    if(rec<0)
        rec=UserSelect(victname);
    if(rec>=0) {
        if(cmnd[0]=='+') {
            OUTTR(fprintf(df,"Changing alliance of player %d upwards",plr));
            Prnt(ALLNCUP);
            }
        else {
            OUTTR(fprintf(df,"Changing alliance of player %d downwards",plr));
            Prnt(ALLNCDN);
            }
        Prnt(rec);
        }
    else {
        OUTTR(fprintf(df,"Alliance with %s not changing\n",victname));
        fprintf(trns[plr],"Alliance not changing:%s\n",cmnd);
        }
    OUTTR(fprintf(df," with respect to %s\n",name[rec]));
    return;
}

/*****************************************************************************/
void ShpPar(void)
/*****************************************************************************/
/* Parse all ship based cmnds        */
{
    TRTRN(printf("ShpPar()\n"));

    switch((cmnd[4])) {
        case 'a':	case 'A':
            ParsAtt();
            break;
        case 'b':	case 'B':
            ParsBuishi();
            break;
        case 'd':	case 'D':
            ParsDeploy();
            break;
        case 'e':	case 'E':
            EngageTrac();
            break;
        case 'g':	case 'G':
            ShipGift();
            break;
        case 'j':	case 'J':
            ParsJump();
            break;
        case 'l':	case 'L':
            ParsLoad();
            break;
        case 'p':	case 'P':
            TransBuyOre();
            break;
        case 'r':	case 'R':
            ParsRetrieve();
            break;
        case 't':	case 'T':
            ParsTend();
            break;
        case 'u':	case 'U':
            ParsUnld();
            break;
        case 'x':	case 'X':
            TransSellOre();
            break;
        case 'z':	case 'Z':
            ParsUnbld();
            break;
        case '=':
            ShipName();
            break;
        default:
            fprintf(stderr,"Unrecognized ship command:%s\n",cmnd);
            fprintf(trns[plr],"Unrecognized ship command:%s\n",cmnd);
            break;
        }
    return;
}

/*****************************************************************************/
void TransGiftPlan(void)
/*****************************************************************************/
/* Gift a planet to another plr */
{
    char victname[10];
    int x,rec;

    TRTRN(printf("TransGiftPlan()\n"));
    if(sbj<100 || sbj>356) {
        OUTTR(fprintf(df,"Not gifting planet %d\n",sbj));
        fprintf(trns[plr],"Planet out of range:%s\n",cmnd);
        return;
        }
    for(x=0;cmnd[x+4]!='\0' && cmnd[x+4]!='\n' && x<10;x++)
        victname[x]=cmnd[x+4];
    for(;x<10;x++)
        victname[x]='\0';
    for(rec= -1,x=0;x<10;x++)
        if(strcasecmp(victname,name[x])==0)
            rec=x;
    if(rec<0)
        rec=UserSelect(victname);
    if(rec<0) {
        OUTTR(fprintf(df,"Not gifting planet %d to %s\n",sbj,victname));
        fprintf(trns[plr],"Not gifting planet %d to %s\n",sbj,victname);
        }
    else {
        Prnt(GIFTPLAN);
        Prnt(sbj-100);
        Prnt(rec);
        }
    OUTTR(fprintf(df,"Gifting planet %d to plr %d:%s\n",sbj,rec,victname));
    return;
}

/*****************************************************************************/
void ShipGift(void)
/*****************************************************************************/
/* Gift a ship to another plr */
{
    char victname[10];
    int x,rec;

    TRTRN(printf("ShipGift\n"));
    if(sbj<100 || sbj>356) {
        OUTTR(fprintf(df,"Not gifting ship %d\n",sbj));
        fprintf(trns[plr],"Ship out of range:%s\n",cmnd);
        return;
        }
    for(x=0;cmnd[x+5]!='\0' && cmnd[x+5]!='\n' && x<10;x++) 
        victname[x]=cmnd[x+5];
    for(;x<10;x++)
        victname[x]='\0';
    for(rec= -1,x=0;x<10;x++)
        if(strcasecmp(victname,name[x])==0)
            rec=x;
    if(rec<0)
        rec=UserSelect(victname);
    if(rec<0) {
        OUTTR(fprintf(df,"Not gifting planet %d to %s\n",sbj,victname));
        fprintf(trns[plr],"Not gifting planet %d to %s\n",sbj,victname);
        }
    else {
        Prnt(GIFTSHIP);
        Prnt(sbj-100);
        Prnt(rec);
        }
    OUTTR(fprintf(df,"Gifting ship %d to plr %d:%s\n",sbj,rec,victname));
    return;
}

/*****************************************************************************/
int AllSelect(char *victname)
/*****************************************************************************/
/* Get the administrator to get appropriate name for alliance message */
{
    int i;
    static char lastname[10]="NEUTRAL";
    static int lastplr=NEUTPLR;

    TRTRN(printf("AllSelect(victname:%s)\n",victname));

    if(strcasecmp(victname,lastname)==0) {
        TRTRN(printf("AllSelect:Using previous name:%d\n",lastplr));
        return(lastplr);
        }
    for(i=0;i<4;i++)
        printf("%-2d: %10s\n",i,allname[i]);
    printf("\nStatus entered was %s, which status is that:",victname);
    scanf("%d",&i);
    if(i<0 || i>3)
        return(-1);
    strcpy(lastname,victname);
    lastplr=i;
    return(i);
}

/*****************************************************************************/
int UserSelect(char *victname)
/*****************************************************************************/
/* Get the administrator to get appropriate name for gifting or alliance change */
{
    int i;
    static char lastname[10]="NEUTRAL";
    static int lastplr=NEUTPLR;

    TRTRN(printf("UserSelect(victname:%s)\n",victname));

    if(strcasecmp(victname,lastname)==0) {
        TRTRN(printf("UserSelect:Using previous name:%d\n",lastplr));
        return(lastplr);
        }
    for(i=0;i<10;i++)
        printf("Player %-2d: %10s\n",i,name[i]);
    printf("\nName entered was %s, which player is that:",victname);
    scanf("%d",&i);
    if(i<0 || i>9)
        return(-1);
    strcpy(lastname,victname);
    lastplr=i;
    return(i);
}

/*****************************************************************************/
void TransSellOre(void)
/*****************************************************************************/
/* Sell ore on Earth Market */
{
    int type,amount,count;

    TRTRN(printf("TransSellOre\n"));

    if(sbj<100 || sbj>356) {
        OUTTR(fprintf(df,"Not selling ore from ship %d\n",sbj));
        fprintf(trns[plr],"Ship out of range:%s\n",cmnd);
        return;
        }
    /* Sell all ore */
    if(cmnd[5]=='\n') {
        Prnt(SELLALL);	
        Prnt(sbj-100);
        return;
        }
    sscanf(&cmnd[5],"%d",&amount);
    for(count=5;isdigit(cmnd[count]);count++);
    count++;
    sscanf(&cmnd[count],"%d",&type);
    if(type<0 || type>9) {
        OUTTR(fprintf(df,"Not selling ore type %d from ship %d\n",type,sbj));
        fprintf(trns[plr],"Ore type out of range:%s\n",cmnd);
        return;
        }
    Prnt(SELLORE);
    Prnt(sbj-100);
    Prnt(amount);
    Prnt(type);
    OUTTR(fprintf(df,"Ship %d sell %d of ore type %d on Earth\n",sbj,amount,type));
    return;
}

/*****************************************************************************/
void TransBuyOre(void)
/*****************************************************************************/
/* Buy ore on the Earth Market */
{
    int type,amount,count;

    TRTRN(printf("TransBuyOre\n"));

    sscanf(&cmnd[5],"%d",&amount);
    if(amount<0) {
        fprintf(trns[plr],"%s:Negative ore amounts (%d) not allowed\n",cmnd,amount);
        return;
        }
    for(count=5;isdigit(cmnd[count]);count++);
    count++;
    sscanf(&cmnd[count],"%d",&type);
    if(type<0 || type>9) {
        fprintf(trns[plr],"%s:Illegal ore type %d\n",cmnd,type);
        return;
        }
    Prnt(BUYORE);
    Prnt(sbj-100);
    Prnt(amount);
    Prnt(type);
    OUTTR(fprintf(df,"Ship %d buy %d of ore type %d on Earth\n",sbj,amount,type));
    return;
}

/*****************************************************************************/
void PlanetName(void)
/*****************************************************************************/
/* Name a planet */
{
    char name[NAMESIZ];
    int x;

    TRTRN(printf("PlanetName\n"));
    Prnt(NAMEPLAN);
    Prnt(sbj-100);
    for(x=0;cmnd[x+4]!='\0' && cmnd[x+4]!='\n' && x<NAMESIZ-1;x++) {
        name[x]=cmnd[x+4];
        Prnt(name[x]);
        }
    Prnt(0);
    name[x]=0;
    OUTTR(fprintf(df,"Naming planet %d : %s\n",sbj,name));
    return;
}

/*****************************************************************************/
void ShipName(void)
/*****************************************************************************/
/* Name a ship */
{
    char name[NAMESIZ];
    int x;

    TRTRN(printf("ShipName\n"));
    Prnt(NAMESHIP);
    Prnt(sbj-100);
    for(x=0;cmnd[x+5]!='\0' && cmnd[x+5]!='\n' && x<NAMESIZ-1;x++) {
        name[x]=cmnd[x+5];
        Prnt(name[x]);
        }
    Prnt(0);
    name[x]=0;
    OUTTR(fprintf(df,"Naming ship %d : %s\n",sbj,name));
    return;
}

/*****************************************************************************/
void EngageTrac(void)
/*****************************************************************************/
/* Engage the tractor beams on another ship */
{
    int vict;

    TRTRN(printf("EngageTrac\n"));
    sscanf(&cmnd[6],"%d",&vict);
    if(vict<100) {
        fprintf(trns[plr],"%s:Invalid ship number:%d\n",cmnd,vict);
        return;
        }
    Prnt(ENGTRAC);
    Prnt(sbj-100);
    Prnt(vict-100);
    OUTTR(fprintf(df,"Ship %d engaging tractor beams on ship %d\n",sbj,vict));
    return;
}

/*****************************************************************************/
void PlnPar(void)
/*****************************************************************************/
/* Parse all the planet based cmnds  */
{
    TRTRN(printf("PlnPar()\n"));

    switch((cmnd[3])) {
        case 'g':	case 'G':
            TransGiftPlan();
            break;
        case '=':
            PlanetName();
            break;
        case 'a':	case 'A':
            if(isdigit(cmnd[4])) {
                int shts;

                sscanf(&cmnd[4],"%d",&shts);	
                if(shts<0) {
                    fprintf(trns[plr],"%s:Illegal number of shots\n",cmnd);
                    return;
                    }
                PlanAtt(shts);
            }
            else
                PlanAtt(-1);
            break;
        case 'b':	case 'B':
            PlanBuild();
            break;
        case 'x':	case 'X':
            ParseTrans();
            break;
        case 'd':	case 'D':
            PlanDeploy();
            break;
        default:
            fprintf(trns[plr],"Unknown command:%s\n",cmnd);
            fprintf(stderr,"Unrecognized planetary command:%s\n",cmnd);
        }
    return;
}

/*****************************************************************************/
void PlanDeploy(void)
/*****************************************************************************/
/* Deploy space mines from the planetary stockpile */
{
    int amount;

    TRTRN(printf("PlanDeploy\n"));

    sscanf(&cmnd[4],"%d",&amount);
    if(amount<0) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amount);
        return;
        }
    Prnt(DEPLPLN);
    Prnt(sbj-100);
    Prnt(amount);
    OUTTR(fprintf(df,"Deploying %d spacemines from planet %d\n",amount,sbj));
    return;
}

/*****************************************************************************/
void ParsDeploy(void)
/*****************************************************************************/
/* Deploy space mines from a ship */
{
    int amount;

    TRTRN(printf("ParsDeploy\n"));

    sscanf(&cmnd[5],"%d",&amount);
    if(amount<0) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amount);
        return;
        }
    Prnt(DEPLSHP);
    Prnt(sbj-100);
    Prnt(amount);
    OUTTR(fprintf(df,"Deploying %d spacemines from ship %d\n",amount,sbj));
    return;
}

/*****************************************************************************/
void ParsRetrieve(void)
/*****************************************************************************/
/* Retrieve or Undeploy space mines */
{
    int amount;

    TRTRN(printf("ParsRetrieve\n"));
    sscanf(&cmnd[5],"%d",&amount);
    if(amount<0) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amount);
        return;
        }
    Prnt(UNDEPL);
    Prnt(sbj-100);
    Prnt(amount);
    OUTTR(fprintf(df,"Retrieving %d space mines into ship %d\n",amount,sbj));
    return;
}

/*****************************************************************************/
void PlanAtt(int shots)
/*****************************************************************************/
/* Get the pdus on the planet to attack the ship or spacemines */
{
    int vict,pos;

    TRTRN(printf("PlanAtt(shots:%d)\n",shots));
    if(shots<0)
        pos=5;
    else
        pos=5+ilog(shots);
    if(cmnd[pos]=='m' || cmnd[pos]=='M') {
        Prnt(PLANSPM);
        Prnt(sbj-100);
        OUTTR(fprintf(df,"Planet %d attack spacemines",sbj));
        }
    else {
        sscanf(&cmnd[pos],"%d",&vict);
        if(vict<100) {
            fprintf(trns[plr],"%s:Invalid victim:%d\n",cmnd,vict);
            return;
            }
        Prnt(PLANATT);
        Prnt(sbj-100);
        Prnt(vict-100);
        OUTTR(fprintf(df,"Planet %d attack ship %d",sbj,vict));
        }
    if(shots>=0)
        OUTTR(fprintf(df," with %d PDUs\n",shots));
    else
        OUTTR(fprintf(df,"\n"));
    Prnt(shots);
    return;
}

/*****************************************************************************/
void PlanBuild(void)
/*****************************************************************************/
/* Parse the planetary building */
{
    int count,amount,type;

    TRTRN(printf("PlanBuild\n"));

    if(isdigit(cmnd[4]))
        sscanf(&cmnd[4],"%d",&amount);
    else {
        fprintf(trns[plr],"%s:Amount should be a number not `%c'\n",cmnd,cmnd[4]);
        return;
        }
    if(amount<0) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amount);
        return;
        }
    for(count=4;isdigit(cmnd[count]);count++);
    sscanf(&cmnd[count+1],"%d",&type);
    switch((cmnd[count])) {
        case 'd':	case 'D':
            ParpDef(amount);
            break;
        case 'i':	case 'I':
            ParpInd(amount);
            break;
        case 'm':	case 'M':
        case 'r':	case 'R':		/* Makes it more orthogonal */
            if(type<0 || type>9) {
                fprintf(trns[plr],"%s:Invalid type:%d\n",cmnd,type);
                return;
                }
            ParpMin(amount,type);
            break;
        case 'h':	case 'H':
            ParpHyp(amount);
            break;
        case 's':	case 'S':
            if(type<0 || type>9) {
                fprintf(trns[plr],"%s:Invalid type:%d\n",cmnd,type);
                return;
                }
            ParpSpcmin(amount,type);
            break;
        default:
            fprintf(trns[plr],"Unknown build type:%s\n",cmnd);
            fprintf(stderr,"Unknown planet build type:%c\n",cmnd[count]);
        }
    return;
}

/*****************************************************************************/
void ParpSpcmin(int amount,int type)
/*****************************************************************************/
/* Build spacemines on planet */
{
    TRTRN(printf("ParpSpcmin(amount:%d,type:%d)\n",amount,type));
    Prnt(BUILDSPM);
    Prnt(sbj-100);
    Prnt(amount);
    Prnt(type);
    OUTTR(fprintf(df,"Building %d spacemines on planet %d with type %d ore\n",amount,sbj,type));
    return;
}

/*****************************************************************************/
void ParsUnld(void)
/*****************************************************************************/
/* Parse all unload cmnds    */
{
    int     count;
    int     amount;
    int     type;

    TRTRN(printf("ParsUnld\n"));

    /* Unload all   */
    if(cmnd[5]=='\n') {
        Prnt(UNLODALL);
        OUTTR(fprintf(df,"Unload all from ship:%d\n",sbj));
        Prnt(sbj-100);
        return;
        }

    /* Unload specific      */
    sscanf(&cmnd[5],"%d",&amount);
    if(amount<0) {
        fprintf(trns[plr],"%s:Illegal amount:%d\n",cmnd,amount);
        return;
        }
    for(count=5;isdigit(cmnd[count]);count++);
    switch((cmnd[count++])) {
        case 'i': case 'I':
            Prnt(UNLODIND);
            OUTTR(fprintf(df,"Unload %d industry from ship %d\n",amount,sbj));
            break;
        case 'd': case 'D':
            Prnt(UNLODDEF);
            OUTTR(fprintf(df,"Unload %d defense from ship %d\n",amount,sbj));
            break;
        case 'm': case 'M':
            sscanf(&cmnd[count++],"%d",&type);
            OUTTR(fprintf(df,"Unload %d mines of type %d from ship %d\n",amount,type,sbj));
            Prnt(UNLODMIN);
            Prnt(type);
            break;
        case 'r': case 'R':
            sscanf(&cmnd[count++],"%d",&type);
            if(type<0 || type>9) {
                fprintf(trns[plr],"%s:Illegal type:%d\n",cmnd,type);
                return;
                }
            OUTTR(fprintf(df,"Unload %d ore of type %d from ship %d\n",amount,type,sbj));
            Prnt(UNLODORE);
            Prnt(type);
            break;
        case 's': case 'S':
            OUTTR(fprintf(df,"Unload %d space mines from ship %d\n",amount,sbj));
            Prnt(UNLODSPM);
            break;
        default:
            fprintf(stderr,"Unload type unkown %s\n",cmnd);
            fprintf(trns[plr],"Unknown unload type:%s\n",cmnd);
        }
    Prnt(sbj-100);
    Prnt(amount);
    return;
}

/*****************************************************************************/
void ParsLoad(void)
/*****************************************************************************/
/* Parse all load cmnds    */
{
    int     count,pricnt;
    int     amount;
    int     type;
    char	pri[13];

    TRTRN(printf("ParsLoad\n"));

    /* Load all   */
    if(cmnd[5]=='\n') {
        OUTTR(fprintf(df,"Load all into ship %d\n",sbj));
        Prnt(LOADALL);
        Prnt(sbj-100);
        return;
        }
    /* Load all with prioity list */
    if(cmnd[5]=='(') {
        OUTTR(fprintf(df,"Load all into ship %d with priority list:",sbj));
        for(pricnt=0,count=6;count<18 && cmnd[count]!=')';count++) {
            if(cmnd[count]=='p' || cmnd[count]=='P' || cmnd[count]=='d' || cmnd[count]=='D') {
                pri[pricnt++]=DEFENSE;
                OUTTR(fprintf(df,"D"));
                }
            else if(cmnd[count]=='s' || cmnd[count]=='S') {
                pri[pricnt++]=SPACEMINE;
                OUTTR(fprintf(df,"S"));
                }
            else if(isdigit(cmnd[count])) {		/* Normal Ore type */
                pri[pricnt++]=(char)cmnd[count]-'0';
                OUTTR(fprintf(df,"%d",pri[count-6]));
                }
            else
                OUTTR(fprintf(df,">%c<",pri[count-6]));
            }
        OUTTR(fprintf(df,"\n"));
        pri[pricnt]=-1;
        Prnt(LOADPRI);
        Prnt(sbj-100);
        for(count=0;count<13;count++)
            Prnt(pri[count]);
        return;
        }

    /* Load specific      */
    sscanf(&cmnd[5],"%d",&amount);
    if(amount<0) {
        fprintf(trns[plr],"%s:Illegal amount:%d\n",cmnd,amount);
        return;
        }
    for(count=5;isdigit(cmnd[count]);count++);
    switch((cmnd[count++])) {
        case 'i':	case 'I':
            Prnt(LOADIND);
            OUTTR(fprintf(df,"Load %d industry into ship %d\n",amount,sbj));
            break;
        case 'd':	case 'D':
            Prnt(LOADDEF);
            OUTTR(fprintf(df,"Load %d defence into ship %d\n",amount,sbj));
            break;
        case 'm':	case 'M':
            sscanf(&cmnd[count++],"%d",&type);
            if(type<0 || type>9) {
                fprintf(trns[plr],"%s:Invalid type:%d\n",cmnd,type);
                return;
                }
            OUTTR(fprintf(df,"Load %d mines of type %d into ship %d\n",amount,type,sbj));
            Prnt(LOADMIN);
            Prnt(type);
            break;
        case 'r':	case 'R':
            sscanf(&cmnd[count++],"%d",&type);
            if(type<0 || type>9) {
                fprintf(trns[plr],"%s:Invalid type:%d\n",cmnd,type);
                return;
                }
            OUTTR(fprintf(df,"Load %d ore of type %d into ship %d\n",amount,type,sbj));
            Prnt(LOADORE);
            Prnt(type);
            break;
        case 's':	case 'S':
            OUTTR(fprintf(df,"Load %d spacemines into ship %d\n",amount,sbj));
            Prnt(LOADSPM);
            break;
        default:
            fprintf(stderr,"Load type unkown %s\n",cmnd);
            fprintf(trns[plr],"Unknown load type:%s\n",cmnd);
        }
    Prnt(sbj-100);
    Prnt(amount);
    return;
}

/*****************************************************************************/
void ParsBuishi(void)
/*****************************************************************************/
/* Parse all cmnds involving the building of units onto ships        */
{
    char cont=0;
    int amount;
    int count,err=0;
    int bid=0;

    TRTRN(printf("ParsBuishi\n"));

    sscanf(&cmnd[5],"%d",&amount);
    if(amount<1) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amount);
        return;
        }
    for(count=5;isdigit(cmnd[count]);count++);
    switch((cmnd[count++])) {
        case 'c':	case 'C':
            if(!isspace(cmnd[count])) {
                cont='c';
                sscanf(&cmnd[count],"%d",&bid);
                if(bid<1) {
                    fprintf(trns[plr],"Invalid bid:%d:%s\n",bid,cmnd);
                    return;
                    }
                Prnt(CONTCARG);
                OUTTR(fprintf(df,"Contracting %d cargo units on ship %d for %d each\n",amount,sbj,bid));
                TRTRN(printf("Contract cargo\n"));
                break;
            }
            Prnt(BUILDCAR);
            OUTTR(fprintf(df,"Building %d cargo units on ship %d\n",amount,sbj));
            TRTRN(printf("Build cargo\n"));
            break;
        case 'f':	case 'F':
            if(!isspace(cmnd[count])) {
                cont='f';
                sscanf(&cmnd[count],"%d",&bid);
                if(bid<1) {
                    fprintf(trns[plr],"%s:Invalid bid:%d\n",cmnd,bid);
                    return;
                    }
                Prnt(CONTFGHT);
                OUTTR(fprintf(df,"Contracting %d fighter units on ship %d for %d each\n",amount,sbj,bid));
                TRTRN(printf("Contract fighter\n"));
                break;
            }
            Prnt(BUILDFGT);
            OUTTR(fprintf(df,"Building %d fighter units on ship %d\n",amount,sbj));
            TRTRN(printf("Build fighter\n"));
            break;
        case 't':	case 'T':
            if(!isspace(cmnd[count])) {
                cont='t';
                sscanf(&cmnd[count],"%d",&bid);
                if(bid<1) {
                    fprintf(trns[plr],"%s:Invalid bid:%d\n",cmnd,bid);
                    return;
                    }
                Prnt(CONTTRAC);
                OUTTR(fprintf(df,"Contracting %d tractor units on ship %d for %d each.\n",amount,sbj,bid));
                TRTRN(printf("Contract tractor\n"));
                break;
            }
            Prnt(BUILDTRC);
            OUTTR(fprintf(df,"Building %d tractor units on ship %d\n",amount,sbj));
            TRTRN(printf("Build tractor\n"));
            break;
        case 's':	case 'S':
            if(!isspace(cmnd[count])) {
                cont='s';
                sscanf(&cmnd[count],"%d",&bid);
                if(bid<1) {
                    fprintf(trns[plr],"%s:Invalid bid:%d\n",cmnd,bid);
                    return;
                    }
                Prnt(CONTSHLD);
                OUTTR(fprintf(df,"Contracting %d shield units on ship %d for %d each\n",sbj,amount,bid));
                TRTRN(printf("Contract shield\n"));
                break;
            }
            Prnt(BUILDSLD);
            OUTTR(fprintf(df,"Building %d shield units on ship %d\n",amount,sbj));
            TRTRN(printf("Build shield\n"));
            break;
        default:
            fprintf(stderr,"Unknown ship build type %s\n",cmnd);
            fprintf(trns[plr],"Unknown build type:%s\n",cmnd);
            err=1;
        }
    if(!err) {
        Prnt(sbj-100);
        Prnt(amount);
        if(bid!=0)
            Prnt(bid);
        }
    if(cont) 
        /* Ship type amount bid plr */
        fprintf(bidfp,"%d %c %d %d %d\n",sbj-100,cont,amount,bid,plr);
    return;
}

/*****************************************************************************/
void ParsJump(void)
/*****************************************************************************/
/* Parse all movement statements        */
{
    int len=strlen(cmnd);

    TRTRN(printf("ParsJump\n"));

    while(cmnd[len-2]==' '||cmnd[len-2]=='\t')
        len--;
    switch(len) {
        case 9:
            Prnt(JUMP1);
            OUTTR(fprintf(df,"Ship %d jumping length 1 to ",sbj));
            Prnt(sbj-100);
            Dest(4);
            break;
        case 13:
            Prnt(JUMP2);
            OUTTR(fprintf(df,"Ship %d jumping length 2 to ",sbj));
            Prnt(sbj-100);
            Dest(4); Dest(8);
            break;
        case 17:
            Prnt(JUMP3);
            Prnt(sbj-100);
            OUTTR(fprintf(df,"Ship %d jumping length 3 to ",sbj));
            Dest(4); Dest(8); Dest(12);
            break;
        case 21:
            Prnt(JUMP4);
            OUTTR(fprintf(df,"Ship %d jumping length 4 to ",sbj));
            Prnt(sbj-100);
            Dest(4); Dest(8); Dest(12); Dest(16);
            break;
        case 25:
            Prnt(JUMP5);
            OUTTR(fprintf(df,"Ship %d jumping lenght 5 to ",sbj));
            Prnt(sbj-100);
            Dest(4); Dest(8); Dest(12); Dest(16); Dest(20);
            break;
        default:
            fprintf(stderr,"Jump statement is length %d\n",len);
            fprintf(trns[plr],"Malformed jump statement:%s\n",cmnd);
        }
        OUTTR(fprintf(df,"\n"));
    return;
}

/*****************************************************************************/
void Dest(int pos)
/*****************************************************************************/
/* Work out the destination of a jump statement */
{
    int     npos;
    TRTRN(printf("Dest(pos: %d)\n",pos));
    sscanf(&cmnd[pos+1],"%d",&npos);
    if(npos<100 || npos>100+NUMPLANETS) {
        fprintf(trns[plr],"%s:Invalid destination:%d\n",cmnd,npos);
        return;
        }
    OUTTR(fprintf(df,"%d ",npos));
    Prnt(npos-100);
    return;
}

/*****************************************************************************/
void ParpHyp(int amount)
/*****************************************************************************/
/* Parse the building of hyperdrive units */
{
    /* 123b23h10/20/30/40 */
    /*        ^F ^C ^T ^S */
    int fgt,crg,shld,trac,count;

    TRTRN(printf("ParpHyp(amount: %d)\n",amount));
    for(count=4;cmnd[count]!='h' && cmnd[count]!='H';count++);
    if(!isdigit(cmnd[++count])) {
        fprintf(trns[plr],"Malformed hyperdrive build:%s\n",cmnd);
        return;
        }
    sscanf(&cmnd[count],"%d",&fgt);
    if(fgt<0) {
        fprintf(trns[plr],"%s:Illegal number of fighter units:%d\n",cmnd,fgt);
        return;
        }
    TRTRN(printf("\tMaking Hyperdrive with %d Fighter units\n",fgt));
    for(;isdigit(cmnd[count]);count++);
    if(cmnd[count]!='/') {
        fprintf(stderr,"%c Need a / to seperate Fight & Carg\n",cmnd[count]);
        fprintf(trns[plr],"Malformed hyperdrive build:%s\n",cmnd);
        return;
        }
    sscanf(&cmnd[++count],"%d",&crg);
    if(crg<0) {
        fprintf(trns[plr],"%s:Illegal number of cargo units:%d\n",cmnd,crg);
        return;
        }
    TRTRN(printf("\tMaking Hyperdrive with %d Cargo units\n",crg));
    for(;isdigit(cmnd[count]);count++);
    if(cmnd[count]!='/') {
        fprintf(stderr,"%c Need a / to seperate Carg & Trac\n",cmnd[count]);
        fprintf(trns[plr],"Malformed hyperdrive build:%s\n",cmnd);
        return;
        }
    sscanf(&cmnd[++count],"%d",&trac);
    if(trac<0) {
        fprintf(trns[plr],"%s:Illegal number of tractor units:%d\n",cmnd,trac);
        return;
        }
    TRTRN(printf("\tMaking Hyperdrive with %d Tractor units\n",trac));
    for(;isdigit(cmnd[count]);count++);
    if(cmnd[count]!='/') {
        fprintf(trns[plr],"Malformed hyperdrive build:%s\n",cmnd);
        fprintf(stderr,"%c Need a / to seperate Trac & Shld\n",cmnd[count]);
        return;
        }
    sscanf(&cmnd[++count],"%d",&shld);
    if(shld<0) {
        fprintf(trns[plr],"%s:Illegal number of shield units:%d\n",cmnd,shld);
        return;
        }
    TRTRN(printf("\tMaking Hyperdrive with %d Shield units\n",shld));
    OUTTR(fprintf(df,"Building %d hyperdrive units on planet %d with %d fighter, ",amount,sbj,fgt));
    OUTTR(fprintf(df,"%d cargo, %d tractor and %d shield units\n",crg,trac,shld));
    Prnt(BUILDHYP);
    Prnt(sbj-100);
    Prnt(amount);
    Prnt(fgt);
    Prnt(crg);
    Prnt(trac);
    Prnt(shld);
    return;
}

/*****************************************************************************/
void ParpDef(int amount)
/*****************************************************************************/
/* Parse the building of defence units  */
{
    TRTRN(printf("ParpDef(amount: %d)\n",amount));
    OUTTR(fprintf(df,"Build %d PDU on planet %d\n",amount,sbj));
    Prnt(BUILDDEF);
    Prnt(sbj-100);
    Prnt(amount);
    return;
}

/*****************************************************************************/
void ParpInd(int amount)
/*****************************************************************************/
/* Parse the building of industry       */
{
    TRTRN(printf("ParpInd(amount: %d)\n",amount));
    OUTTR(fprintf(df,"Build  %d industry on planet %d\n",amount,sbj));
    Prnt(BUILDIND);
    Prnt(sbj-100);
    Prnt(amount);
}

/*****************************************************************************/
void ParpMin(int amount,int type)
/*****************************************************************************/
/* Parse the building of mines  */
{
    TRTRN(printf("ParpMin(amount: %d, type %d)\n",amount,type));
    OUTTR(fprintf(df,"Build %d mines of type %d on planet %d\n",amount,type,sbj));
    if(type<0 || type>9) {
        fprintf(trns[plr],"%s:Invlid mine type %d\n",cmnd,type);
        return;
        }
    Prnt(BUILDMIN);
    Prnt(sbj-100);
    Prnt(type);
    Prnt(amount);
    return;
}

/*****************************************************************************/
void ParsAtt(void)
/*****************************************************************************/
/* Parse the attacks    */
{
    int shots,pos=5;

    TRTRN(printf("ParsAtt\n"));
    if(isdigit(cmnd[5])) {
        sscanf(&cmnd[5],"%d",&shots);
        if(shots<0) {
            fprintf(trns[plr],"%s:Invalid number of shots:%d\n",cmnd,shots);
            return;
            }
        pos=5+ilog(shots);
        }
    else 
        shots= -1;
    switch((cmnd[pos])) {
        case 's':	case 'S':
            if(cmnd[pos+1]=='m' || cmnd[pos+1]=='M')
                AttSpcmin(shots);
            else
                AttShip(shots);
            break;
        case 'i':	case 'I':
            AttInd(shots);
            break;
        case 'd':	case 'D':
            AttDef(shots);
            break;
        case 'm':	case 'M':
            AttMin(shots);
            break;
        case 'r':	case 'R':
            AttOre(shots);
            break;
        default:
            fprintf(stderr,"Inavalid attack cmnd: %c\n",cmnd[5]);
            fprintf(trns[plr],"Invalid attack:%s\n",cmnd);
        }
    return;
}

/*****************************************************************************/
void AttSpcmin(int shots)
/*****************************************************************************/
/* Ship attack space mines */
{
    TRTRN(printf("AttSpcmin\n"));
    OUTTR(fprintf(df,"Ship %d attacking space mines",sbj));
    if(shots<0) {
        OUTTR(fprintf(df,"\n"));
        }
    else {
        OUTTR(fprintf(df," with %d fighter units\n",shots));
        }
    Prnt(ATTKSPM);
    Prnt(sbj-100);
    Prnt(shots);
    return;
}

/*****************************************************************************/
void AttShip(int shots)
/*****************************************************************************/
/* Attack a ship        */
{
    int victim;

    TRTRN(printf("AttShip(shots=%d)\n",shots));
    sscanf(&cmnd[6+(shots<0?0:ilog(shots))],"%d",&victim);
    if(victim<100 || victim>100+NUMSHIPS) {
        fprintf(trns[plr],"%s:Invlid victim:%d\n",cmnd,victim);
        return;
        }
    OUTTR(fprintf(df,"Ship %d attacked ship %d",sbj,victim));
    if(shots<0) {
        OUTTR(fprintf(df,"\n"));
        }
    else {
        OUTTR(fprintf(df," with %d fighter units\n",shots));
        }
    Prnt(ATTKSHP);
    Prnt(sbj-100);
    Prnt(victim-100);
    Prnt(shots);
    return;
}

/*****************************************************************************/
void AttInd(int shots)
/*****************************************************************************/
/* Attack industry      */
{
    TRTRN(printf("AttInd(shots=%d)\n",shots));

    OUTTR(fprintf(df,"Ship %d attacking industry",sbj));
    if(shots<0) {
        OUTTR(fprintf(df,"\n"));
        }
    else {
        OUTTR(fprintf(df," with %d fighter units\n",shots));
        }
    Prnt(ATTKIND);
    Prnt(sbj-100);
    Prnt(shots);
    return;
}

/*****************************************************************************/
void AttDef(int shots)
/*****************************************************************************/
/* Attack PDUs on a planet      */
{
    TRTRN(printf("AttDef(shots=%d)\n",shots));
    Prnt(ATTKDEF);
    OUTTR(fprintf(df,"Ship %d attacking defence",sbj));
    if(shots<0) {
        OUTTR(fprintf(df,"\n"));
        }
    else {
        OUTTR(fprintf(df," with %d fighter units\n",shots));
        }
    Prnt(sbj-100);
    Prnt(shots);
    return;
}

/*****************************************************************************/
void AttOre(int shots)
/*****************************************************************************/
/* Attack ore on a planet     */
{
    int type,pos;

    TRTRN(printf("AttOre(shots=%d)\n",shots));
    if(shots<0)
        pos=6;
    else
        pos=6+ilog(shots);
    sscanf(&cmnd[pos],"%d",&type);
    TRTRN(printf("AttOre:type:%d\n",type));
    if(type<0 || type>9) {
        fprintf(trns[plr],"Invalid ore type:%s\n",cmnd);
        OUTTR(fprintf(df,"Invalid ore type:%d\n",type));
        return;
        }
    Prnt(ATTKORE);
    Prnt(type);
    OUTTR(fprintf(df,"Ship %d is attacking ore of type %d",sbj,type));
    if(shots<0) {
        OUTTR(fprintf(df,"\n"));
        }
    else {
        OUTTR(fprintf(df," with %d fighter units\n",shots));
        }
    Prnt(sbj-100);
    Prnt(shots);
    return;
}

/*****************************************************************************/
void AttMin(int shots)
/*****************************************************************************/
/* Attack mines on a planet     */
{
    int type,pos;

    TRTRN(printf("AttMin(shots=%d)\n",shots));
    if(shots<0)
        pos=6;
    else
        pos=6+ilog(shots);
    sscanf(&cmnd[pos],"%d",&type);
    TRTRN(printf("AttMin:type:%d\n",type));
    if(type<0 || type>9) {
        fprintf(trns[plr],"Invalid ore type:%s\n",cmnd);
        OUTTR(fprintf(df,"Invalid ore type:%d\n",type));
        return;
        }
    Prnt(ATTKMIN);
    Prnt(type);
    OUTTR(fprintf(df,"Ship %d is attacking mines of type %d",sbj,type));
    if(shots<0) {
        OUTTR(fprintf(df,"\n"));
        }
    else {
        OUTTR(fprintf(df," with %d fighter units\n",shots));
        }
    Prnt(sbj-100);
    Prnt(shots);
    return;
}

/*****************************************************************************/
void CmdDump(void)
/*****************************************************************************/
/* Dump  all the contents of cmdarr to disk */
{
    FILE *cf;
    int death=0;
    int c1,c2;

    TRTRN(printf("CmdDump\n"));

    strcpy(temp,"/tmp/ZDTXXXXXX");
    mktemp(temp);
    if((cf=fopen(temp,"w"))==NULL) {
        fprintf(stderr,"Couldn't open %s for writing\n",temp);
        exit(0);
        }

    for(c1=1;c1<NUMPLAYERS+1;c1++) {
        TRTRN(printf("Player :%d\n",c1));
        fprintf(cf,"%d ",c1);
        for(death=0,c2=0;death==0;c2++) {
            if(cmdarr[c1][c2]==BLANK) {
                TRTRN(printf("Endplayer recieved\n"));
                death=1;
                fprintf(cf,"%d %d\n",NOOPERAT,ENDCOMMAND);
                continue;
                }
            else {
                if(cmdarr[c1][c2]==ENDCOMMAND) {
                    fprintf(cf,"%d\n%d ",ENDCOMMAND,c1);
                    continue;
                    }

                TRTRN2(printf("saving: c1:%4d c2:%4d %4d\n",c1,c2,cmdarr[c1][c2]));
                fprintf(cf,"%d ",cmdarr[c1][c2]);
                }
            }
        }
    fclose(cf);
    return;
}

/*****************************************************************************/
void OpenSord(const char *mode)
/*****************************************************************************/
/* Open standing order output files */
{
    char str[80];
    int x;

    TRTRN(printf("OpenSord(mode:%s)\n",mode));
    for(x=1;x<NUMPLAYERS+1;x++) {
        sprintf(str,"%s%d/sord.%d",path,gm,x);
        if((sord[x]=fopen(str,mode))==NULL) {
            fprintf(stderr,"Could not open file %s with mode %s\n",str,mode);
            exit(-1);
            }
        }
}

/*****************************************************************************/
void CloseSord(void)
/*****************************************************************************/
/* Close standing order output files */
{
    int x;

    TRTRN(printf("CloseSord()\n"));
    for(x=1;x<NUMPLAYERS+1;x++) {
        fclose(sord[x]);
        }
}

/*****************************************************************************/
void OpenBid(void)
/*****************************************************************************/
/* Open the bid file */
{
    TRTRN(printf("OpenBid()\n"));
    strcpy(temp2,"/tmp/ZDBXXXXXX");
    mktemp(temp2);
    if((bidfp=fopen(temp2,"w"))==NULL) {
        fprintf(stderr,"Couldn't open %s for writing\n",temp2);
        exit(0);
        }
}

/*****************************************************************************/
void CloseBid(void)
/*****************************************************************************/
{
    TRTRN(printf("CloseBid()\n"));
    fclose(bidfp);
}

/*****************************************************************************/
void ValidCheck(void)
/*****************************************************************************/
{
    int x,y;

    TRTRN(printf("ValidCheck()\n"));
    for(x=0;x<NUMPLANETS;x++) {
        if(galaxy[x].owner>NUMPLAYERS)
            printf("Owner overload on planet %d:%s",x,cmnd);
        for(y=0;y<10;y++) 
            if(galaxy[x].ore[y]<0 || galaxy[x].ore[y]>1000)
                printf("Ore %d overload on planet %d:%s\n",x,y,cmnd);
        }
}

/*****************************************************************************/
void ParsTend(void)
/*****************************************************************************/
/* Parse all tend cmnds */
{
    int vict;
    TRTRN(printf("ParsTend()\n"));

    switch(cmnd[5]) {
        case 's': case 'S':
            sscanf(&cmnd[6],"%d",&vict);
            if(vict<100 || vict>100+NUMSHIPS) {
                fprintf(trns[plr],"Illegal ship number:%d:%s",vict,cmnd);
                return;
                }
            OUTTR(fprintf(df,"Tend all from ship %d to ship %d\n",sbj,vict));
            Prnt(TENDALL);
            Prnt(sbj-100);
            Prnt(vict-100);
            break;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
            TendState();	
            break;
        default:
            fprintf(trns[plr],"Invalid tend command:%s",cmnd);
            break;
        }
    return;
}

/****************************************************************************/
void TendState(void)
/****************************************************************************/
/* Parse tending amount */
{
    int amnt=0,posn=5,vict,type;

    TRTRN(printf("TendState()\n"));

    while(isdigit(cmnd[posn]))
        amnt=amnt*10+cmnd[posn++]-'0';
    if(amnt<0) {
        fprintf(trns[plr],"%s:Illegal amount:%d\n",cmnd,amnt);
        return;
        }
    if(cmnd[posn]!='s' && cmnd[posn]!='S') {
        fprintf(trns[plr],"Illegal ship specification:%s",cmnd);
        return;
        }
    sscanf(&cmnd[++posn],"%d",&vict);
    if(vict<100 || vict>100+NUMSHIPS) {
        fprintf(trns[plr],"Illegal recipient ship number:%d:%s",vict,cmnd);
        return;
        }
    posn+=3;
    switch(cmnd[posn]) {
        case 'i':	case 'I':
            Prnt(TENDIND);
            OUTTR(fprintf(df,"Tend %d indfrom ship %d to ship %d\n",amnt,sbj,vict));
            break;
        case 'd':	case 'D':
            Prnt(TENDDEF);
            OUTTR(fprintf(df,"Tend %d PDU from ship %d into ship %d\n",amnt,sbj,vict));
            break;
        case 'm':	case 'M':
            OUTTR(fprintf(df,"Tend %d mines from ship %d into ship %d\n",amnt,sbj,vict));
            Prnt(TENDMIN);
            break;
        case 'r':	case 'R':
            sscanf(&cmnd[++posn],"%d",&type);
            if(type<0 || type>9) {
                fprintf(trns[plr],"%s:Invalid type:%d\n",cmnd,type);
                return;
                }
            OUTTR(fprintf(df,"Tend %d ore type %d from ship %d into ship %d\n",amnt,type,sbj,vict));
            Prnt(TENDORE);
            Prnt(type);
            break;
        case 's':	case 'S':
            OUTTR(fprintf(df,"Tend %d spacemines from ship %d to ship %d\n",amnt,sbj,vict));
            Prnt(TENDSPM);
            break;
        default:
            fprintf(stderr,"Tend type unkown %s\n",cmnd);
            fprintf(trns[plr],"Unknown Tend type:%s\n",cmnd);
        }
    Prnt(sbj-100);
    Prnt(vict-100);
    Prnt(amnt);
    return;
}

/*****************************************************************************/
void ParseTrans(void)
/*****************************************************************************/
/* Parse all ore transmutation cmnds */
{
    int cnt;
    Amount amt;
    Amount to,from;

    TRTRN(printf("ParseTrans()\n"));

    sscanf(&cmnd[4],"%d",&amt);
    if(amt<0) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amt);
        return;
        }
    for(cnt=5;cmnd[cnt];cnt++)
        if(cmnd[cnt]=='R' || cmnd[cnt]=='r')
            break;
    sscanf(&cmnd[cnt+1],"%d",&from);
    if(from<0 && from>9) {
        fprintf(trns[plr],"%s:Invalid Ore Type:%d\n",cmnd,from);
        return;
        }
    sscanf(&cmnd[cnt+3],"%d",&to);
    if(to<0 && to>9) {
        fprintf(trns[plr],"%s:Invalid Ore Type:%d\n",cmnd,to);
        return;
        }
    Prnt(TRANSORE);
    Prnt(sbj-100);
    Prnt(amt);
    Prnt(from);
    Prnt(to);
    OUTTR(fprintf(df,"Transmuting %d ore from R%d to R%d on planet %d\n",amt,from,to,sbj));
}

/*****************************************************************************/
void ParsUnbld(void)
/*****************************************************************************/
/* Parse all cmnds involving the unbuilding of units from ships        */
{
    int amount;
    int count,err=0;

    TRTRN(printf("ParsUnbld\n"));

    sscanf(&cmnd[5],"%d",&amount);
    if(amount<1) {
        fprintf(trns[plr],"%s:Invalid amount:%d\n",cmnd,amount);
        return;
        }
    for(count=5;isdigit(cmnd[count]);count++);
    switch((cmnd[count++])) {
        case 'c':	case 'C':
            Prnt(UNBLDCAR);
            OUTTR(fprintf(df,"Unbuilding %d cargo units on ship %d\n",amount,sbj));
            TRTRN(printf("Unbuild cargo\n"));
            break;
        case 'f':	case 'F':
            Prnt(UNBLDFGT);
            OUTTR(fprintf(df,"Unbuilding %d fighter units on S%d\n",amount,sbj));
            TRTRN(printf("Unbuild fighter\n"));
            break;
        case 't':	case 'T':
            Prnt(UNBLDTRC);
            OUTTR(fprintf(df,"Unbuilding %d tractor units on S%d\n",amount,sbj));
            TRTRN(printf("Unbuild tractor\n"));
            break;
        case 's':	case 'S':
		Prnt(UNBLDSLD);
		OUTTR(fprintf(df,"Unbuilding %d shield units on S%d\n",amount,sbj));
		TRTRN(printf("Unbuild shield\n"));
		break;
	default:
		fprintf(stderr,"Unknown ship unbuild type %s\n",cmnd);
		fprintf(trns[plr],"Unknown unit type:%s\n",cmnd);
		err=1;
	}
    if(!err) {
        Prnt(sbj-100);
        Prnt(amount);
        }
    return;
}

/*****************************************************************************/
void GameLength(void)
/*****************************************************************************/
/* Change the desired game length of the player */
{
    int num;

    TRTRN(printf("GameLength()\n"));

    sscanf(&cmnd[4],"%d",&num);
    if(num<10 || num > 100) {
        OUTTR(fprintf(df,"Turn %d out of range\n",num));
        fprintf(trns[plr],"Turn out of range:%s\n",cmnd);
        return;
        }
        
    Prnt(GAMELEN);
    Prnt(num);
    OUTTR(fprintf(df,"Setting desired gamelength to %d\n",num));
    return;
}
