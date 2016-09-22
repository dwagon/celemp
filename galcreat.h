/* Prototype file for galcreat.c */
/* (c) 2016 Dougal Scott */

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
int LinkCmp(const void *a, const void *b);
void SortLinks(void);
void givePlanetName(Planet num);
