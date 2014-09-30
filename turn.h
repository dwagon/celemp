/* Include file for turn creation program for Celestial Empire */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/turn.h,v 1.46 1993/05/24 03:10:29 dwagon Exp $ */
/* $Log: turn.h,v $
 * Revision 1.46  1993/05/24  03:10:29  dwagon
 * Moved CalcPlrInc to librfs
 *
 * Revision 1.45  1993/03/04  07:04:44  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.44  1992/09/16  14:02:17  dwagon
 * Initial RCS'd version
 * */

/*
 * 21/5/92	Removed underscores, and changed proc names
 ****** Version 1.44 ******
 * 25/8/92	Added prototyping
 */

#define TRTUR(x)    if(strstr(dbgstr,"TURN") || strstr(dbgstr,"turn")) x
#define TRTUR2(x)	if(strstr(dbgstr,"TUR2") || strstr(dbgstr,"turn2")) x

#include <string.h>

int		main(int argc, char **argv);
int		PrintGalaxy(char *fname);
void	Init(void);
void	Prnt(const char *string, FILE *stream);
void	Process(Player plr);
void	CatExhist(FILE *stream, Player plyr);
void	ListRes(FILE *stream);
void	CatMotd(FILE *stream);
void	CatSpec(FILE *stream, Player plr);
int		Interest(Player plr, Planet plan);
void	DoPlanet(Planet plan, Player plr, FILE *stream);
void	DoShip(Player plr, Planet plan, FILE *stream);
void	DoEnemy(Ship shp, FILE *stream);
void	DoFriend(Ship shp, FILE *stream);
int		ChekPlan(Planet plan, Player plr, FILE *stream);
void	ChekTot(Player plr, FILE *stream);
void	ChekShip(Ship shp, Player plr, FILE *stream);
void	ResOutput(Player plr, FILE *stream);
void	TypeSummary(FILE *outfile);
void	Headings(FILE *outfile, Player plr);
void	PlanetSummary(Player plr, FILE *outfile);
void	OwnerSummary(FILE *stream);
void	UnitSummary(FILE *stream);
void	WinningDetails(FILE *stream);
void	CostDetails(FILE *stream);
char	*TexName(char *str);
char	*ShipName(Ship shp);
void	ChekShipTot(Player plr,FILE *stream);
