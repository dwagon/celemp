/* TRANS.H 	Include file for Celestial Empire Turn translation program */
/* (C) Dougal Scott 1992 */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/trans.h,v 1.47 1993/10/20 03:58:51 dwagon Exp $ */
/* $Log: trans.h,v $
 * Revision 1.47  1993/10/20  03:58:51  dwagon
 * Added GameLength() function
 *
 * Revision 1.46  1993/03/04  07:04:44  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 *
 * Revision 1.45  1992/11/09  03:34:53  dwagon
 * Added prototype of ore attack command AttOre()
 *
 * Revision 1.44  1992/09/16  14:01:30  dwagon
 * Initial RCS'd version
 * */

/*
 * 25/8/92	Added prototyping
 */

#include <ctype.h>

#define TRTRN(x)	if(strstr(dbgstr,"TRANS") || strstr(dbgstr,"trans")) x
#define TRTRN2(x)	if(strstr(dbgstr,"TRAN2") || strstr(dbgstr,"tran2")) x
#define OUTTR(x)	x

/* Integer log, good up to log 4 */
#define ilog(x)     ((x>9?(x>99?(x>999?4:3):2):1))

int main(int argc, char **argv);
void WriteSord(void);
void OpenDebug(void);
void InitArray(void);
void ReadIn(void);
void Prnt(int numb);
void Parse(void);
void SetSord(void);
void ShipSord(void);
void PlanSord(void);
void ClearSord(void);
void ClearShipSord(void);
void ClearPlanSord(void);
void Broadcast(void);
void Personal(void);
void AllMsg(void);
void Scan(void);
void ChangeAlliance(void);
void ShpPar(void);
void TransGiftPlan(void);
void ShipGift(void);
int AllSelect(char *victname);
int UserSelect(char *victname);
void TransSellOre(void);
void TransBuyOre(void);
void PlanetName(void);
void ShipName(void);
void EngageTrac(void);
void PlnPar(void);
void PlanDeploy(void);
void ParsDeploy(void);
void ParsRetrieve(void);
void PlanAtt(int shots);
void PlanBuild(void);
void ParpSpcmin(int amount, int type);
void ParsUnld(void);
void ParsLoad(void);
void ParsBuishi(void);
void ParsJump(void);
void Dest(int pos);
void ParpHyp(int amount);
void ParpDef(int amount);
void ParpInd(int amount);
void ParpMin(int amount, int type);
void ParsAtt(void);
void AttSpcmin(int shots);
void AttShip(int shots);
void AttInd(int shots);
void AttDef(int shots);
void AttOre(int shots);
void AttMin(int shots);
void CmdDump(void);
void OpenSord(const char *mode);
void CloseSord(void);
void OpenBid(void);
void CloseBid(void);
void ValidCheck(void);
void ParsTend(void);
void TendState(void);
void ParseTrans(void);
void ParsUnbld(void);
void MegaDump(void);
void GameLength(void);
