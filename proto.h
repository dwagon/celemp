/* Definitions file for Celestial Empire
 * (C) 2016 Dougal Scott
 */

/* ATK.C */
void EngageTractor(Ship shp, Ship targ);
void ShpShp(Ship shp, Ship targ, Units amnt);
void PlanetAttack(Planet sbj, Ship vict, Units amnt);
void PlanetSpcmin(Planet sbj, Amount amnt);
void PduShp(Planet num, Ship shp, Amount amnt);
void ShpPdu(Ship shp, Units amnt);
void ShpSpcmin(Ship shp, Units amnt);
void SpcminShp(Ship shp);
void ShpOre(Oretype typ, Ship shp, Units amnt);
void ShpMin(Oretype typ, Ship shp, Units amnt);
void ShpInd(Ship shp, Units amnt);
void EarthRetaliate(void);

/* BLD.C */
void BuildSpcmines(Planet plan, Amount amt, Oretype type);
void BuildTrac(Ship shp, Amount amt);
void BuildShield(Ship shp, Amount amt);
void BuildInd(Planet sbj, Amount amt);
void BuildMine(Planet sbj, Oretype type, Amount amt);
void BuildHyp(Planet sbj, Amount amt, Units fgt, Units crg, Units trac, Units
shld);
void BuildDef(Planet num,Amount amt);
void BuildCargo(Ship shp, Amount amt);
void BuildFight(Ship shp, Amount amt);

/* EARTH.C */
void RebuildEarth(void);
void CalcEarthDmg(void);
void DoEarth(void);
void ContCarg(Ship shp, Amount amt, Number bid);
void ContFght(Ship shp, Amount amt, Number bid);
void ContTrac(Ship shp, Amount amt, Number bid);
void ContShld(Ship shp, Amount amt, Number bid);
void BuyOre(Ship shp, Amount amt, Oretype type);
void SellOre(Ship shp, Amount amt, Oretype type);

/* FILE.C */
int OpenExhist(const char *mode);
void CloseExhist(void);
int ReadGalflt(void);
void WriteGalflt(void);
void Compress(void);

/* JUMP.C */
int Chekfuel(Ship shp, Number dist);
int Chekpath(Ship shp, Planet dest);
void Moveto(Ship shp, Planet pln, Flag dist, int segm);
void Jump1(Ship shp, Planet dest1);
void Jump2(Ship shp, Planet dest1, Planet dest2);
void Jump3(Ship shp, Planet dest1, Planet dest2, Planet dest3);
void Jump4(Ship shp, Planet dest1, Planet dest2, Planet dest3, Planet dest4);
void Jump5(Ship shp, Planet dest1, Planet dest2, Planet dest3, Planet dest4,
Planet dest5);

/* LOAD.C */
void LoadSpcmine(Ship shp, Amount amt);
void LoadOre(Oretype typ, Ship shp, Amount amt);
void LoadDef(Ship shp, Amount amt);
void LoadMin(Oretype typ, Ship shp, Amount amt);
void LoadInd(Ship shp, Amount amt);
void LoadAll(Ship shp);
void LoadPri(Ship shp, int pri[13]);

/* MAIN.C */
void ProcessTurn(void);
void Execute(int cmd);
void ResetScan(void);
int ReadInTurn(void);

/* MISC.C */
void Transmute(Planet num, Amount amt, Oretype from, Oretype to);
void DeployShp(Ship shp, Amount amt);
void DeployPlnt(Planet plnt, Amount amt);
void Undeploy(Ship shp, Amount amt);
void GiftShip(Ship sbj, Player vict);
void GiftPlan(Planet sbj, Player vict);
void NamePlanet(Planet sbj, char *pname);
void NameShip(Ship sbj, char *pname);
void HelpHelp(const char *target);

/* PRODUCT.C */
void ProdMine(Planet num, Oretype min);
Ship DetectShip(Ship num, Ship start);
int MultiOwner(Planet num);
void ChngOwner(Planet num);
void ProdAll(Planet num);
void ChngHull(Ship shp);
void EndOfTurn(void);
void ResolveDamage(void);
void PerformShots(Ship shp);
void RecalcCargo(Ship shp);
void RemoveCargo(Ship shp);
void DistrMin(Planet plan);
void CalcPrices(void);
void CheckWinning(void);
void CalcEndTurn(void);

/* TEND.C */
void TendSpcmine(Ship sbj, Ship vict, Amount amt);
void TendOre(Oretype typ, Ship sbj, Ship vict, Amount amt);
void TendDef(Ship sbj, Ship vict, Amount amt);
void TendMin(Ship sbj, Ship vict, Amount amt);
void TendInd(Ship sbj, Ship vict, Amount amt);
void TendAll(Ship sbj, Ship vict);

/* UNBLD.C */
void UnbldTrac(Ship shp, Amount amt);
void UnbldShield(Ship shp, Amount amt);
void UnbldCargo(Ship shp, Amount amt);
void UnbldFight(Ship shp, Amount amt);

/* UNLOAD.C */
void UnlSpcmin(Ship shp, Amount amt);
void UnlOre(Oretype typ, Ship shp, Amount amt);
void UnlDef(Ship shp, Amount amt);
void UnlMin(Oretype typ, Ship shp, Amount amt);
void UnlInd(Ship shp, Amount amt);
void UnlAll(Ship shp);

/* LIBRFS.C */
int CalcPlrInc(Player plr);
int IsEmpty(Ship shp);
int Pdus(Amount amnt);
int Shields(Ship shp);
int CalcWeight(Ship shp);
Number CalcType(Ship shp);
Number DeathStarType(Ship shp);
Number BattleType(Ship shp);
Number ShipType(Ship shp);
Number CargoType(Ship shp);
int IsHome(Planet plan, Player plr);
int IsResearch(Planet plan);
int IsEarth(Planet plan);
int LocateEarth(void);
int IsShip(Planet plan);
int NumRes(Player plr);
int Shots(Ship shp, Amount shts);
int CalcIncome(Planet num);
int EffEff(Ship shp);

/* Proto Library */
void LoadProto(char *protofile);
void ProtoWinning(char buff[80], char *tok, int line);
void ProtoHome(char buff[80], char *tok, int line);
void ProtoEarth(char buff[80], char *tok, int line);
void ProtoHomeOre(char buff[80], char *tok, int line);
void ProtoEarthOre(char buff[80], char *tok, int line);
void ProtoHomeMine(char buff[80], char *tok, int line);
void ProtoEarthMine(char buff[80], char *tok, int line);
void ProtoEarthBid(char buff[80], char *tok, int line);
void ProtoShip(char buff[80], char *tok, int line);
void ProtoGal(char buff[80], char *tok, int line);
void ProtoShip2(char buff[80], char *tok, int line);
