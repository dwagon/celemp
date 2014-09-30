/* FILE.C to do all the i/o stuff for Celestial Empire by Zer Dwagon */
/* (c) 1992 Dougal Scott */
/* $Header: /nelstaff/edp/dwagon/rfs/RCS/file.c,v 1.47 1993/10/20 03:56:22 dwagon Exp $ */
/* $Log: file.c,v $
 * Revision 1.47  1993/10/20  03:56:22  dwagon
 * Added turn winning condition. Save and loaded here.
 *
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:02:50  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  13:54:37  dwagon
 * Initial RCS'd version
 * */

/* 18/5/92	Added game details structure
 * 20/5/92	Removed underscores, and changed function names
 ****** Version 1.44 ******
 * 20/6/92	Made Compress a separate function
 */

#include "def.h"

#define TRFIL(x)	if(strstr(dbgstr,"FILE") || strstr(dbgstr,"file")) x

extern int errno;
extern FILE *trns[NUMPLAYERS+1];
extern Number score[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern int turn,gm;
extern Ship shiptr;
extern Number price[10];
extern char name[NUMPLAYERS+1][10];
extern char *path;
extern Flag alliance[NUMPLAYERS+1][NUMPLAYERS+1];
extern char *dbgstr;
extern Number ecredit[NUMPLAYERS+1];
extern game gamedet;
extern int desturn[NUMPLAYERS+1];

/*****************************************************************************/
int OpenExhist(const char *mode)
/*****************************************************************************/
/* Open the execution history files */
{
char str[124];
int x;

for(x=0;x<NUMPLAYERS+1;x++) {
	sprintf(str,"%s%d/exhist.%d",path,gm,x);
	TRFIL(printf("Opening exechist file %s\n",str));
	if((trns[x]=fopen(str,mode))==NULL) {
		fprintf(stderr,"file:OpenExhist: Couldn't open %s for writing\n",str);
		return(-1);
		}
	}
return(0);
}

/*****************************************************************************/
void CloseExhist(void)
/*****************************************************************************/
/* Close the execution history files */
{
int x;

for(x=0;x<NUMPLAYERS+1;x++) {
	TRFIL(printf("Closing exechist file #%d\n",x));
	fclose(trns[x]);
	}
return;
}

/*****************************************************************************/
int ReadGalflt(void)
/*****************************************************************************/
/* Read in the whole data structure     */
{
int gsize,gmbak;
char *gstart;
FILE *galfile;
char str[124], str2[124];

sprintf(str,"%s%d/galfile",path,gm);

TRFIL(printf("Opening file %s for reading\n",str));
/* Try to open uncompressed data file */
if((galfile=fopen(str,"rb"))==NULL) {
	/* See if compressed datafile exists */
	sprintf(str2,"%s%d/galfile.Z",path,gm);
	if(access(str2,R_OK)==-1) {
		fprintf(stderr,"ReadGalflt: Couldn't open %s for reading\n",str);
		return(-1);
		}
	else {
/* Try to uncompress data file */
		TRFIL(printf("Uncompressing %s\n",str2));
		sprintf(str2,"uncompress %s%d/galfile.Z",path,gm);
		if(system(str2)!=0) {
			fprintf(stderr,"ReadGalflt: Uncompression of compressed data file failed\n");
			return(-1);
			}
		else {
/* Try to open uncompressed data file */
			if((galfile=fopen(str,"rb"))==NULL) {
				fprintf(stderr,"ReadGalflt:Couldn't open %s for readng\n",str);
				return(-1);
				}
			}
		}
	}

gstart=(char *)name;
gsize=sizeof(name);
TRFIL(printf("Reading in name table (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read name file failed\n");
	return(-1);
	}

gstart=(char *)galaxy;
gsize=sizeof(galaxy);
TRFIL(printf("Reading galaxy structure (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	fprintf(stderr,"Read galaxy failed\n");
	return(-1);
	}

gsize=sizeof(fleet);
gstart=(char *)fleet;
TRFIL(printf("Reading fleet structure (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read fleet failed\n");
	return(-1);
	}

gstart=(char *) &shiptr;
gsize=sizeof(shiptr);
TRFIL(printf("Reading shiptr (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read shiptr failed\n");
	return(-1);
	}

gstart=(char *) &gmbak;
gsize=sizeof(gmbak);
TRFIL(printf("Reading gm (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read gm failed\n");
	return(-1);
	}

if(gm!=gmbak) {
	printf("Savefile has been modified\n");
	printf("Gm:%d Read-in-Gm:%d\n",gm,gmbak);
	/* return(-1); */
	}

gstart=(char *) &turn;
gsize=sizeof(turn);
TRFIL(printf("Reading turn (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read turn failed\n");
	return(-1);
	}

gstart=(char *)score;
gsize=sizeof(score);
TRFIL(printf("Reading in score (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read score failed\n");
	return(-1);
	}

gstart=(char *)price;
gsize=sizeof(price);
TRFIL(printf("Reading in price (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read price failed\n");
	return(-1);
	}

gstart=(char *)alliance;
gsize=sizeof(alliance);
TRFIL(printf("Reading in alliance table (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read alliance table failed\n");
	return(-1);
	}

gstart=(char *)ecredit;
gsize=sizeof(ecredit);
TRFIL(printf("Reading in ecredit table (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read ecredit table failed\n");
	return(-1);
	}

gstart=(char *) &gamedet;
gsize=sizeof(gamedet);
TRFIL(printf("Reading in gamedet table (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read gamedet table failed\n");
	/* return(-1); */
	}

gstart=(char *) &desturn;
gsize=sizeof(desturn);
TRFIL(printf("Reading in desturn table (%d bytes).\n",gsize));
if(!fread(gstart,gsize,1,galfile)) {
	printf("Read desturn table failed\n");
	/* return(-1); */
	}
fclose(galfile);
return(0);
}

/*****************************************************************************/
void WriteGalflt(void)
/*****************************************************************************/
/* Write the whole data structure       */
{
int gsize;
char *gstart;
FILE *galfile;
char str[124],tmpstr[124],deststr[124];

/* Move any existing datafile to /tmp */
/* Check for compressed version */
sprintf(str,"%s%d/galfile.Z",path,gm);
if(access(str,R_OK)==0) {
	TRFIL(printf("Moving existing version to /tmp/%s.Z\n",tmpstr));
	sprintf(tmpstr,"gfileA%d.XXXXXX",gm);
	mktemp(tmpstr);
	TRFIL(printf("Moving existing version to /tmp/%s.Z\n",tmpstr));
	sprintf(deststr,"mv %s%d/galfile.Z /tmp/%s.Z",path,gm,tmpstr);
	(void)system(deststr);
	unlink(str);
	}
/* Check for uncompressed version */
sprintf(str,"%s%d/galfile",path,gm);
if(access(str,R_OK)==0) {
	sprintf(tmpstr,"gfileB%d.XXXXXX",gm);
	mktemp(tmpstr);
	TRFIL(printf("Moving existing version to /tmp/%s\n",tmpstr));
	sprintf(deststr,"mv %s%d/galfile /tmp/%s",path,gm,tmpstr);
	(void)system(deststr);
	TRFIL(printf("Compressing bakup save file...\n"));
	sprintf(deststr,"compress -f /tmp/%s",tmpstr);
	(void)system(deststr);
	unlink(str);
	}

TRFIL(printf("Opening file for writing\n"));
sprintf(str,"%s%d/galfile",path,gm);
if((galfile=fopen(str,"wb"))==NULL) {
	fprintf(stderr,"Could not open file %s for writing\n",str);
	fprintf(stderr,"Writing to /tmp/galfile%d instead\n",gm);
	sprintf(str,"/tmp/galfile%d",gm);
	if((galfile=fopen(str,"wb"))==NULL) {
		fprintf(stderr,"That barfed too. You lose\n");
		exit(-1);
		}
	}

gsize=sizeof(name);
TRFIL(printf("Writing names structure (%d bytes).\n",gsize));
gstart=(char *)name;
TRFIL(printf("Writing name file\n"));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write names failed\n");

gsize=sizeof(galaxy);
gstart=(char *)galaxy;
TRFIL(printf("Writing galaxy structure (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write galaxy failed\n");

gsize=sizeof(fleet);
gstart=(char *)fleet;
TRFIL(printf("Writing fleet structure (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write fleet failed\n");

gstart=(char *) &shiptr;
gsize=sizeof(shiptr);
TRFIL(printf("Writing shiptr (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write shiptr failed\n");

gstart=(char *) &gm;
gsize=sizeof(gm);
TRFIL(printf("Writing gm (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write gm failed\n");

gstart=(char *) &turn;
gsize=sizeof(turn);
TRFIL(printf("Writing turn (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write turn failed\n");

gstart=(char *)score;
gsize=sizeof(score);
TRFIL(printf("Writing score (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write score failed\n");

gstart=(char *)price;
gsize=sizeof(price);
TRFIL(printf("Writing price (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write price failed\n");

gstart=(char *)alliance;
gsize=sizeof(alliance);
TRFIL(printf("Writing alliance table (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write alliance table failed\n");

gstart=(char *)ecredit;
gsize=sizeof(ecredit);
TRFIL(printf("Writing ecredit table (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write ecredit table failed\n");

gstart=(char *) &gamedet;
gsize=sizeof(gamedet);
TRFIL(printf("Writing gamedet table (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write gamedet table failed\n");

gstart=(char *) &desturn;
gsize=sizeof(desturn);
TRFIL(printf("Writing desturn table (%d bytes).\n",gsize));
if(!fwrite(gstart,gsize,1,galfile))
	printf("Write desturn table failed\n");

fclose(galfile);
}

/*****************************************************************************/
void Compress(void)
/*****************************************************************************/
/* Compress the save file */
{
char str[124];

TRFIL(printf("Compressing save file...\n"));
sprintf(str,"compress -f %s%d/galfile",path,gm);
(void)system(str);
return;
}
