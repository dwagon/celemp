/* FILE.C to do all the i/o stuff for Celestial Empire by Zer Dwagon */
/* (c) 2016 Dougal Scott */

#include "def.h"
#include <json.h>
#include <json_util.h>

extern int errno;
extern FILE *trns[NUMPLAYERS+1];
extern Number score[NUMPLAYERS+1];
extern planet galaxy[NUMPLANETS];
extern ship fleet[NUMSHIPS];
extern int turn, gm;
extern Ship shiptr;
extern Number price[10];
extern char name[NUMPLAYERS+1][10];
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
    char filename[BUFSIZ];

    for(int plr=0;plr<NUMPLAYERS+1;plr++) {
        PlrFile("exhist", plr, filename);
        if((trns[plr]=fopen(filename, mode)) == NULL) {
            fprintf(stderr, "file:OpenExhist: Couldn't open %s for writing\n", filename);
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

    for(x=0; x<NUMPLAYERS+1; x++) {
        fclose(trns[x]);
        }
    return;
}

/*****************************************************************************/
const char *GameDir()
/*****************************************************************************/
{
    char *dname;
    char game_path[BUFSIZ];
    dname = malloc(BUFSIZ);

    if((getenv("CELEMPPATH")) == NULL) {
        fprintf(stderr,"set CELEMPPATH to the appropriate directory\n");
        exit(-1);
    }
    else {
        strncpy(game_path, getenv("CELEMPPATH"), BUFSIZ);
    }

    sprintf(dname, "%s/game%d", game_path, gm);
    return dname;
}

/*****************************************************************************/
void FilePath(const char *name, char *filename)
/*****************************************************************************/
{
    sprintf(filename, "%s/%s", GameDir(), name);
}

/*****************************************************************************/
void PlrFile(const char *name, const Player plr, char *filename)
/*****************************************************************************/
{
    char namepatt[BUFSIZ];

    sprintf(namepatt, "%s.%d", name, plr);
    FilePath(namepatt, filename);
}

/*****************************************************************************/
void LoadPlanet(int pln, json_object *jso)
/*****************************************************************************/
{
    int arraylen;

    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "numlinks") == 0) {
            galaxy[pln].numlinks = json_object_get_int(val);
        }
        else if(strcmp(key, "name") == 0) {
            strncpy(galaxy[pln].name, json_object_get_string(val), NAMESIZ);
        }
        else if(strcmp(key, "stndord") == 0) {
            strncpy(galaxy[pln].stndord, json_object_get_string(val), 10);
        }
        else if(strcmp(key, "deployed") == 0) {
            galaxy[pln].deployed = json_object_get_int(val);
        }
        else if(strcmp(key, "pduleft") == 0) {
            galaxy[pln].pduleft = json_object_get_int(val);
        }
        else if(strcmp(key, "spacemine") == 0) {
            galaxy[pln].spacemine = json_object_get_int(val);
        }
        else if(strcmp(key, "scanned") == 0) {
            galaxy[pln].scanned = json_object_get_int(val);
        }
        else if(strcmp(key, "spec") == 0) {
            galaxy[pln].spec = json_object_get_int(val);
        }
        else if(strcmp(key, "income") == 0) {
            galaxy[pln].income = json_object_get_int(val);
        }
        else if(strcmp(key, "pdu") == 0) {
            galaxy[pln].pdu = json_object_get_int(val);
        }
        else if(strcmp(key, "owner") == 0) {
            galaxy[pln].owner = json_object_get_int(val);
        }
        else if(strcmp(key, "ind") == 0) {
            galaxy[pln].ind = json_object_get_int(val);
        }
        else if(strcmp(key, "indleft") == 0) {
            galaxy[pln].indleft = json_object_get_int(val);
        }
        else if(strcmp(key, "ore") == 0) {
            arraylen = json_object_array_length(val);
            json_object *tmp;
            for (int rtype = 0; rtype < arraylen; rtype++) {
                tmp = json_object_array_get_idx(val, rtype);
                galaxy[pln].ore[rtype] = json_object_get_int(tmp);
            }
        }
        else if(strcmp(key, "mine") == 0) {
            arraylen = json_object_array_length(val);
            json_object *tmp;
            for (int rtype = 0; rtype < arraylen; rtype++) {
                tmp = json_object_array_get_idx(val, rtype);
                galaxy[pln].mine[rtype] = json_object_get_int(tmp);
            }
        }
        else if(strcmp(key, "knows") == 0) {
            arraylen = json_object_array_length(val);
            json_object *tmp;
            for (int plr = 0; plr <= NUMPLAYERS; plr++) {
                tmp = json_object_array_get_idx(val, plr);
                galaxy[pln].knows[plr] = json_object_get_int(tmp);
            }
        }
        else if(strcmp(key, "link") == 0) {
            arraylen = json_object_array_length(val);
            json_object *tmp;
            for (int lnum = 0; lnum < arraylen; lnum++) {
                tmp = json_object_array_get_idx(val, lnum);
                galaxy[pln].link[lnum] = json_object_get_int(tmp);
            }
        }
        else {
            fprintf(stderr, "Unknown planet key %s", key);
        }
    }
}

