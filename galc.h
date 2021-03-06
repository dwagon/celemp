/* Galaxy Structure definition file for Celestial Empire
 * Do NOT distribute (c) 1992 Dougal Scott */

/* $Header: /nelstaff/edp/dwagon/rfs/RCS/galc.h,v 1.46 1993/07/08 03:24:18 dwagon Exp $ */
/* $Log: galc.h,v $
 * Revision 1.46  1993/07/08  03:24:18  dwagon
 * Made NEUTRAL player 0.
 * Removed lots of associated special checks for writing to trans[0] which
 * is now open.
 *
 * Revision 1.45  1993/03/04  07:04:44  dwagon
 * Changed debugging messages to a run-time option with dbgstr
 * Fixed up link table to remove warning about brackets.
 *
 * Revision 1.44  1992/09/16  13:55:51  dwagon
 * Initial RCS'd version
 * */

/* 21/5/92	Changed to remove all underscores.
 * 10/7/92	Re-ordered the home planets
 */

static int linkmap[NUMPLANETS+1][4]={
/*0*/{11,237,NP,NP} ,{2,13,225,NP} ,{1,3,NP,NP} ,{2,4,25,214} ,
/*4*/{3,NP,NP,230} ,{NP,226,6,17} ,{5,NP,7,NP} ,{6,218,8,28} ,
/*8*/{7,NP,NP,245} ,{NP,227,10,21} ,{9,NP,11,NP} ,{10,222,0,31} ,
/*12*/{23,NP,13,NP} ,{12,1,43,24} ,{NP,215,15,26} ,{14,NP,16,NP} ,
/*16*/{15,NP,17,NP} ,{16,5,47,27} ,{NP,219,19,29} ,{18,NP,20,NP} ,
/*20*/{19,NP,21,NP} ,{20,9,51,30} ,{NP,223,23,32} ,{22,NP,12,NP} ,
/*24*/{NP,13,NP,42} ,{NP,3,26,34} ,{25,14,NP,55} ,{NP,17,46,NP} ,
/*28*/{NP,7,29,37} ,{28,18,NP,58} ,{NP,21,NP,50} ,{NP,11,32,40} ,
/*32*/{31,22,NP,61} ,{34,NP,NP,NP} ,{25,33,44,NP} ,{ERTH,45,120,195} ,
/*36*/{37,NP,NP,NP} ,{28,36,48,NP} ,{ERTH,49,124,199} ,{40,NP,NP,NP} ,
/*40*/{31,39,52,NP} ,{ERTH,53,128,203} ,{24,54,NP,NP} ,{13,44,54,NP} ,
/*44*/{34,43,45,64} ,{35,44,231,NP} ,{27,57,NP,NP} ,{17,48,57,NP} ,
/*48*/{37,47,49,68} ,{38,48,246,NP} ,{30,60,NP,NP} ,{21,52,60,NP} ,
/*52*/{40,51,53,72} ,{41,52,237,NP} ,{42,43,75,NP} ,{26,56,65,NP} ,
/*56*/{55,NP,NP,NP} ,{46,47,76,NP} ,{29,59,69,NP} ,{58,NP,NP,NP} ,
/*60*/{50,51,77,NP} ,{32,62,73,NP} ,{61,NP,NP,NP} ,{74,236,64,238} ,
/*64*/{63,44,65,81} ,{64,55,66,92} ,{65,67,NP,NP} ,{66,68,231,232} ,
/*68*/{67,48,69,85} ,{68,58,70,96} ,{69,71,NP,NP} ,{70,72,245,247} ,
/*72*/{71,52,73,89} ,{72,61,74,100} ,{73,63,NP,NP} ,{54,79,NP,NP} ,
/*76*/{57,83,NP,NP} ,{60,87,NP,NP} ,{89,238,NP,NP} ,{75,80,91,NP} ,
/*80*/{79,81,NP,NP} ,{64,80,82,103} ,{81,234,NP,NP} ,{76,84,95,NP} ,
/*84*/{83,85,NP,NP} ,{68,84,86,106} ,{85,248,NP,NP} ,{77,88,99,NP} ,
/*88*/{87,89,NP,NP} ,{72,88,109,78} ,{91,101,NP,NP} ,{79,90,102,118} ,
/*92*/{65,93,104,NP} ,{92,94,NP,NP} ,{93,95,NP,NP} ,{83,94,105,122} ,
/*96*/{69,97,107,NP} ,{96,98,NP,NP} ,{97,99,NP,NP} ,{87,98,108,126} ,
/*100*/{73,101,110,NP} ,{90,100,NP,NP} ,{91,117,NP,NP} ,{81,104,112,NP} ,
/*104*/{92,103,130,NP} ,{95,121,NP,NP} ,{85,107,114,NP} ,{96,106,133,NP} ,
/*108*/{99,125,NP,NP} ,{89,110,116,NP} ,{100,109,136,NP} ,{112,NP,NP,NP} ,
/*112*/{103,111,119,NP} ,{114,NP,NP,NP} ,{106,113,123,NP} ,{116,NP,NP,NP} ,
/*116*/{109,115,127,NP} ,{102,129,NP,NP} ,{91,119,129,NP} ,{112,118,120,139} ,
/*120*/{35,119,233,NP} ,{105,132,NP,NP} ,{95,123,132,NP} ,{114,122,124,143} ,
/*124*/{38,123,249,NP} ,{108,135,NP,NP} ,{99,127,135,NP} ,{116,126,128,147} ,
/*128*/{41,127,240,NP} ,{117,118,150,NP} ,{104,131,140,NP} ,{130,NP,NP,NP} ,
/*132*/{121,122,151,NP} ,{107,134,144,NP} ,{133,NP,NP,NP} ,{125,126,152,NP} ,
/*136*/{110,137,148,NP} ,{136,NP,NP,NP} ,{149,139,240,241} ,{138,119,140,156} ,
/*140*/{130,139,141,167} ,{140,142,NP,NP} ,{141,143,234,253} ,{123,142,144,160} ,
/*144*/{133,143,145,171} ,{144,146,NP,NP} ,{145,147,247,252} ,{127,146,148,164} ,
/*148*/{136,147,149,175} ,{148,138,NP,NP} ,{129,154,NP,NP} ,{132,158,NP,NP} ,
/*152*/{135,162,NP,NP} ,{164,242,NP,NP} ,{150,155,166,NP} ,{154,156,NP,NP} ,
/*156*/{139,155,157,178} ,{156,253,NP,NP} ,{151,159,170,NP} ,{158,160,NP,NP} ,
/*160*/{143,159,161,181} ,{160,250,NP,NP} ,{152,174,163,NP} ,{162,164,NP,NP} ,
/*164*/{147,163,153,184} ,{166,176,NP,NP} ,{154,165,177,193} ,{140,168,179,NP} ,
/*168*/{167,169,NP,NP} ,{168,170,NP,NP} ,{158,180,197,169} ,{144,172,182,NP} ,
/*172*/{171,173,NP,NP} ,{172,174,NP,NP} ,{162,173,183,201} ,{148,176,185,NP} ,
/*176*/{175,165,NP,NP} ,{166,192,NP,NP} ,{156,179,187,NP} ,{167,178,205,NP} ,
/*180*/{170,196,NP,NP} ,{160,182,189,NP} ,{171,181,208,NP} ,{174,200,NP,NP} ,
/*184*/{164,185,191,NP} ,{175,184,211,NP} ,{187,NP,NP,NP} ,{178,186,194,NP} ,
/*188*/{189,NP,NP,NP} ,{181,188,198,NP} ,{191,NP,NP,NP} ,{184,190,202,NP} ,
/*192*/{177,204,NP,NP} ,{166,204,194,NP} ,{187,193,195,214} ,{35,194,255,NP} ,
/*196*/{180,207,NP,NP} ,{170,207,198,NP} ,{189,197,199,218} ,{38,198,251,NP} ,
/*200*/{183,210,NP,NP} ,{174,210,202,NP} ,{191,201,203,222} ,{202,41,243,NP} ,
/*204*/{192,193,225,NP} ,{179,206,215,NP} ,{205,NP,NP,NP} ,{196,197,226,NP} ,
/*208*/{182,209,219,NP} ,{208,NP,NP,NP} ,{200,201,227,NP} ,{185,212,223,NP} ,
/*212*/{211,NP,NP,NP} ,{224,243,235,214} ,{194,213,215,3} ,{205,214,216,14} ,
/*216*/{215,217,NP,NP} ,{216,218,255,229} ,{217,198,219,7} ,{218,208,220,18} ,
/*220*/{219,221,NP,NP} ,{220,222,251,244} ,{202,221,223,11} ,{211,222,224,22} ,
/*224*/{223,213,NP,NP} ,{204,1,NP,NP} ,{207,5,NP,NP} ,{210,9,NP,NP} ,
/*228*/{35,38,41,NP} ,{217,231,NP,NP} ,{4,231,NP,NP} ,{45,67,229,230} ,
/*232*/{67,233,NP,NP} ,{120,232,234,NP} ,{82,233,142,NP} ,{213,236,NP,NP} ,
/*236*/{235,237,63,NP} ,{0,236,53,NP} ,{63,78,239,NP} ,{238,240,NP,NP} ,
/*240*/{138,239,128,NP} ,{138,242,NP,NP} ,{153,241,243,NP} ,{242,213,203,NP} ,
/*244*/{221,245,NP,NP} ,{8,71,244,246} ,{49,245,NP,NP} ,{71,146,249,NP} ,
/*248*/{86,249,NP,NP} ,{248,247,124,NP} ,{161,252,NP,NP} ,{199,221,252,NP} ,
/*252*/{146,250,251,NP} ,{142,157,254,NP} ,{253,255,NP,NP} ,{195,217,254,NP} 
} ;

static int res[NUMRES]=
{
	25,28,31,33,36,39,66,70,74,103,106,109,111,113,115,141,145,149,
	178,181,184,186,188,190,216,220,224 };

static int home[NUMPLAYERS+1]=
{
	0,214,68,222,143,139,147,64,218,72 };

