/* Include file for turn creation program for Celestial Empire */
/* (c) 2016 Dougal Scott */

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
void	AllianceStatus(Player plr, FILE *stream);
void	EarthDetails(Player plr, FILE *stream);
void	CatMotd(FILE *stream);
void	CatSpec(FILE *stream, Player plr);
int		Interest(Player plr, Planet plan);
void	DoPlanet(Planet plan, Player plr, FILE *stream);
void	GraphPlanet(Planet plan, Player plr, FILE *stream);
void	DoShip(Player plr, Planet plan, FILE *stream);
void	DoEnemy(Ship shp, FILE *stream);
void	DoFriend(Ship shp, FILE *stream);
int		ChekPlan(Planet plan, Player plr, FILE *stream);
void	ChekTot(Player plr, FILE *stream);
void	ChekShip(Ship shp, Player plr, FILE *stream);
void	ResOutput(Player plr, FILE *stream);
void	TypeSummary(FILE *outfile);
void	Headings(FILE *outfile, FILE *dotfile, Player plr);
void	ShipSummary(Player plr, FILE *outfile);
void	PlanetSummary(Player plr, FILE *outfile);
void	OwnerSummary(FILE *stream);
void	UnitSummary(FILE *stream);
void	WinningDetails(FILE *stream);
void	CostDetails(FILE *stream);
char	*TexName(char *str);
char	*ShipName(Ship shp);
void	ChekShipTot(Player plr,FILE *stream);