/*****************************************************************************/
void LoadGalaxy(json_object *jso)
/*****************************************************************************/
{
    json_object *jvalue;
    enum json_type type;
    int arraylen = json_object_array_length(jso);

    for (int pln = 0; pln < arraylen; pln++) {
        jvalue = json_object_array_get_idx(jso, pln);
        type = json_object_get_type(jvalue);
        LoadPlanet(pln, jvalue);
    }
}

/*****************************************************************************/
void LoadAlliance(json_object *jso)
/*****************************************************************************/
{
    json_object *jval1, *jval2;
    int arraylen1, arraylen2;

    arraylen1 = json_object_array_length(jso);
    for (int plra = 0; plra < arraylen1; plra++) {
        jval1 = json_object_array_get_idx(jso, plra);
        arraylen2 = json_object_array_length(jval1);
        for (int plrb = 0; plrb < arraylen2; plrb++) {
            jval2 = json_object_array_get_idx(jval1, plrb);
            alliance[plra][plrb] = json_object_get_int(jval2);
        }
    }
}

/*****************************************************************************/
void LoadGamedetEarth(json_object *jso)
/*****************************************************************************/
{
    json_object *jvalue;

    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "amnesty") == 0) {
            gamedet.earth.amnesty = json_object_get_int(val);
        }
        else if(strcmp(key, "earthmult") == 0) {
            gamedet.earth.earthmult = json_object_get_int(val);
        }
        else if(strcmp(key, "fbid") == 0) {
            gamedet.earth.fbid = json_object_get_int(val);
        }
        else if(strcmp(key, "cbid") == 0) {
            gamedet.earth.cbid = json_object_get_int(val);
        }
        else if(strcmp(key, "sbid") == 0) {
            gamedet.earth.sbid = json_object_get_int(val);
        }
        else if(strcmp(key, "tbid") == 0) {
            gamedet.earth.tbid = json_object_get_int(val);
        }
        else if(strcmp(key, "ind") == 0) {
            gamedet.earth.ind = json_object_get_int(val);
        }
        else if(strcmp(key, "ore") == 0) {
            int arraylen = json_object_array_length(val);
            for (int rtype = 0; rtype < arraylen; rtype++) {
                jvalue = json_object_array_get_idx(val, rtype);
                gamedet.earth.ore[rtype] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "mine") == 0) {
            int arraylen = json_object_array_length(val);
            for (int rtype = 0; rtype < arraylen; rtype++) {
                jvalue = json_object_array_get_idx(val, rtype);
                gamedet.earth.mine[rtype] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "pdu") == 0) {
            gamedet.earth.pdu = json_object_get_int(val);
        }
        else if(strcmp(key, "spacemine") == 0) {
            gamedet.earth.spacemine = json_object_get_int(val);
        }
        else if(strcmp(key, "deployed") == 0) {
            gamedet.earth.deployed = json_object_get_int(val);
        }
        else if(strcmp(key, "flag") == 0) {
            gamedet.earth.flag = json_object_get_int(val);
        }
        else {
            fprintf(stderr, "Unhandled gamedet earth key %s\n", key);
        }
    }
}


/*****************************************************************************/
void LoadGamedetHome(json_object *jso)
/*****************************************************************************/
{
    json_object *jvalue;

    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "ind") == 0) {
            gamedet.home.ind = json_object_get_int(val);
        }
        else if(strcmp(key, "ore") == 0) {
            int arraylen = json_object_array_length(val);
            for (int rtype = 0; rtype < arraylen; rtype++) {
                jvalue = json_object_array_get_idx(val, rtype);
                gamedet.home.ore[rtype] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "mine") == 0) {
            int arraylen = json_object_array_length(val);
            for (int rtype = 0; rtype < arraylen; rtype++) {
                jvalue = json_object_array_get_idx(val, rtype);
                gamedet.home.mine[rtype] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "pdu") == 0) {
            gamedet.home.pdu = json_object_get_int(val);
        }
        else if(strcmp(key, "spacemine") == 0) {
            gamedet.home.spacemine = json_object_get_int(val);
        }
        else if(strcmp(key, "deployed") == 0) {
            gamedet.home.deployed = json_object_get_int(val);
        }
        else {
            fprintf(stderr, "Unhandled gamedet home key %s\n", key);
        }
    }
}

