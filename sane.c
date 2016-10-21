/* Check for sanity in file system */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/sane.c,v 1.48 1993/07/08 03:24:18 dwagon Exp dwagon $ */
/* $Log: sane.c,v $
 * Revision 1.48  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.47  1993/07/06  06:58:14  dwagon
 * Added definition for NEUTPLR instead of 9 for future changes
 *
 * Revision 1.46  1993/05/24  05:04:47  dwagon
 * Fixed pointer problem
 *
 * Revision 1.45  1993/05/24  05:03:58  dwagon
 * *** empty log message ***
 *
 * Revision 1.44  1992/09/16  14:00:44  dwagon
 * Initial RCS'd version
 * */

#include "def.h"

char *dbgstr;
FILE *trns[NUMPLAYERS+1];
Number score[NUMPLAYERS+1];
planet galaxy[NUMPLANETS];
game gamedet;
ship fleet[NUMSHIPS];
int turn,gm;
Ship shiptr;
Number price[10];
char name[NUMPLAYERS+1][10];
Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
Number ecredit[NUMPLAYERS+1];
int desturn[NUMPLAYERS+1];

int changed=0;

int main(int argc,char **argv)
{
char *gmstr;
int count,count2;

if((dbgstr = getenv("CELEMPDEBUG")) == NULL ) {
	dbgstr=(char *)"null";
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
if(ReadGalflt()!=0) {
	fprintf(stderr,"Read failed. Fatal Error. Aborting\n");
	exit(-1);
	}
printf("Checking sanity...\n");
for(count=0;count<NUMPLANETS;count++) {
	if(galaxy[count].owner<0 || galaxy[count].owner>9) {
		printf("Planet %d has owner %d\n",count,galaxy[count].owner);
		galaxy[count].owner=NEUTPLR;
		changed=1;
		}
	for(count2=0;count2<10;count2++) {
		if(galaxy[count].ore[count2]<0 || galaxy[count].ore[count2]>1000) {
			printf("Planet %d has ore type %d at %d\n",count,count2,galaxy[count].ore[count2]);
			galaxy[count].ore[count2]=0;
			changed=1;
			}
		if(galaxy[count].mine[count2]<0 || galaxy[count].mine[count2]>1000) {
			printf("Planet %d has mine type %d at %d\n",count,count2,galaxy[count].mine[count2]);
			galaxy[count].mine[count2]=0;
			changed=1;
			}
		}
	if(galaxy[count].ind<0 || galaxy[count].ind>1000) {
		printf("Planet %d has %d industry\n",count,galaxy[count].ind);
		galaxy[count].ind=0;
		changed=1;
		}
	}
if(changed) {
	printf("Writing galaxy...\n");
	WriteGalflt();
	}
return(0);
}
