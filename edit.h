/* EDIT.H - header file for structure editor in Celestial Empire */
/* (c) 1992 Dougal Scott */
/* $Header: /nelstaff/edp/dwagon/rfs/RCS/edit.h,v 1.45 1993/07/06 06:10:06 dwagon Exp $ */
/* $Log: edit.h,v $
 * Revision 1.45  1993/07/06  06:10:06  dwagon
 * Added prototype for UnitIncrease()
 *
 * Revision 1.44  1992/09/16  13:54:18  dwagon
 * Initial RCS'd version
 * */

/* 30/5/92	Added Proto file analysis
 * 25/8/92	Added prototyping
 */
void bcopy();
void bzero();

int main(int argc, char **argv);
void MainMenu(void);
void PlanetEdit(void);
void ShipEdit(Ship num);
void AllianceEdit(void);
void NameEdit(void);
void ScoreEdit(void);
void PrintPlanet(Planet num, FILE *ef);
void PrintShip(Planet num, FILE *ef);
void PrintAlliance(FILE *ef);
void PrintName(FILE *ef);
void PrintScore(FILE *ef);
void AnalyzePlanet(char filename[80], Planet num);
void AnalyzeShip(char filename[80], Ship num);
void AnalyzeAlliance(char filename[80]);
void AnalyzeName(char filename[80]);
void AnalyzeScore(char filename[80]);
void Validate(void);
int ValidateLink(Planet pln, Planet lnk);
void PriceEdit(void);
void GameEdit(void);
void PrintGame(FILE *ef);
void UnitIncrease(void);