/*****************************************************************************/
void LoadGamedetGal(json_object *jso)
/*****************************************************************************/
{
    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "nomine") == 0) {
            gamedet.gal.nomine = json_object_get_int(val);
        }
        else if(strcmp(key, "extramine") == 0) {
            gamedet.gal.extramine = json_object_get_int(val);
        }
        else if(strcmp(key, "extraore") == 0) {
            gamedet.gal.extraore = json_object_get_int(val);
        }
        else if(strcmp(key, "hasind") == 0) {
            gamedet.gal.hasind = json_object_get_int(val);
        }
        else if(strcmp(key, "haspdu") == 0) {
            gamedet.gal.haspdu = json_object_get_int(val);
        }
        else {
            fprintf(stderr, "Unhandled gamedet gal key %s\n", key);
        }
    }
}

/*****************************************************************************/
void LoadGamedetShip(json_object *jso)
/*****************************************************************************/
{
    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "num") == 0) {
            gamedet.ship.num = json_object_get_int(val);
        }
        else if(strcmp(key, "fight") == 0) {
            gamedet.ship.fight = json_object_get_int(val);
        }
        else if(strcmp(key, "cargo") == 0) {
            gamedet.ship.cargo = json_object_get_int(val);
        }
        else if(strcmp(key, "shield") == 0) {
            gamedet.ship.shield = json_object_get_int(val);
        }
        else if(strcmp(key, "tractor") == 0) {
            gamedet.ship.tractor = json_object_get_int(val);
        }
        else if(strcmp(key, "eff") == 0) {
            gamedet.ship.eff = json_object_get_int(val);
        }
        else {
            fprintf(stderr, "Unhandled gamedet ship key %s\n", key);
        }
    }
}

/*****************************************************************************/
void LoadGamedetShip2(json_object *jso)
/*****************************************************************************/
{
    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "num") == 0) {
            gamedet.ship2.num = json_object_get_int(val);
        }
        else if(strcmp(key, "fight") == 0) {
            gamedet.ship2.fight = json_object_get_int(val);
        }
        else if(strcmp(key, "cargo") == 0) {
            gamedet.ship2.cargo = json_object_get_int(val);
        }
        else if(strcmp(key, "shield") == 0) {
            gamedet.ship2.shield = json_object_get_int(val);
        }
        else if(strcmp(key, "tractor") == 0) {
            gamedet.ship2.tractor = json_object_get_int(val);
        }
        else if(strcmp(key, "eff") == 0) {
            gamedet.ship2.eff = json_object_get_int(val);
        }
        else {
            fprintf(stderr, "Unhandled gamedet ship2 key %s\n", key);
        }
    }
}

/*****************************************************************************/
void LoadGamedet(json_object *jso)
/*****************************************************************************/
{
    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "winning") == 0) {
            gamedet.winning = json_object_get_int(val);
        }
        else if(strcmp(key, "score") == 0) {
            gamedet.score = json_object_get_int(val);
        }
        else if(strcmp(key, "planets") == 0) {
            gamedet.planets = json_object_get_int(val);
        }
        else if(strcmp(key, "home") == 0) {
            LoadGamedetHome(val);
        }
        else if(strcmp(key, "earth") == 0) {
            LoadGamedetEarth(val);
        }
        else if(strcmp(key, "gal") == 0) {
            LoadGamedetGal(val);
        }
        else if(strcmp(key, "ship") == 0) {
            LoadGamedetShip(val);
        }
        else if(strcmp(key, "ship2") == 0) {
            LoadGamedetShip2(val);
        }
        else if(strcmp(key, "income") == 0) {
            gamedet.income = json_object_get_int(val);
        }
        else if(strcmp(key, "credits") == 0) {
            gamedet.credits = json_object_get_int(val);
        }
        else if(strcmp(key, "turn") == 0) {
            gamedet.turn = json_object_get_int(val);
        }
        else {
            fprintf(stderr, "Unhandled gamedet key %s\n", key);
        }
    }
}

