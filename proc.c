/*
 * texproc: embedded command preprocessor for TeX and LaTeX
 * (c) 2016 Dougal Scott
 * Any comments, criticisms, patches to 
 * Dougal.Scott@gmail.com
 *
 * Convert LaTeX:
 * ...
 * %# gnuplot
 * plot sin, cos, and tan
 * %#
 * ...
 *
 * to 
 *
 * ...
 * \begin{picture}
 * \lotsadots
 * \end{picture}
 * ...
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

int process(char *);

extern int errno;
FILE *tmpfh;

int main(int argc, char **argv)
{
    char buff[BUFSIZ];

    while(fgets(buff, sizeof(buff), stdin)!=NULL) {
        if(buff[0]=='%' && buff[1]=='#') {
            process(buff);
        }
        else {
            fprintf(stdout, "%s", buff);
        }
    }
    return(0);
}

int runprogram(char *progname, char *tmpname)
{
    FILE	*p;
    char cmdline[BUFSIZ];
    char outbuff[BUFSIZ];

    progname[strcspn(progname, "\n")] = 0;
    sprintf(cmdline, "%s %s", progname, tmpname);
    fprintf(stderr, "%s %s\n", progname, tmpname);
    if((p=popen(cmdline, "r"))==NULL) {
        fprintf(stderr, "Could not open pipe to %s\n", cmdline);
        exit(-1);
    }
    while(fgets(outbuff, sizeof(outbuff), p)!=NULL) {
        fprintf(stdout, "%s", outbuff);
    }
    pclose(p);
    // unlink(tmpname);
    return(0);
}

FILE *open_temp(char *tmpname) {
    if((tmpfh=fopen(tmpname, "w"))==NULL) {
        fprintf(stderr, "Could not open %s for writing\n", tmpname);
        fprintf(stderr, "Program aborting\n");
        exit(-1);
        }
    return tmpfh;
}

int process(char *buff)
{
    char tmpbuf[BUFSIZ];
    char progname[BUFSIZ];
    char tmpname[BUFSIZ];

    strcpy(tmpname, "/tmp/PrcXXXXXX");
    mktemp(tmpname);

    tmpfh = open_temp(tmpname);
    strcpy(progname, &buff[3]);
    fprintf(stdout, "%% Including output from %s\n", progname);
    while(fgets(tmpbuf, sizeof(tmpbuf), stdin) != NULL) {
        if(tmpbuf[0]=='%' && tmpbuf[1]=='#') {
            fclose(tmpfh);
            runprogram(progname, tmpname);
            return(0);
        }
        else {
            fprintf(tmpfh, "%s", tmpbuf);
        }
    }
    return(0);
}

/* EOF */
