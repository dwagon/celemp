/* Prototype file for galcreat.c */
/* (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/galcreat.h,v 1.46 1993/09/17 07:24:05 dwagon Exp $ */
/* $Log: galcreat.h,v $
 * Revision 1.46  1993/09/17  07:24:05  dwagon
 * Planets are given initial names
 *
 * Revision 1.45  1993/07/15  06:44:37  dwagon
 * Added InitShip2() and moved InitShip() to InitShip1()
 *
 * Revision 1.44  1992/09/16  13:56:45  dwagon
 * Initial RCS'd version
 * */

/* 21/5/92	Changed to remove all underscores
 * 31/5/92	Added link sorting
 * 25/8/92	Added prototyping
 */

char *index();

int main(int,char **);
void Init(Planet num);
void PrtDesc(Planet pln);
void SetHome(Player plyr, Planet plnt);
void SetResch(Planet plnt);
void SetEarth(Planet plnt);
int Normal(void);
void DistrMine(int nummin, Planet num);
void SetPlan(Planet num);
void InitDriver(char *protofile);
void InitAlliances(void);
void Ships(Player plr, Planet plnt);
void WriteGalaxyInfo(void);
void WriteShipInfo(void);
void GetName(Player plr);
void PrtShip(Ship shp);
void ShipSet(Planet plan, Player plr);
void InitShip1(Ship shp);
void InitShip2(Ship shp);
int LinkCmp(Link *a, Link *b);
void SortLinks(void);
void NamePlanet(Planet num);