/*****************************************************************************/
void LoadFleet(json_object *jso)
/*****************************************************************************/
{
    json_object *jvalue;
    int arraylen = json_object_array_length(jso);

    for (int shp = 0; shp < arraylen; shp++) {
        jvalue = json_object_array_get_idx(jso, shp);
        json_object_object_foreach(jvalue, key, val) {
            if(strcmp(key, "name") == 0) {
                strncpy(fleet[shp].name, json_object_get_string(val), NAMESIZ);
            }
            else if(strcmp(key, "owner") == 0) {
                fleet[shp].owner = json_object_get_int(val);
            }
            else if(strcmp(key, "fight") == 0) {
                fleet[shp].fight = json_object_get_int(val);
            }
            else if(strcmp(key, "cargo") == 0) {
                fleet[shp].cargo = json_object_get_int(val);
            }
            else if(strcmp(key, "shield") == 0) {
                fleet[shp].shield = json_object_get_int(val);
            }
            else if(strcmp(key, "tractor") == 0) {
                fleet[shp].tractor = json_object_get_int(val);
            }
            else if(strcmp(key, "type") == 0) {
                fleet[shp].type = json_object_get_int(val);
            }
            else if(strcmp(key, "ore") == 0) {
                int arraylen = json_object_array_length(val);
                for (int rtype = 0; rtype < arraylen; rtype++) {
                    json_object *tmp = json_object_array_get_idx(val, rtype);
                    fleet[shp].ore[rtype] = json_object_get_int(tmp);
                }
            }
            else if(strcmp(key, "ind") == 0) {
                fleet[shp].ind = json_object_get_int(val);
            }
            else if(strcmp(key, "mines") == 0) {
                fleet[shp].mines = json_object_get_int(val);
            }
            else if(strcmp(key, "pdu") == 0) {
                fleet[shp].pdu = json_object_get_int(val);
            }
            else if(strcmp(key, "cargleft") == 0) {
                fleet[shp].cargleft = json_object_get_int(val);
            }
            else if(strcmp(key, "hits") == 0) {
                fleet[shp].hits = json_object_get_int(val);
            }
            else if(strcmp(key, "engage") == 0) {
                fleet[shp].engage = json_object_get_int(val);
            }
            else if(strcmp(key, "moved") == 0) {
                fleet[shp].moved = json_object_get_int(val);
            }
            else if(strcmp(key, "planet") == 0) {
                fleet[shp].planet = json_object_get_int(val);
            }
            else if(strcmp(key, "efficiency") == 0) {
                fleet[shp].efficiency = json_object_get_int(val);
            }
            else if(strcmp(key, "spacemines") == 0) {
                fleet[shp].spacemines = json_object_get_int(val);
            }
            else if(strcmp(key, "figleft") == 0) {
                fleet[shp].figleft = json_object_get_int(val);
            }
            else if(strcmp(key, "stndord") == 0) {
                strncpy(fleet[shp].stndord, json_object_get_string(val), 10);
            }
            else if(strcmp(key, "pduhits") == 0) {
                fleet[shp].pduhits = json_object_get_int(val);
            }
            else {
                fprintf(stderr, "Unhandled fleet key %s\n", key);
            }
        }
    }
}

/*****************************************************************************/
int ReadGalflt(void)
/*****************************************************************************/
{
    char fname[BUFSIZ];
    json_object *jso = json_object_new_object();
    json_object *jvalue;

    FilePath("galfile.json", fname);
    jso = json_object_from_file(fname);

    json_object_object_foreach(jso, key, val) {
        if(strcmp(key, "name") == 0) {
            int arraylen = json_object_array_length(val);
            for (int i = 0; i < arraylen; i++) {
                jvalue = json_object_array_get_idx(val, i);
                strncpy(name[i], json_object_get_string(jvalue), NAMESIZ);
            }
        }
        else if(strcmp(key, "galaxy") == 0) {
            LoadGalaxy(val);
        }
        else if(strcmp(key, "fleet") == 0) {
            LoadFleet(val);
        }
        else if(strcmp(key, "shiptr") == 0) {
            shiptr = json_object_get_int(val);
        }
        else if(strcmp(key, "gm") == 0) {
            gm = json_object_get_int(val);
        }
        else if(strcmp(key, "turn") == 0) {
            turn = json_object_get_int(val);
        }
        else if(strcmp(key, "score") == 0) {
            int arraylen = json_object_array_length(val);
            for (int i = 0; i < arraylen; i++) {
                jvalue = json_object_array_get_idx(val, i);
                score[i] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "price") == 0) {
            int arraylen = json_object_array_length(val);
            for (int i = 0; i < arraylen; i++) {
                jvalue = json_object_array_get_idx(val, i);
                price[i] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "alliance") == 0) {
            LoadAlliance(val);
        }
        else if(strcmp(key, "ecredit") == 0) {
            int arraylen = json_object_array_length(val);
            for (int i = 0; i < arraylen; i++) {
                jvalue = json_object_array_get_idx(val, i);
                ecredit[i] = json_object_get_int(jvalue);
            }
        }
        else if(strcmp(key, "gamedet") == 0) {
            LoadGamedet(val);
        }
        else if(strcmp(key, "desturn") == 0) {
            int arraylen = json_object_array_length(val);
            for (int i = 0; i < arraylen; i++) {
                jvalue = json_object_array_get_idx(val, i);
                desturn[i] = json_object_get_int(jvalue);
            }
        }
        else {
            fprintf(stderr, "Unhandled key %s\n", key);
            return(-1);
        }
    }
    return(0);
}

/*****************************************************************************/
json_object * JsonGame()
/*****************************************************************************/
{
    json_object *jso = json_object_new_object();
    // Winning
    json_object *jwinning = json_object_new_int(gamedet.winning);
    json_object_object_add(jso, "winning", jwinning);

    // Score
    json_object *jscore = json_object_new_int(gamedet.score);
    json_object_object_add(jso, "score", jscore);

    // Planets
    json_object *jplanets = json_object_new_int(gamedet.planets);
    json_object_object_add(jso, "planets", jplanets);

    // Home
    json_object *jhome = json_object_new_object();

    // ind
    json_object *jind = json_object_new_int(gamedet.home.ind);
    json_object_object_add(jhome, "ind", jind);

    // Ore[10]
    json_object *jore_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *jore = json_object_new_int(gamedet.home.ore[rtype]);
        json_object_array_add(jore_array, jore);
    }
    json_object_object_add(jhome, "ore", jore_array);

    // Mine[10]
    json_object *jmine_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *jmine = json_object_new_int(gamedet.home.mine[rtype]);
        json_object_array_add(jmine_array, jmine);
    }
    json_object_object_add(jhome, "mine", jmine_array);

    // PDU
    json_object *jpdu = json_object_new_int(gamedet.home.pdu);
    json_object_object_add(jhome, "pdu", jpdu);

    // spacemine
    json_object *jspacemine = json_object_new_int(gamedet.home.spacemine);
    json_object_object_add(jhome, "spacemine", jspacemine);

    // deployed
    json_object *jdeployed = json_object_new_int(gamedet.home.deployed);
    json_object_object_add(jhome, "deployed", jdeployed);

    json_object_object_add(jso, "home", jhome);

    // Earth
    json_object *jearth = json_object_new_object();

    // Amnesty
    json_object *je_amnesty = json_object_new_int(gamedet.earth.amnesty);
    json_object_object_add(jearth, "amnesty", je_amnesty);

    // earthmult
    json_object *je_earthmult = json_object_new_int(gamedet.earth.earthmult);
    json_object_object_add(jearth, "earthmult", je_earthmult);

    // fbid
    json_object *je_fbid = json_object_new_int(gamedet.earth.fbid);
    json_object_object_add(jearth, "fbid", je_fbid);

    // cbid
    json_object *je_cbid = json_object_new_int(gamedet.earth.cbid);
    json_object_object_add(jearth, "cbid", je_cbid);

    // sbid
    json_object *je_sbid = json_object_new_int(gamedet.earth.sbid);
    json_object_object_add(jearth, "sbid", je_sbid);

    // tbid
    json_object *je_tbid = json_object_new_int(gamedet.earth.tbid);
    json_object_object_add(jearth, "tbid", je_tbid);

    // ind
    json_object *je_ind = json_object_new_int(gamedet.earth.ind);
    json_object_object_add(jearth, "ind", je_ind);

    // Ore[10]
    json_object *je_ore_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *je_ore = json_object_new_int(gamedet.earth.ore[rtype]);
        json_object_array_add(je_ore_array, je_ore);
    }
    json_object_object_add(jearth, "ore", je_ore_array);

    // Mine[10]
    json_object *je_mine_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *je_mine = json_object_new_int(gamedet.earth.mine[rtype]);
        json_object_array_add(je_mine_array, je_mine);
    }
    json_object_object_add(jearth, "mine", je_mine_array);

    // pdu
    json_object *je_pdu = json_object_new_int(gamedet.earth.pdu);
    json_object_object_add(jearth, "pdu", je_pdu);

    // spacemine
    json_object *je_spacemine = json_object_new_int(gamedet.earth.spacemine);
    json_object_object_add(jearth, "spacemine", je_spacemine);

    // deployed
    json_object *je_deployed = json_object_new_int(gamedet.earth.deployed);
    json_object_object_add(jearth, "deployed", je_deployed);

    // flag
    json_object *je_flag = json_object_new_int(gamedet.earth.flag);
    json_object_object_add(jearth, "flag", je_flag);

    json_object_object_add(jso, "earth", jearth);

    // gal
    json_object *jgal = json_object_new_object();

    // nomine
    json_object *jg_nomine = json_object_new_int(gamedet.gal.nomine);
    json_object_object_add(jgal, "nomine", jg_nomine);

    // extramine
    json_object *jg_extramine = json_object_new_int(gamedet.gal.extramine);
    json_object_object_add(jgal, "extramine", jg_extramine);

    // extraore
    json_object *jg_extraore = json_object_new_int(gamedet.gal.extraore);
    json_object_object_add(jgal, "extraore", jg_extraore);

    // hasind
    json_object *jg_hasind = json_object_new_int(gamedet.gal.hasind);
    json_object_object_add(jgal, "hasind", jg_hasind);

    // haspdu
    json_object *jg_haspdu = json_object_new_int(gamedet.gal.haspdu);
    json_object_object_add(jgal, "haspdu", jg_haspdu);

    json_object_object_add(jso, "gal", jgal);

    // ship
    json_object *jship = json_object_new_object();

    //num
    json_object *js_num = json_object_new_int(gamedet.ship.num);
    json_object_object_add(jship, "num", js_num);

    //fight
    json_object *js_fight = json_object_new_int(gamedet.ship.fight);
    json_object_object_add(jship, "fight", js_fight);

    //cargo
    json_object *js_cargo = json_object_new_int(gamedet.ship.cargo);
    json_object_object_add(jship, "cargo", js_cargo);

    //shield
    json_object *js_shield = json_object_new_int(gamedet.ship.shield);
    json_object_object_add(jship, "shield", js_shield);

    //tractor
    json_object *js_tractor = json_object_new_int(gamedet.ship.tractor);
    json_object_object_add(jship, "tractor", js_tractor);

    //eff
    json_object *js_eff = json_object_new_int(gamedet.ship.eff);
    json_object_object_add(jship, "eff", js_eff);

    json_object_object_add(jso, "ship", jship);

    // ship2
    json_object *jship2 = json_object_new_object();
    //num
    json_object *js2_num = json_object_new_int(gamedet.ship2.num);
    json_object_object_add(jship2, "num", js2_num);

    //fight
    json_object *js2_fight = json_object_new_int(gamedet.ship2.fight);
    json_object_object_add(jship2, "fight", js2_fight);

    //cargo
    json_object *js2_cargo = json_object_new_int(gamedet.ship2.cargo);
    json_object_object_add(jship2, "cargo", js2_cargo);

    //shield
    json_object *js2_shield = json_object_new_int(gamedet.ship2.shield);
    json_object_object_add(jship2, "shield", js2_shield);

    //tractor
    json_object *js2_tractor = json_object_new_int(gamedet.ship2.tractor);
    json_object_object_add(jship2, "tractor", js2_tractor);

    //eff
    json_object *js2_eff = json_object_new_int(gamedet.ship2.eff);
    json_object_object_add(jship2, "eff", js2_eff);
    json_object_object_add(jso, "ship2", jship2);

    // income
    json_object *jincome = json_object_new_int(gamedet.income);
    json_object_object_add(jso, "income", jincome);

    // credits
    json_object *jcredits = json_object_new_int(gamedet.credits);
    json_object_object_add(jso, "credits", jcredits);

    // turn
    json_object *jturn = json_object_new_int(gamedet.turn);
    json_object_object_add(jso, "turn", jturn);

    return jso;
}

/*****************************************************************************/
json_object * JsonShip(int shp)
/*****************************************************************************/
{
    json_object *jso = json_object_new_object();

    // Name
    json_object *jname = json_object_new_string(fleet[shp].name);
    json_object_object_add(jso, "name", jname);

    // Owner
    json_object *jowner = json_object_new_int(fleet[shp].owner);
    json_object_object_add(jso, "owner", jowner);

    // Fight
    json_object *jfight = json_object_new_int(fleet[shp].fight);
    json_object_object_add(jso, "fight", jfight);

    // Cargo
    json_object *jcargo = json_object_new_int(fleet[shp].cargo);
    json_object_object_add(jso, "cargo", jcargo);

    // Shield
    json_object *jshield = json_object_new_int(fleet[shp].shield);
    json_object_object_add(jso, "shield", jshield);

    // Tractor
    json_object *jtractor = json_object_new_int(fleet[shp].tractor);
    json_object_object_add(jso, "tractor", jtractor);

    // Type
    json_object *jtype = json_object_new_int(fleet[shp].type);
    json_object_object_add(jso, "type", jtype);

    // Ore[10]
    json_object *jore_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *jore = json_object_new_int(fleet[shp].ore[rtype]);
        json_object_array_add(jore_array, jore);
    }
    json_object_object_add(jso, "ore", jore_array);

    // ind
    json_object *jind = json_object_new_int(fleet[shp].ind);
    json_object_object_add(jso, "ind", jind);

    // mines
    json_object *jindleft = json_object_new_int(fleet[shp].mines);
    json_object_object_add(jso, "mines", jindleft);

    // pdu
    json_object *jpdu = json_object_new_int(fleet[shp].pdu);
    json_object_object_add(jso, "pdu", jpdu);

    // cargleft
    json_object *jcargleft = json_object_new_int(fleet[shp].cargleft);
    json_object_object_add(jso, "cargleft", jcargleft);

    // hits
    json_object *jhits = json_object_new_int(fleet[shp].hits);
    json_object_object_add(jso, "hits", jhits);

    // engage
    json_object *jengage = json_object_new_int(fleet[shp].engage);
    json_object_object_add(jso, "engage", jengage);

    // moved
    json_object *jmoved = json_object_new_int(fleet[shp].moved);
    json_object_object_add(jso, "moved", jmoved);

    // planet
    json_object *jplanet = json_object_new_int(fleet[shp].planet);
    json_object_object_add(jso, "planet", jplanet);

    // efficiency
    json_object *jefficiency = json_object_new_int(fleet[shp].efficiency);
    json_object_object_add(jso, "efficiency", jefficiency);

    // spacemines
    json_object *jspacemines = json_object_new_int(fleet[shp].spacemines);
    json_object_object_add(jso, "spacemines", jspacemines);

    // figleft
    json_object *jfigleft = json_object_new_int(fleet[shp].figleft);
    json_object_object_add(jso, "figleft", jfigleft);

    // stndord
    json_object *jstndord = json_object_new_string(fleet[shp].stndord);
    json_object_object_add(jso, "stndord", jstndord);

    // pduhits
    json_object *jpduhits = json_object_new_int(fleet[shp].pduhits);
    json_object_object_add(jso, "pduhits", jpduhits);

    return(jso);
}

/*****************************************************************************/
json_object * JsonPlanet(int plan)
/*****************************************************************************/
{
    json_object *jso = json_object_new_object();

    // Name
    json_object *jname = json_object_new_string(galaxy[plan].name);
    json_object_object_add(jso, "name", jname);

    // Owner
    json_object *jowner = json_object_new_int(galaxy[plan].owner);
    json_object_object_add(jso, "owner", jowner);

    // Ore[10]
    json_object *jore_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *jore = json_object_new_int(galaxy[plan].ore[rtype]);
        json_object_array_add(jore_array, jore);
    }
    json_object_object_add(jso, "ore", jore_array);

    // Mine[10]
    json_object *jmine_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *jmine = json_object_new_int(galaxy[plan].mine[rtype]);
        json_object_array_add(jmine_array, jmine);
    }
    json_object_object_add(jso, "mine", jmine_array);

    // ind
    json_object *jind = json_object_new_int(galaxy[plan].ind);
    json_object_object_add(jso, "ind", jind);

    // indleft
    json_object *jindleft = json_object_new_int(galaxy[plan].indleft);
    json_object_object_add(jso, "indleft", jindleft);

    // pdu
    json_object *jpdu = json_object_new_int(galaxy[plan].pdu);
    json_object_object_add(jso, "pdu", jpdu);

    // link[4]
    json_object *jlink_array = json_object_new_array();
    for (int lnum = 0; lnum < 4; lnum++) {
        json_object *jlink = json_object_new_int(galaxy[plan].link[lnum]);
        json_object_array_add(jlink_array, jlink);
    }
    json_object_object_add(jso, "link", jlink_array);

    // numlinks
    json_object *jnumlinks = json_object_new_int(galaxy[plan].numlinks);
    json_object_object_add(jso, "numlinks", jnumlinks);

    // income
    json_object *jincome = json_object_new_int(galaxy[plan].income);
    json_object_object_add(jso, "income", jincome);

    // spec
    json_object *jspec = json_object_new_int(galaxy[plan].spec);
    json_object_object_add(jso, "spec", jspec);

    // scanned
    json_object *jscanned = json_object_new_int(galaxy[plan].scanned);
    json_object_object_add(jso, "scanned", jscanned);

    // spacemine
    json_object *jspacemine = json_object_new_int(galaxy[plan].spacemine);
    json_object_object_add(jso, "spacemine", jspacemine);

    // deployed
    json_object *jdeployed = json_object_new_int(galaxy[plan].deployed);
    json_object_object_add(jso, "deployed", jdeployed);

    // pduleft
    json_object *jpduleft = json_object_new_int(galaxy[plan].pduleft);
    json_object_object_add(jso, "pduleft", jpduleft);

    // stndord
    json_object *jstndord = json_object_new_string(galaxy[plan].stndord);
    json_object_object_add(jso, "stndord", jstndord);

    // knows[10]
    json_object *jknows_array = json_object_new_array();
    for (int plr = 0; plr <= NUMPLAYERS; plr++) {
        json_object *jknows = json_object_new_int(galaxy[plan].knows[plr]);
        json_object_array_add(jknows_array, jknows);
    }
    json_object_object_add(jso, "knows", jknows_array);


    return(jso);
}

/*****************************************************************************/
void WriteGalflt(void)
/*****************************************************************************/
/* Write the whole data structure       */
{
    char fname[BUFSIZ];
    json_object *jso = json_object_new_object();

    // Name
    json_object *jname = json_object_new_array();
    for (int nmctr = 0; nmctr < 10; nmctr++) {
        json_object *jstring = json_object_new_string(name[nmctr]);
        json_object_array_add(jname, jstring);
    }
    json_object_object_add(jso, "name", jname);

    // Galaxy
    json_object *jgalaxy = json_object_new_array();
    for (int plctr = 0; plctr < NUMPLANETS; plctr++) {
        json_object *jplanet = JsonPlanet(plctr);
        json_object_array_add(jgalaxy, jplanet);
    }
    json_object_object_add(jso, "galaxy", jgalaxy);

    // Fleet
    json_object *jfleet = json_object_new_array();
    for (int plctr = 0; plctr < NUMSHIPS; plctr++) {
        json_object *jship = JsonShip(plctr);
        json_object_array_add(jfleet, jship);
    }
    json_object_object_add(jso, "fleet", jfleet);

    // Shiptr
    json_object *jshiptr = json_object_new_int(shiptr);
    json_object_object_add(jso, "shiptr", jshiptr);

    // gm
    json_object *jgm = json_object_new_int(gm);
    json_object_object_add(jso, "gm", jgm);

    // turn
    json_object *jturn = json_object_new_int(turn);
    json_object_object_add(jso, "turn", jturn);

    // score
    json_object *jscore_array = json_object_new_array();
    for (int plr = 0; plr <= NUMPLAYERS; plr++) {
        json_object *jscore = json_object_new_int(score[plr]);
        json_object_array_add(jscore_array, jscore);
    }
    json_object_object_add(jso, "score", jscore_array);
    
    // price
    json_object *jprice_array = json_object_new_array();
    for (int rtype = 0; rtype < 10; rtype++) {
        json_object *jprice = json_object_new_int(price[rtype]);
        json_object_array_add(jprice_array, jprice);
    }
    json_object_object_add(jso, "price", jprice_array);

    // alliance
    json_object *jalliance_array = json_object_new_array();
    for (int splr = 0; splr <= NUMPLAYERS; splr++) {
        json_object *tmp_array = json_object_new_array();
        for (int dplr = 0; dplr <= NUMPLAYERS; dplr++) {
            json_object *jall = json_object_new_int(alliance[splr][dplr]);
            json_object_array_add(tmp_array, jall);
        }
        json_object_array_add(jalliance_array, tmp_array);
    }
    json_object_object_add(jso, "alliance", jalliance_array);

    // ecredit
    json_object *jecredit_array = json_object_new_array();
    for (int plr = 0; plr <= NUMPLAYERS; plr++) {
        json_object *jecredit = json_object_new_int(ecredit[plr]);
        json_object_array_add(jecredit_array, jecredit);
    }
    json_object_object_add(jso, "ecredit", jecredit_array);

    // gamedet
    json_object *jgamedet = JsonGame();
    json_object_object_add(jso, "gamedet", jgamedet);

    // desturn
    json_object *jdesturn_array = json_object_new_array();
    for (int plr = 0; plr <= NUMPLAYERS; plr++) {
        json_object *jdesturn = json_object_new_int(desturn[plr]);
        json_object_array_add(jdesturn_array, jdesturn);
    }
    json_object_object_add(jso, "desturn", jdesturn_array);

    FilePath("galfile.json", fname);
    jso = json_object_from_file(fname);
    int rc = json_object_to_file(fname, jso);
    if (rc < 0) {
        fprintf(stderr, "Error: %d\n", rc);
    }
}
