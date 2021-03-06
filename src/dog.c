/*	SCCS Id: @(#)dog.c	3.4	2002/09/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"
#include "emin.h"
#include "epri.h"

#ifdef OVLB

STATIC_DCL int pet_type(void);

void
initedog(mtmp)
register struct monst *mtmp;
{
	mtmp->mtame = is_domestic(mtmp->data) ? 10 : 5;
	mtmp->mpeaceful = 1;
	mtmp->mavenge = 0;
	set_malign(mtmp); /* recalc alignment now that it's tamed */
	mtmp->mleashed = 0;
	mtmp->meating = 0;
	EDOG(mtmp)->droptime = 0;
	EDOG(mtmp)->dropdist = 10000;
	EDOG(mtmp)->apport = 10;
	EDOG(mtmp)->whistletime = 0;
	EDOG(mtmp)->hungrytime = 1000 + monstermoves;
	EDOG(mtmp)->ogoal.x = -1;	/* force error if used before set */
	EDOG(mtmp)->ogoal.y = -1;
	EDOG(mtmp)->abuse = 0;
	EDOG(mtmp)->revivals = 0;
	EDOG(mtmp)->mhpmax_penalty = 0;
	EDOG(mtmp)->killed_by_u = 0;

	/* if the pet has a weapon, we want it to wield, not drop it! --Amy */
	if (mtmp->mtame && attacktype(mtmp->data, AT_WEAP)) {
		mtmp->weapon_check = NEED_HTH_WEAPON;
		(void) mon_wield_item(mtmp);
	}
}

STATIC_OVL int
pet_type()
{
	if (urole.petnum != NON_PM)
	    return (urole.petnum);
	else if (preferred_pet == 'c')
	    return (PM_KITTEN);
	else if (preferred_pet == 'd')
	    return (PM_LITTLE_DOG);
	else if (Role_if(PM_PIRATE))
		return (rn2(2) ? PM_AIRBORNE_PARROT : PM_MONKEY);
	else if (Role_if(PM_GOFF))
		return (rn2(2) ? PM_NINJA_BOY : PM_NINJA_GIRL);
	else if (Role_if(PM_KORSAIR))
		switch (rnd(5)) {   
		case 1: return (PM_LITTLE_DOG);
		case 2: return (PM_KITTEN);
		case 3: return (PM_SEWER_RAT);
		case 4: return (PM_MONKEY);
		case 5: return (PM_BABY_CROCODILE);
		}
	else if (Role_if(PM_LUNATIC))
		switch (rnd(7)) {   
		case 1: return (PM_WOLF);
		case 2: return (PM_JACKAL);
		case 3: return (PM_SEWER_RAT);
		case 4: return (PM_PANTHER);
		case 5: return (PM_TIGER);
		case 6: return (PM_VIPER);
		case 7: return (PM_GIANT_SPIDER);
		}
	else if (Role_if(PM_CHEVALIER))
		switch (rnd(11)) {   
		case 1: return (PM_BABY_YELLOW_DRAGON);
		case 2: return (PM_BABY_GREEN_DRAGON);
		case 3: return (PM_BABY_BLUE_DRAGON);
		case 4: return (PM_BABY_BLACK_DRAGON);
		case 5: return (PM_BABY_ORANGE_DRAGON);
		case 6: return (PM_BABY_WHITE_DRAGON);
		case 7: return (PM_BABY_RED_DRAGON);
		case 8: return (PM_BABY_DEEP_DRAGON);
		case 9: return (PM_BABY_SHIMMERING_DRAGON);
		case 10: return (PM_BABY_SILVER_DRAGON);
		case 11: return (PM_BABY_GRAY_DRAGON);
		}
	else if (Role_if(PM_MEDIUM)) {
		switch (u.ualign.type) {
			case A_LAWFUL: return (PM_WHITE_UNICORN_FOAL);
			case A_NEUTRAL: return (PM_GRAY_UNICORN_FOAL);
			case A_CHAOTIC: return (PM_BLACK_UNICORN_FOAL);
			default: return (PM_GRAY_UNICORN_FOAL);
		}
	}
	else if (Role_if(PM_TRANSVESTITE) || Role_if(PM_TRANSSYLVANIAN) )
		return ( !rn2(5) ? PM_ASIAN_GIRL : !rn2(10) ? PM_ESTRELLA_GIRL : rn2(2) ? PM_LITTLE_GIRL : PM_LITTLE_BOY);
	else if (Role_if(PM_TOPMODEL))
		switch (rnd(3)) {   
		case 1: return (PM_DARK_GIRL);
		case 2: return (PM_REDGUARD_GIRL);
		case 3: return (PM_THIEVING_GIRL);
		}
	else if (Role_if(PM_FAILED_EXISTENCE))
		switch (rnd(3)) {   
		case 1: return (PM_DARK_GIRL);
		case 2: return (PM_REDGUARD_GIRL);
		case 3: return (PM_THIEVING_GIRL);
		}
	else if (Role_if(PM_LADIESMAN))
		switch (rnd(3)) {   
		case 1: return (PM_DARK_GIRL);
		case 2: return (PM_REDGUARD_GIRL);
		case 3: return (PM_THIEVING_GIRL);
		}
	else
	    return (rn2(2) ? PM_KITTEN : PM_LITTLE_DOG);
}

struct monst *
make_familiar(otmp,x,y,quietly)
register struct obj *otmp;
xchar x, y;
boolean quietly;
{
	struct permonst *pm;
	struct monst *mtmp = 0;
	int chance, trycnt = 100;

	do {
	    if (otmp) {	/* figurine; otherwise spell */
		int mndx = otmp->corpsenm;
		pm = &mons[mndx];
		/* activating a figurine provides one way to exceed the
		   maximum number of the target critter created--unless
		   it has a special limit (erinys, Nazgul) */
		if ((mvitals[mndx].mvflags & G_EXTINCT) &&
			mbirth_limit(mndx) != MAXMONNO) {
		    if (!quietly)
			/* have just been given "You <do something with>
			   the figurine and it transforms." message */
			pline(Hallucination ? "... into a pile of garbage. Even you know that that's of no use." : "... into a pile of dust.");
		    break;	/* mtmp is null */
		}
	    } else if (!rn2(3)) {
		pm = &mons[pet_type()];
	    } else {
		pm = rndmonst();
		if (!pm) {
		  if (!quietly)
		    There("seems to be nothing available for a familiar.");
		  break;
		}
	    }

	    mtmp = makemon(pm, x, y, MM_EDOG|MM_IGNOREWATER|MM_NOSPECIALS);
	    if (otmp && !mtmp) { /* monster was genocided or square occupied */
	 	if (!quietly)
		   pline_The("figurine writhes and then shatters into pieces!");
		break;
	    }
	} while (!mtmp && --trycnt > 0);

	if (!mtmp) return (struct monst *)0;

	if ((is_waterypool(mtmp->mx, mtmp->my) || is_watertunnel(mtmp->mx, mtmp->my)) && minliquid(mtmp))
		return (struct monst *)0;

	initedog(mtmp);
	mtmp->msleeping = 0;
	if (otmp) { /* figurine; resulting monster might not become a pet */
	    chance = rn2(5);	/* 0==tame, 1==peaceful, 2==hostile */
	    if (chance > 2) chance = otmp->blessed ? 0 : !otmp->cursed ? 1 : 2;
	    if (otmp && otmp->oartifact == ART_GUARANTEED_SPECIAL_PET) chance = 0;
	    /* 0,1,2:  b=60%,20,20; nc=20%,60,20; c=20%,20,60 */
	    if (chance > 0) {
		mtmp->mtame = 0;	/* not tame after all */
		if (chance == 2) { /* hostile (cursed figurine) */
		    if (!quietly)
		       You(Hallucination ? "shiver." : "get a bad feeling about this.");
		    mtmp->mpeaceful = 0;
		    set_malign(mtmp);
		}
	    }
	    /* if figurine has been named, give same name to the monster */
	    if (otmp->onamelth)
		mtmp = christen_monst(mtmp, ONAME(otmp));
	}
	set_malign(mtmp); /* more alignment changes */
	newsym(mtmp->mx, mtmp->my);

	/* must wield weapon immediately since pets will otherwise drop it */
	if (mtmp->mtame && attacktype(mtmp->data, AT_WEAP)) {
		mtmp->weapon_check = NEED_HTH_WEAPON;
		(void) mon_wield_item(mtmp);
	}
	return mtmp;
}


struct monst *
make_helper(mnum,x,y)
	int mnum;
	xchar x, y;
{
	struct permonst *pm;
	struct monst *mtmp = 0;
	int trycnt = 100;

	do {
		pm = &mons[mnum];

		pm->pxlth += sizeof (struct edog);
		mtmp = makemon(pm, x, y, NO_MINVENT|MM_NOSPECIALS);
		pm->pxlth -= sizeof (struct edog);
	} while (!mtmp && --trycnt > 0);

	if(!mtmp) return((struct monst *) 0); /* genocided */

	initedog(mtmp);
	mtmp->msleeping = 0;
	set_malign(mtmp); /* more alignment changes */
	newsym(mtmp->mx, mtmp->my);

	/* must wield weapon immediately since pets will otherwise drop it */
	if (mtmp->mtame && attacktype(mtmp->data, AT_WEAP)) {
		mtmp->weapon_check = NEED_HTH_WEAPON;
		(void) mon_wield_item(mtmp);
	}
	return (mtmp);
}


struct monst *
makedog()
{
	register struct monst *mtmp;
	register struct obj *otmp;
	const char *petname;
	int   pettype, petsym;
	static int petname_used = 0;

	if (!Role_if(PM_KURWA) && (preferred_pet == 'n')) return((struct monst *) 0);

	pettype = pet_type();

	/* In Soviet Russia, people don't like exotic pets. Even interesting ones like Green-elves or animated wedge sandals they despise for some reason, and want them to be "normalized" into being dogs or cats. Of course, for other races there's no such communist normalization, so they will get their actual pets. --Amy */
	if (issoviet) pettype = rn2(2) ? PM_LITTLE_DOG : PM_KITTEN;

	petsym = mons[pettype].mlet;
	if (pettype == PM_WINTER_WOLF_CUB || pettype == PM_WOLF)
		petname = wolfname;
	else if (pettype == PM_GHOUL)
		petname = ghoulname;
	else if (pettype == PM_PONY || pettype == PM_GREEN_NIGHTMARE)
		petname = horsename;
	else if (pettype == PM_SEWER_RAT)
		petname = ratname;
#if 0
	else if (petsym == S_BAT)
		petname = batname;
	else if (petsym == S_SNAKE)
		petname = snakename;
	else if (petsym == S_RODENT)
		petname = ratname;
	else if (pettype == PM_GIANT_BADGER)
		petname = badgername;
	else if (pettype == PM_BABY_RED_DRAGON)
		petname = reddragonname;
	else if (pettype == PM_BABY_WHITE_DRAGON)
		petname = whitedragonname;
#endif
	else if (petsym == S_DOG)
		petname = dogname;
	else
		petname = catname;

	/* default pet names */
	if (!*petname && pettype == PM_LITTLE_DOG) {
	    /* All of these names were for dogs. */
	    if(Role_if(PM_CAVEMAN)) petname = "Slasher";   /* The Warrior */
	    if(Role_if(PM_SAMURAI)) petname = "Hachi";     /* Shibuya Station */
	    if(Role_if(PM_BARBARIAN)) petname = "Idefix";  /* Obelix */
	    if(Role_if(PM_RANGER)) petname = "Sirius";     /* Orion's dog */
	}
	if (!*petname && pettype == PM_SEWER_RAT) {
	    if(Role_if(PM_CONVICT)) petname = "Nicodemus"; /* Rats of NIMH */
    }
	if (pettype == PM_MONKEY) petname = "Ugga-Ugga";
	if (pettype == PM_AIRBORNE_PARROT) petname = "Squawks";
	if (pettype == PM_SPEEDHORSE) petname = "Harley Davidson";
	if (pettype == PM_BABY_CROCODILE) petname = "Snappy"; /* in Germany it would be called Schnappi */
	if (pettype == PM_EEVEE) petname = "HUSI";
	if (pettype == PM_WHITE_UNICORN_FOAL) petname = "Rastafari"; /* creature of peace */
	if (pettype == PM_GRAY_UNICORN_FOAL) petname = "Balance of Neutrality";
	if (pettype == PM_BLACK_UNICORN_FOAL) petname = "Orderly Chaos";

	if (pettype == PM_AGGRESSIVE_LICHEN) petname = "Sessilium";
	if (pettype == PM_PILE_OF_COPPER_COINS) petname = "Counterfeit";

	if (pettype == PM_GRIMER) petname = "BADEB";
	if (pettype == PM_DARK_NIGHTMARE) petname = "Opel Manta";

	if (pettype == PM_VENOM_FUNGUS) petname = "This Is A BioHazard";
	if (pettype == PM_MARTTI_IHRASAARI) petname = "Martti Ihrasaari";
	if (pettype == PM_ELIF) petname = "Elif";
	if (pettype == PM_JENNIFER) petname = "Jennifer";

	if (pettype == PM_KICKBOXING_GIRL) petname = "Marija";

	if (pettype == PM_NINJA_BOY) petname = "Cornelia Fuck"; /* My Immortal fanfic */
	if (pettype == PM_NINJA_GIRL) petname = "Doris Rumbridge";

	if (pettype == PM_DRAGONBALL_KID) petname = "Android Nr. 17"; /* don't know the real name, but it was something like this --Amy */

	if (pettype == PM_GREEN_ELF) petname = "Dray Harp";
	if (pettype == PM_BARD) petname = "Michael Jackson";
	if (pettype == PM_DROW) petname = "Roflth"; /* Lolth */
	if (pettype == PM_OFFICER) petname = "Officer O'Brian";
	if (pettype == PM_SOLDIER) petname = "Lieutenant Surge"; /* Pokemon Yellow */
	if (pettype == PM_VALKYRIE) petname = "Rue";	/* hunger games */
	if (pettype == PM_PLATYPUS) petname = "Donald Duck";
	if (pettype == PM_ORDINATOR) petname = "Andragil";

	if (pettype == PM_FLOATING_EYE) petname = "Gazing Beholder Orb";

	if (pettype == PM_GOODWIFE) petname = "Patchouli Knowledge";

	if (pettype == PM_EVASIVE_SNIPER) petname = "Tingsi Major";

	if (pettype == PM_PRIEST) petname = "Septimus";

	if (pettype == PM_HUMAN_THIEF) petname = "Wiesbek the Thief"; /* dunno where I originally got that name from --Amy */

	if (pettype == PM_VAMPIRE) petname = "Cheater";

	if (pettype == PM_RAVEN) petname = "Nevermore";

	if (pettype == PM_ANTIMATTER_VORTEX) petname = "Expensive Special Effect";

	if (pettype == PM_DARK_GRUE) petname = "ZORK!";
	if (pettype == PM_FRY) petname = "Perry";

	if (pettype == PM_CLEFAIRY) petname = "Thief Pokemon";
	if (pettype == PM_CHAMELEON) petname = "The RNG";
	if (pettype == PM_CHAOS_POLYMORPHER) petname = "Mr. X";

	if (pettype == PM_PANTHER) petname = "Tomcat Karlo";
	if (pettype == PM_TIGER) petname = "Simba";
	if (pettype == PM_VIPER) petname = "Lukas";
	if (pettype == PM_GIANT_SPIDER) petname = "Andreas";
	if (pettype == PM_ZUBAT) petname = "Cliff Racer"; /* Morrowind */
	if (pettype == PM_DANCING_GIRL) petname = "Thaedil"; /* Oblivion, Shivering Esles */
	if (pettype == PM_THALMOR) petname = "Rulindil"; /* Skyrim */
	if (pettype == PM_CRUEL_ABUSER) petname = "Ms. Robinson"; /* 50 Shades of Fucked Up */

	if (pettype == PM_ANIMATED_WEDGE_SANDAL) petname = "Larissa"; /* just a common female first name */
	if (pettype == PM_GOLD_GOLEM) petname = "Midas"; /* the greedy king who unfortunately lost all his gold :( */

	if (pettype == PM_JEDI) petname = "Luke Skywalker";

	if (pettype == PM_LITTLE_GIRL) petname = "Sarah"; /* just a common female first name */
	if (pettype == PM_LITTLE_BOY) petname = "Jonas"; /* just a common male first name */
	if (pettype == PM_ASIAN_GIRL) petname = "Whitney"; /* the Normal-type gym leader in Pokemon Crystal */
	if (pettype == PM_ESTRELLA_GIRL) petname = "Estrella"; /* uncommon female first name */

	if (pettype == PM_DARK_GIRL) petname = "Everella"; /* taken from a fanfic */
	if (pettype == PM_REDGUARD_GIRL) petname = "Jasajeen"; /* taken from a fanfic */
	if (pettype == PM_THIEVING_GIRL) petname = "Esruth"; /* taken from a fanfic */

	if (pettype == PM_SEXY_GIRL) petname = "Natalje";

	if (pettype == PM_ACTIVISTOR) petname = "Helen"; /* yet another common first name */

	if (pettype == PM_BABY_YELLOW_DRAGON || pettype == PM_BABY_GREEN_DRAGON || pettype == PM_BABY_BLUE_DRAGON || pettype == PM_BABY_RED_DRAGON || pettype == PM_BABY_ORANGE_DRAGON || pettype == PM_BABY_WHITE_DRAGON || pettype == PM_BABY_BLACK_DRAGON || pettype == PM_BABY_DEEP_DRAGON || pettype == PM_BABY_SHIMMERING_DRAGON || pettype == PM_BABY_GRAY_DRAGON || pettype == PM_BABY_SILVER_DRAGON) petname = "Odahviing";

	if (petname && !(strcmp(petname, "Glorious Dead") ) ) petname = "Glorious Alive";

	mtmp = makemon(&mons[pettype], u.ux, u.uy, MM_EDOG);

	if(!mtmp) return((struct monst *) 0); /* pets were genocided */

	/* Horses already wear a saddle */
	if ((pettype == PM_PONY || pettype == PM_GREEN_NIGHTMARE || pettype == PM_SPEEDHORSE) && !!(otmp = mksobj(Race_if(PM_INKA) ? INKA_SADDLE : LEATHER_SADDLE, TRUE, FALSE))) {
	    if (mpickobj(mtmp, otmp, TRUE)) {
		impossible("merged saddle?");
		} else {
	    mtmp->misc_worn_check |= W_SADDLE;
	    otmp->dknown = otmp->bknown = otmp->rknown = 1;
	    otmp->owornmask = W_SADDLE;
	    otmp->leashmon = mtmp->m_id;
	    update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
		}
	}

	if (!petname_used++ && *petname)
		mtmp = christen_monst(mtmp, petname);

	initedog(mtmp);
	return(mtmp);
}

/* record `last move time' for all monsters prior to level save so that
   mon_arrive() can catch up for lost time when they're restored later */
void
update_mlstmv()
{
	struct monst *mon;

	/* monst->mlstmv used to be updated every time `monst' actually moved,
	   but that is no longer the case so we just do a blanket assignment */
	for (mon = fmon; mon; mon = mon->nmon)
	    if (!DEADMONSTER(mon)) mon->mlstmv = monstermoves;
}

void
losedogs()
{
	register struct monst *mtmp, *mtmp0 = 0, *mtmp2;

	while ((mtmp = mydogs) != 0) {
		mydogs = mtmp->nmon;
		mon_arrive(mtmp, TRUE);
	}

	for(mtmp = migrating_mons; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if (mtmp->mux == u.uz.dnum && mtmp->muy == u.uz.dlevel) {
		    if(mtmp == migrating_mons)
			migrating_mons = mtmp->nmon;
		    else
			mtmp0->nmon = mtmp->nmon;
		    mon_arrive(mtmp, FALSE);
		} else
		    mtmp0 = mtmp;
	}
}

/* called from resurrect() in addition to losedogs() */
void
mon_arrive(mtmp, with_you)
struct monst *mtmp;
boolean with_you;
{
	struct trap *t;
	xchar xlocale, ylocale, xyloc, xyflags, wander;
	int num_segs;

	mtmp->nmon = fmon;
	fmon = mtmp;
	if (mtmp->isshk)
	    set_residency(mtmp, FALSE);

	num_segs = mtmp->wormno;
	/* baby long worms have no tail so don't use is_longworm() */
	if ((mtmp->data == &mons[PM_LONG_WORM]) &&
#ifdef DCC30_BUG
	    (mtmp->wormno = get_wormno(), mtmp->wormno != 0))
#else
	    (mtmp->wormno = get_wormno()) != 0)
#endif
	{
	    initworm(mtmp, num_segs);
	    /* tail segs are not yet initialized or displayed */
	} else mtmp->wormno = 0;

	/* some monsters might need to do something special upon arrival
	   _after_ the current level has been fully set up; see dochug() */
	mtmp->mstrategy |= STRAT_ARRIVE;

	/* make sure mnexto(rloc_to(set_apparxy())) doesn't use stale data */
	mtmp->mux = u.ux,  mtmp->muy = u.uy;
	xyloc	= mtmp->mtrack[0].x;
	xyflags = mtmp->mtrack[0].y;
	xlocale = mtmp->mtrack[1].x;
	ylocale = mtmp->mtrack[1].y;
	mtmp->mtrack[0].x = mtmp->mtrack[0].y = 0;
	mtmp->mtrack[1].x = mtmp->mtrack[1].y = 0;

	if (mtmp == u.usteed)
	    return;	/* don't place steed on the map */
	if (with_you) {
	    /* When a monster accompanies you, sometimes it will arrive
	       at your intended destination and you'll end up next to
	       that spot.  This code doesn't control the final outcome;
	       goto_level(do.c) decides who ends up at your target spot
	       when there is a monster there too. */
	    if (!MON_AT(u.ux, u.uy) &&
		    !rn2(mtmp->mtame ? 10 : mtmp->mpeaceful ? 5 : 2))
		rloc_to(mtmp, u.ux, u.uy);
	    else
		mnexto(mtmp);
	    return;
	}
	/*
	 * The monster arrived on this level independently of the player.
	 * Its coordinate fields were overloaded for use as flags that
	 * specify its final destination.
	 */

	if (mtmp->mlstmv < monstermoves - 1L) {
	    /* heal monster for time spent in limbo */
	    long nmv = monstermoves - 1L - mtmp->mlstmv;

	    mon_catchup_elapsed_time(mtmp, nmv);
	    mtmp->mlstmv = monstermoves - 1L;

	    /* let monster move a bit on new level (see placement code below) */
	    wander = (xchar) min(nmv, 8);
	} else
	    wander = 0;

	switch (xyloc) {
	 case MIGR_APPROX_XY:	/* {x,y}locale set above */
		break;
	 case MIGR_EXACT_XY:	wander = 0;
		break;
	 case MIGR_NEAR_PLAYER:	xlocale = u.ux,  ylocale = u.uy;
		break;
	 case MIGR_STAIRS_UP:	xlocale = xupstair,  ylocale = yupstair;
		break;
	 case MIGR_STAIRS_DOWN:	xlocale = xdnstair,  ylocale = ydnstair;
		break;
	 case MIGR_LADDER_UP:	xlocale = xupladder,  ylocale = yupladder;
		break;
	 case MIGR_LADDER_DOWN:	xlocale = xdnladder,  ylocale = ydnladder;
		break;
	 case MIGR_SSTAIRS:	xlocale = sstairs.sx,  ylocale = sstairs.sy;
		break;
	 case MIGR_PORTAL:
		if (In_endgame(&u.uz)) {
		    /* there is no arrival portal for endgame levels */
		    /* BUG[?]: for simplicity, this code relies on the fact
		       that we know that the current endgame levels always
		       build upwards and never have any exclusion subregion
		       inside their TELEPORT_REGION settings. */
		    xlocale = rn1(updest.hx - updest.lx + 1, updest.lx);
		    ylocale = rn1(updest.hy - updest.ly + 1, updest.ly);
		    break;
		}
		/* find the arrival portal */
		for (t = ftrap; t; t = t->ntrap)
		    if (t->ttyp == MAGIC_PORTAL) break;
		if (t) {
		    xlocale = t->tx,  ylocale = t->ty;
		    break;
		} else impossible("mon_arrive: no corresponding portal?");
		/*FALLTHRU*/
	 default:
	 case MIGR_RANDOM:	xlocale = ylocale = 0;
		    break;
	    }

	if (xlocale && wander) {
	    /* monster moved a bit; pick a nearby location */
	    /* mnearto() deals w/stone, et al */
	    char *r = in_rooms(xlocale, ylocale, 0);
	    if (r && *r) {
		coord c;
		/* somexy() handles irregular rooms */
		if (somexy(&rooms[*r - ROOMOFFSET], &c))
		    xlocale = c.x,  ylocale = c.y;
		else
		    xlocale = ylocale = 0;
	    } else {		/* not in a room */
		int i, j;
		i = max(1, xlocale - wander);
		j = min(COLNO-1, xlocale + wander);
		xlocale = rn1(j - i, i);
		i = max(0, ylocale - wander);
		j = min(ROWNO-1, ylocale + wander);
		ylocale = rn1(j - i, i);
	    }
	}	/* moved a bit */

	mtmp->mx = 0;	/*(already is 0)*/
	mtmp->my = xyflags;
	if (xlocale)
	    (void) mnearto(mtmp, xlocale, ylocale, FALSE);
	else {
	    if (!rloc(mtmp,TRUE)) {
		/*
		 * Failed to place migrating monster,
		 * probably because the level is full.
		 * Dump the monster's cargo and leave the monster dead.
		 */
	    	struct obj *obj, *corpse;
		while ((obj = mtmp->minvent) != 0) {
		    obj_extract_self(obj);
		    obj_no_longer_held(obj);
		    if (obj->owornmask & W_WEP)
			setmnotwielded(mtmp,obj);
		    obj->owornmask = 0L;
		    if (xlocale && ylocale)
			    place_object(obj, xlocale, ylocale);
		    else {
		    	rloco(obj);
			get_obj_location(obj, &xlocale, &ylocale, 0);
		    }
		}
		corpse = mkcorpstat(CORPSE, (struct monst *)0, mtmp->data,
				xlocale, ylocale, FALSE);
#ifndef GOLDOBJ
		if (mtmp->mgold) {
		    if (xlocale == 0 && ylocale == 0 && corpse) {
			(void) get_obj_location(corpse, &xlocale, &ylocale, 0);
			(void) mkgold(mtmp->mgold, xlocale, ylocale);
		    }
		    mtmp->mgold = 0L;
		}
#endif
		mongone(mtmp);
	    }
	}
}

/* heal monster for time spent elsewhere */
void
mon_catchup_elapsed_time(mtmp, nmv)
struct monst *mtmp;

long nmv;		/* number of moves */
{
	struct edog *edog = EDOG(mtmp);

	int imv = 0;	/* avoid zillions of casts and lint warnings */

#if defined(DEBUG) || defined(BETA)
	if (nmv < 0L) {			/* crash likely... */
	    panic("catchup from future time?");
	    /*NOTREACHED*/
	    return;
	} else if (nmv == 0L) {		/* safe, but should'nt happen */
	    impossible("catchup from now?");
	} else
#endif
	if (nmv >= LARGEST_INT)		/* paranoia */
	    imv = LARGEST_INT - 1;
	else
	    imv = (int)nmv;

	/* might stop being afraid, blind or frozen */
	/* set to 1 and allow final decrement in movemon() */
	if (mtmp->mblinded) {
	    if (imv >= (int) mtmp->mblinded) mtmp->mblinded = 1;
	    else mtmp->mblinded -= imv;
	}
	if (mtmp->mfrozen) {
	    if (imv >= (int) mtmp->mfrozen) mtmp->mfrozen = 1;
	    else mtmp->mfrozen -= imv;
	}
	if (mtmp->mfleetim) {
	    if (imv >= (int) mtmp->mfleetim) mtmp->mfleetim = 1;
	    else mtmp->mfleetim -= imv;
	}

	/* might recover from temporary trouble */
	if (mtmp->mtrapped && rn2(imv + 1) > 40/2) mtmp->mtrapped = 0;
	if (mtmp->mconf    && rn2(imv + 1) > 50/2) mtmp->mconf = 0;
	if (mtmp->mstun    && rn2(imv + 1) > 10/2) mtmp->mstun = 0;

	/* might finish eating or be able to use special ability again */
	if (imv > mtmp->meating) mtmp->meating = 0;
	else mtmp->meating -= imv;
	if (imv > mtmp->mspec_used) mtmp->mspec_used = 0;
	else mtmp->mspec_used -= imv;


		   /*                    
			*      M1_MINDLESS __
			*      M2_UNDEAD     |
			*      M2_WERE       |-- These types will go ferral
			*      M2_DEMON      |
			*      M1_ANIMAL   --
			*/
 
			if ((is_animal(mtmp->data) || mindless(mtmp->data) ||
			    is_demon(mtmp->data)  || is_undead(mtmp->data) || mtmp->egotype_undead ||
			    is_were(mtmp->data)) && (issoviet || ((monstermoves + 5000) > edog->hungrytime)) ) { 
				/* reduce tameness for every 
				 * 150 moves you are away 
				 Amy -- edit so that well-satiated pets can be on another level for much longer */
		/* In Soviet Russia, dogs HATE it if their owners are gone for more than a measly few minutes. They will
		 * assume that their holders are dead, and not recognize them anymore. No matter how well-fed they were.
		 * I'm not sure if my improvements were in SLASHTHEM, but probably not, because if they were, I'd bet actual
		 * money on Soviet reverting it "because the Amy made this change so it must be teh suxx0rz!!!!!111" --Amy */
			/*struct edog *edog = EDOG(mtmp);*/

				if (mtmp->mtame > (nmv / (issoviet ? 150 : 1000))) /* increased by Amy */
					mtmp->mtame -= (nmv / (issoviet ? 150 : 1000));
				else mtmp->mtame = 0;
	}
	/* check to see if it would have died as a pet; if so, go wild instead
	 * of dying the next time we call dog_move()
	 */
	if (mtmp->mtame && !mtmp->isminion &&
			(carnivorous(mtmp->data) || herbivorous(mtmp->data) || metallivorous(mtmp->data) || lithivorous(mtmp->data) || mtmp->egotype_lithivore || mtmp->egotype_metallivore )) {

	    if ((monstermoves > edog->hungrytime + 500 && mtmp->mhp < 3) ||
		    (monstermoves > edog->hungrytime + 750)) {

	/* let's make a pet with high tameness resistant to becoming hostile. Give them a chance to be peaceful instead. */
		if (mtmp->mtame >= 1 && rn2(2) && rn2(mtmp->mtame) )
		mtmp->mtame = 0;
		else mtmp->mtame = mtmp->mpeaceful = 0;

		}
	}

	if (!mtmp->mtame && mtmp->mleashed) {
	    /* leashed monsters should always be with hero, consequently
	       never losing any time to be accounted for later */
	    impossible("catching up for leashed monster?");
	    m_unleash(mtmp, FALSE);
	}

	/* recover lost hit points */
	if (!regenerates(mtmp->data) && (!mtmp->egotype_regeneration) ) imv /= (ishaxor ? 10 : 20);
	if (mtmp->mhp + imv >= mtmp->mhpmax)
	    mtmp->mhp = mtmp->mhpmax;
	else mtmp->mhp += imv;

	/* good riding skill gives extra regeneration to ridden monster --Amy */

	if (!(PlayerCannotUseSkills)) {

	if (P_SKILL(P_RIDING) == P_SKILLED && u.usteed && (mtmp == u.usteed) && !rn2(10) ) {
		if (mtmp->mhp + 1 >= mtmp->mhpmax)
		      mtmp->mhp = mtmp->mhpmax;
		else mtmp->mhp++;
	}
	if (P_SKILL(P_RIDING) == P_EXPERT && u.usteed && (mtmp == u.usteed) && !rn2(5) ) {
		if (mtmp->mhp + 1 >= mtmp->mhpmax)
		      mtmp->mhp = mtmp->mhpmax;
		else mtmp->mhp++;
	}
	if (P_SKILL(P_RIDING) == P_MASTER && u.usteed && (mtmp == u.usteed) && !rn2(3) ) {
		if (mtmp->mhp + 1 >= mtmp->mhpmax)
		      mtmp->mhp = mtmp->mhpmax;
		else mtmp->mhp++;
	}
	if (P_SKILL(P_RIDING) == P_GRAND_MASTER && u.usteed && (mtmp == u.usteed) ) {
		if (mtmp->mhp + 1 >= mtmp->mhpmax)
		      mtmp->mhp = mtmp->mhpmax;
		else mtmp->mhp++;
	}

	if (P_SKILL(P_RIDING) == P_SUPREME_MASTER && u.usteed && (mtmp == u.usteed) ) {
		if (mtmp->mhp + 1 >= mtmp->mhpmax)
		      mtmp->mhp = mtmp->mhpmax;
		else mtmp->mhp++;
	}

	}
}

#endif /* OVLB */
#ifdef OVL2

/* called when you move to another level */
void
keepdogs(pets_only)
boolean pets_only;	/* true for ascension or final escape */
{
	register struct monst *mtmp, *mtmp2;
	register struct obj *obj;
	int num_segs;
	boolean stay_behind;
	extern d_level new_dlevel;	/* in do.c */
	int extraradius = 0;
  	char qbuf[QBUFSZ];
	boolean qbufdefined = 0; /* fail safe */

	if (!(PlayerCannotUseSkills)) {
		switch (P_SKILL(P_PETKEEPING)) {
			default: break;
			case P_BASIC: extraradius = 4; break;
			case P_SKILLED: extraradius = 8; break;
			case P_EXPERT: extraradius = 12; break;
			case P_MASTER: extraradius = 16; break;
			case P_GRAND_MASTER: extraradius = 20; break;
			case P_SUPREME_MASTER: extraradius = 25; break;
		}
	}

	for (mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;
	    if (DEADMONSTER(mtmp)) continue;
	    if (pets_only && !mtmp->mtame) continue;

	    if (mtmp && !program_state.gameover && isok(u.ux, u.uy) && extraradius && mtmp->mtame && levl_follower(mtmp) && (distu(mtmp->mx, mtmp->my) < (4 + extraradius))) {
		sprintf(qbuf, "You can take %s with you. Do it?", noit_mon_nam(mtmp));
		qbufdefined = 1;
	    }

	    if (((monnear(mtmp, u.ux, u.uy) && levl_follower(mtmp))

#ifdef RECORD_ACHIEVE
			/* come on, if you ascend then all tame monsters should ascend with you. --Amy */
			|| (mtmp->mtame && (achieve.ascended))
#endif

			|| (mtmp && !(program_state.gameover) && qbufdefined && isok(u.ux, u.uy) && extraradius && mtmp->mtame && levl_follower(mtmp) && (distu(mtmp->mx, mtmp->my) < (4 + extraradius)) && (yn(qbuf) == 'y') ) ||
			(mtmp == u.usteed) ||
		/* the wiz will level t-port from anywhere to chase
		   the amulet; if you don't have it, will chase you
		   only if in range. -3. */
			(u.uhave.amulet && mtmp->iswiz))
		&& ((!mtmp->msleeping && mtmp->mcanmove)
		    /* eg if level teleport or new trap, steed has no control
		       to avoid following */
		    || (mtmp == u.usteed)
		    )
		/* monster won't follow if it hasn't noticed you yet */
		&& !(mtmp->mstrategy & STRAT_WAITFORU)) {
		stay_behind = FALSE;
		if (mtmp->mtame && mtmp->meating) {
			if (canseemon(mtmp))
			    pline("%s is still eating.", Monnam(mtmp));
			stay_behind = TRUE;
		} else if (mtmp->mtame && 
		    (Is_blackmarket(&new_dlevel) || Is_blackmarket(&u.uz))) {
			pline("%s can't follow you %s.",
			      Monnam(mtmp), Is_blackmarket(&u.uz) ?
			      "through the portal" : "into the Black Market");
			stay_behind = TRUE;
		} else if (mon_has_amulet(mtmp)) {
			if (canseemon(mtmp))
			    pline("%s seems very disoriented for a moment.",
				Monnam(mtmp));
			stay_behind = TRUE;
		} else if (mtmp->mtame && mtmp->mtrapped) {
			if (canseemon(mtmp))
			    pline("%s is still trapped.", Monnam(mtmp));
			stay_behind = TRUE;
		}
		if (mtmp == u.usteed) stay_behind = FALSE;
		if (stay_behind) {
			if (mtmp->mleashed) {
				pline("%s leash suddenly comes loose.",
					humanoid(mtmp->data)
					    ? (mtmp->female ? "Her" : "His")
					    : "Its");
				m_unleash(mtmp, FALSE);
			}
			continue;
		}
		if (mtmp->isshk)
			set_residency(mtmp, TRUE);

		if (mtmp->wormno) {
		    register int cnt;
		    /* NOTE: worm is truncated to # segs = max wormno size */
		    cnt = count_wsegs(mtmp);
		    num_segs = min(cnt, MAX_NUM_WORMS - 1);
		    wormgone(mtmp);
		} else num_segs = 0;

		/* set minvent's obj->no_charge to 0 */
		for(obj = mtmp->minvent; obj; obj = obj->nobj) {
		    if (Has_contents(obj))
			picked_container(obj);	/* does the right thing */
		    obj->no_charge = 0;
		}

		relmon(mtmp);
		newsym(mtmp->mx,mtmp->my);
		mtmp->mx = mtmp->my = 0; /* avoid mnexto()/MON_AT() problem */
		mtmp->wormno = num_segs;
		mtmp->mlstmv = monstermoves;
		mtmp->nmon = mydogs;
		mydogs = mtmp;
	    } else if (mtmp->iswiz) {
		/* we want to be able to find him when his next resurrection
		   chance comes up, but have him resume his present location
		   if player returns to this level before that time */
		migrate_to_level(mtmp, ledger_no(&u.uz),
				 MIGR_EXACT_XY, (coord *)0);
	    } else if (mtmp->mleashed) {
		/* this can happen if your quest leader ejects you from the
		   "home" level while a leashed pet isn't next to you */
		pline("%s leash goes slack.", s_suffix(Monnam(mtmp)));
		m_unleash(mtmp, FALSE);
	    }
	}
}

#endif /* OVL2 */
#ifdef OVLB

void
migrate_to_level(mtmp, tolev, xyloc, cc)
	register struct monst *mtmp;
	xchar tolev;	/* destination level */
	xchar xyloc;	/* MIGR_xxx destination xy location: */
	coord *cc;	/* optional destination coordinates */
{
	register struct obj *obj;
	d_level new_lev;
	xchar xyflags;
	int num_segs = 0;	/* count of worm segments */

	if (mtmp->isshk)
	    set_residency(mtmp, TRUE);

	if (mtmp->wormno) {
	    register int cnt;
	  /* **** NOTE: worm is truncated to # segs = max wormno size **** */
	    cnt = count_wsegs(mtmp);
	    num_segs = min(cnt, MAX_NUM_WORMS - 1);
	    wormgone(mtmp);
	}

	/* set minvent's obj->no_charge to 0 */
	for(obj = mtmp->minvent; obj; obj = obj->nobj) {
	    if (Has_contents(obj))
		picked_container(obj);	/* does the right thing */
	    obj->no_charge = 0;
	}

	if (mtmp->mleashed) {
		mtmp->mtame--;
		m_unleash(mtmp, TRUE);
	}
	relmon(mtmp);
	mtmp->nmon = migrating_mons;
	migrating_mons = mtmp;
	newsym(mtmp->mx,mtmp->my);

	new_lev.dnum = ledger_to_dnum((xchar)tolev);
	new_lev.dlevel = ledger_to_dlev((xchar)tolev);
	/* overload mtmp->[mx,my], mtmp->[mux,muy], and mtmp->mtrack[] as */
	/* destination codes (setup flag bits before altering mx or my) */
	xyflags = (depth(&new_lev) < depth(&u.uz));	/* 1 => up */
	if (In_W_tower(mtmp->mx, mtmp->my, &u.uz)) xyflags |= 2;
	mtmp->wormno = num_segs;
	mtmp->mlstmv = monstermoves;
	mtmp->mtrack[1].x = cc ? cc->x : mtmp->mx;
	mtmp->mtrack[1].y = cc ? cc->y : mtmp->my;
	mtmp->mtrack[0].x = xyloc;
	mtmp->mtrack[0].y = xyflags;
	mtmp->mux = new_lev.dnum;
	mtmp->muy = new_lev.dlevel;
	mtmp->mx = mtmp->my = 0;	/* this implies migration */
}

#endif /* OVLB */
#ifdef OVL1

/* return quality of food; the lower the better */
/* fungi will eat even tainted food */
int
dogfood(mon,obj)
struct monst *mon;
register struct obj *obj;
{
	boolean carni = carnivorous(mon->data);
	boolean herbi = (herbivorous(mon->data) || mon->egotype_petty);
	struct permonst *fptr = &mons[obj->corpsenm];
	boolean starving;
	struct monst *potentialpet;

	if (is_quest_artifact(obj) || obj_resists(obj, 0, 95))
	    return (obj->cursed ? TABU : APPORT);

	/* KMH -- Koalas can only eat eucalyptus */
	if (mon->data == &mons[PM_KOALA])
		return (obj->otyp == EUCALYPTUS_LEAF ? DOGFOOD : APPORT);
	if (mon->data == &mons[PM_GIANT_KOALA])
		return (obj->otyp == EUCALYPTUS_LEAF ? DOGFOOD : APPORT);

	switch(obj->oclass) {
	case FOOD_CLASS:
	    if (obj->otyp == CORPSE &&
		((touch_petrifies(&mons[obj->corpsenm]) && !resists_ston(mon)) ||
		((((potentialpet = get_mtraits(obj, FALSE)) != (struct monst *)0) ) && !issoviet && potentialpet->mtame)
		 || is_rider(fptr) || (!issoviet && (fptr->cnutrit < 1)) || is_deadlysin(fptr) || fptr->mlet == S_TROVE )) /* troves are meant for the player --Amy */
		    return TABU;
	    /* Ghouls only eat old corpses... yum! */
	    if (mon->data == &mons[PM_GHOUL] || mon->data == &mons[PM_GHAST] || mon->data == &mons[PM_GASTLY] || mon->data == &mons[PM_PHANTOM_GHOST]
		|| mon->data == &mons[PM_HAUNTER] || mon->data == &mons[PM_GENGAR]) {
		return (obj->otyp == CORPSE && obj->corpsenm != PM_ACID_BLOB &&
		  peek_at_iced_corpse_age(obj) + 5*rn1(20,10) <= monstermoves) ?
			DOGFOOD : TABU;
	    }
	    /* vampires only "eat" very fresh corpses ... 
	     * Assume meat -> blood
	     */
	    if (is_vampire(mon->data)) {
	    	return (obj->otyp == CORPSE &&
		  has_blood(&mons[obj->corpsenm]) && !obj->oeaten && !obj->odrained &&
	    	  peek_at_iced_corpse_age(obj) + 5 >= monstermoves) ?
			DOGFOOD : TABU;
	    }

	    if (!carni && !herbi)
		    return (obj->cursed ? UNDEF : APPORT);

 	    /* a starving pet will eat almost anything */
 	    starving = (mon->mtame && !mon->isminion &&
 			EDOG(mon)->mhpmax_penalty);

	    switch (obj->otyp) {
		case TRIPE_RATION:
		case MEATBALL:
		case MEAT_RING:
		case MEAT_STICK:
		case HUGE_CHUNK_OF_MEAT:
		    return (carni ? DOGFOOD : MANFOOD);
		case EGG:
		    if (touch_petrifies(&mons[obj->corpsenm]) && !resists_ston(mon))
			return POISON;
		    return (carni ? CADAVER : MANFOOD);
		case CORPSE:
                    /* WAC add don't eat own class*/
		    if (mons[obj->corpsenm].mlet == mon->data->mlet)
			return (starving && carni ? ACCFOOD : TABU);
		    else
		    if ((peek_at_iced_corpse_age(obj) + 50L <= monstermoves
					    && !nocorpsedecay(&mons[obj->corpsenm])
					    && mon->data->mlet != S_FUNGUS) ||
			(acidic(&mons[obj->corpsenm]) && !resists_acid(mon)) ||
			(poisonous(&mons[obj->corpsenm]) &&
						!resists_poison(mon)))
			return POISON;
		    else if (vegan(fptr))
			return (herbi ? CADAVER : MANFOOD);
		    else return (carni ? CADAVER : MANFOOD);
		case CLOVE_OF_GARLIC:
		    return ( (is_undead(mon->data) || mon->egotype_undead) ? TABU :
			    ((herbi || starving) ? ACCFOOD : MANFOOD));
		case TIN:
		    return ( (metallivorous(mon->data) || mon->egotype_metallivore) ? ACCFOOD : MANFOOD);
		case APPLE:
		case CARROT:
		    return (herbi ? DOGFOOD : starving ? ACCFOOD : MANFOOD);
		case BANANA:
		    return ((mon->data->mlet == S_YETI) ? DOGFOOD :
			    ((herbi || starving) ? ACCFOOD : MANFOOD));
		default:
		    if (starving) return ACCFOOD;
		    return (obj->otyp > SLIME_MOLD ?
			    (carni ? ACCFOOD : MANFOOD) :
			    (herbi ? ACCFOOD : MANFOOD));
	    }
	default:
	    if (obj->otyp == AMULET_OF_STRANGULATION ||
			obj->otyp == RIN_SLOW_DIGESTION)
		return TABU;
	    if (hates_silver(mon->data) &&
		objects[obj->otyp].oc_material == SILVER)
		return(TABU);
	    if (hates_viva(mon->data) &&
		objects[obj->otyp].oc_material == VIVA)
		return(TABU);
	    if (hates_inka(mon->data) &&
		objects[obj->otyp].oc_material == INKA)
		return(TABU);
		/* KMH -- Taz likes organics, too! */
	    if ((organivorous(mon->data) || mon->egotype_organivore) && is_organic(obj))
		return(ACCFOOD);
	    if ( (metallivorous(mon->data) || mon->egotype_metallivore) && is_metallic(obj) && (is_rustprone(obj) || mon->data != &mons[PM_RUST_MONSTER])) {
		/* Non-rustproofed ferrous based metals are preferred. */
		return((is_rustprone(obj) && !obj->oerodeproof) ? DOGFOOD :
			ACCFOOD);
	    }

	    if ( (lithivorous(mon->data) || mon->egotype_lithivore) && is_lithic(obj) ) return( DOGFOOD );

		/* Lithivores can eat any lithic object. They really like eating such items, too. */

	    if(!obj->cursed && obj->oclass != BALL_CLASS &&
						obj->oclass != CHAIN_CLASS)
		return(APPORT);
	    /* fall into next case */
	case ROCK_CLASS:
	    if ( (lithivorous(mon->data) || mon->egotype_lithivore) && is_lithic(obj) ) return( DOGFOOD );
	    return(UNDEF);
	}
}

#endif /* OVL1 */
#ifdef OVLB

struct monst *
tamedog(mtmp, obj, guaranteed)
register struct monst *mtmp;
register struct obj *obj;
boolean guaranteed;
{
	register struct monst *mtmp2;

	/* The Wiz, Medusa and the quest nemeses aren't even made peaceful. */
	if (mtmp->iswiz || mtmp->data == &mons[PM_MEDUSA]
				|| (mtmp->data->mflags3 & M3_WANTSARTI))
		{
		return((struct monst *)0);
		}

	if (Aggravate_monster && !rn2(20)) {
		if (mtmp->mpeaceful && !mtmp->mtame) {
	        	pline("%s is aggravated!", Monnam(mtmp));
			mtmp->mpeaceful = 0;
		} else if (!mtmp->mpeaceful && !mtmp->mtame) {
	        	pline("%s is frenzied!", Monnam(mtmp));
			mtmp->mfrenzied = 1;
		}
		return((struct monst *)0);
	}

	if (u.uprops[HATE_TRAP_EFFECT].extrinsic || HateTrapEffect || (uarms && uarms->oartifact == ART_REAL_PSYCHOS_WEAR_PURPLE) || (uarms && uarms->oartifact == ART_REAL_MEN_WEAR_PSYCHOS) || have_hatestone() || (uarmf && uarmf->oartifact == ART_KATIE_MELUA_S_FLEECINESS) ) {
        	pline("%s hates you too much!", Monnam(mtmp));
		return((struct monst *)0);
	}

	/* worst case, at least it'll be peaceful. */
	mtmp->mpeaceful = 1;
	mtmp->mtraitor  = 0;	/* No longer a traitor */
	set_malign(mtmp);
	if(flags.moonphase == FULL_MOON && night() && rn2(6) && obj
						&& mtmp->data->mlet == S_DOG)
		{
		return((struct monst *)0);

		}
    if ( (Role_if(PM_CONVICT) || Role_if(PM_MURDERER)) && (is_domestic(mtmp->data) && obj)) {
        /* Domestic animals are wary of the Convict */
        pline("%s still looks wary of you.", Monnam(mtmp));
        return((struct monst *)0);
        }

	/* If we cannot tame it, at least it's no longer afraid. */
	mtmp->mflee = 0;
	mtmp->mfleetim = 0;

	/* make grabber let go now, whether it becomes tame or not */
	if (mtmp == u.ustuck) {
	    if (u.uswallow)
		expels(mtmp, mtmp->data, TRUE);
	    else if (!(Upolyd && sticks(youmonst.data)))
		unstuck(mtmp);
	}

	/* feeding it treats makes it tamer */
	if (mtmp->mtame && obj) {
	    int tasty;

	    if (mtmp->mcanmove && !mtmp->mconf && !mtmp->meating &&
		((tasty = dogfood(mtmp, obj)) == DOGFOOD ||
		 (tasty <= ACCFOOD && EDOG(mtmp)->hungrytime <= monstermoves))) {
		/* pet will "catch" and eat this thrown food */
		if (canseemon(mtmp)) {
		    boolean big_corpse = (obj->otyp == CORPSE &&
					  obj->corpsenm >= LOW_PM &&
				mons[obj->corpsenm].msize > mtmp->data->msize);
		    pline("%s catches %s%s",
			  Monnam(mtmp), the(xname(obj)),
			  !big_corpse ? "." : ", or vice versa!");
		} else if (cansee(mtmp->mx,mtmp->my))
		    pline("%s.", Tobjnam(obj, "stop"));

		/* since there's no way to say sorry if you accidentally hit your pet, feeding has a similar effect --Amy */
		if (!rn2(5) && !mtmp->isminion && EDOG(mtmp)->abuse) EDOG(mtmp)->abuse--;

		/* dog_eat expects a floor object */
		place_object(obj, mtmp->mx, mtmp->my);
		(void) dog_eat(mtmp, obj, mtmp->mx, mtmp->my, FALSE);
		/* eating might have killed it, but that doesn't matter here;
		   a non-null result suppresses "miss" message for thrown
		   food and also implies that the object has been deleted */
		return mtmp;
	    } else {

		if (!is_domestic(mtmp->data) && !is_petty(mtmp->data)) return (struct monst *)0;
		}
	}


	if (mtmp->mtame || !mtmp->mcanmove ||
	    /* monsters with conflicting structures cannot be tamed */
	    mtmp->isshk || mtmp->isgd || mtmp->ispriest || mtmp->isminion ||
	    /* KMH -- Added gypsy */
	    mtmp->isgyp ||
	    (is_covetous(mtmp->data) && (issoviet || rn2(50) ) ) || (is_human(mtmp->data) && (issoviet || rn2(4) ) ) ||
	    (is_demon(mtmp->data) && !is_demon(youmonst.data) && !Race_if(PM_HUMANOID_DEVIL) && (issoviet || rn2(10) ) ) ||
	    /* Mik -- New flag to indicate which things cannot be tamed... */
	    cannot_be_tamed(mtmp->data) || mtmp->mfrenzied ||
	    (obj && dogfood(mtmp, obj) >= MANFOOD)) {

	/* In Soviet Russia, people only know dogs, cats and maybe horses. Everything else cannot be tamed, and variety
	 * is also something that absolutely no one likes. Communism means everyone gets the same, so you can't have an
	 * exotic pet! If you do then you'll "vanish" silently, like all the other lawbreakers. --Amy */

		/* Attire charm technique and certain other methods set the guaranteed flag,
		 * which allows players to tame humans and certain other monsters. --Amy */

		if (!is_domestic(mtmp->data) && !is_petty(mtmp->data) && !(guaranteed) )
			return (struct monst *)0;

		if (!(guaranteed) && obj && dogfood(mtmp, obj) >= MANFOOD)
			return (struct monst *)0;

		}

	/* failsafe for things that REALLY cannot be tamed --Amy */
	if (cannot_be_tamed(mtmp->data) || mtmp->mfrenzied || mtmp->mtame || mtmp->isshk || mtmp->isgd || mtmp->ispriest || mtmp->isminion || mtmp->isgyp)
		return (struct monst *)0;

	if (mtmp->m_id == quest_status.leader_m_id)
		{
	    return((struct monst *)0);
		}

	/* make a new monster which has the pet extension */
	mtmp2 = newmonst(sizeof(struct edog) + mtmp->mnamelth);
	*mtmp2 = *mtmp;
	mtmp2->mxlth = sizeof(struct edog);
	if (mtmp->mnamelth) strcpy(NAME(mtmp2), NAME(mtmp));
	initedog(mtmp2);
	replmon(mtmp, mtmp2);
	/* `mtmp' is now obsolete */

	if (obj) {		/* thrown food */
	    /* defer eating until the edog extension has been set up */
	    place_object(obj, mtmp2->mx, mtmp2->my);	/* put on floor */
	    /* devour the food (might grow into larger, genocided monster) */
	    if (dog_eat(mtmp2, obj, mtmp2->mx, mtmp2->my, TRUE) == 2)
		return mtmp2;		/* oops, it died... */
	    /* `obj' is now obsolete */
	}

	newsym(mtmp2->mx, mtmp2->my);
	if (attacktype(mtmp2->data, AT_WEAP)) {
		mtmp2->weapon_check = NEED_HTH_WEAPON;
		(void) mon_wield_item(mtmp2);
	}
	return(mtmp2);
}

int make_pet_minion(mnum,alignment)
int mnum;
aligntyp alignment;
{
    register struct monst *mon;
    register struct monst *mtmp2;
    mon = makemon(&mons[mnum], u.ux, u.uy, MM_NOSPECIALS);
    if (!mon) return 0;
    /* now tame that puppy... */
    mtmp2 = newmonst(sizeof(struct edog) + mon->mnamelth);
    *mtmp2 = *mon;
    mtmp2->mxlth = sizeof(struct edog);
    if(mon->mnamelth) strcpy(NAME(mtmp2), NAME(mon));
    initedog(mtmp2);
    replmon(mon,mtmp2);
    newsym(mtmp2->mx, mtmp2->my);
    mtmp2->mpeaceful = 1;
    set_malign(mtmp2);
    mtmp2->mtame = 10;
    /* this section names the creature "of ______" */
    if (mons[mnum].pxlth == 0) {
	mtmp2->isminion = TRUE;
	EMIN(mtmp2)->min_align = alignment;
    } else if (mnum == PM_ANGEL) {
	 mtmp2->isminion = TRUE;
	 EPRI(mtmp2)->shralign = alignment;
    }
    return 1;
}

/*
 * Called during pet revival or pet life-saving.
 * If you killed the pet, it revives wild.
 * If you abused the pet a lot while alive, it revives wild.
 * If you abused the pet at all while alive, it revives untame.
 * If the pet wasn't abused and was very tame, it might revive tame.
 */
void
wary_dog(mtmp, was_dead)
struct monst *mtmp;
boolean was_dead;
{
    struct edog *edog;
    boolean quietly = was_dead;

    mtmp->meating = 0;
    if (!mtmp->mtame) return;
    edog = !mtmp->isminion ? EDOG(mtmp) : 0;

    /* if monster was starving when it died, undo that now */
    if (edog && edog->mhpmax_penalty) {
	mtmp->mhpmax += edog->mhpmax_penalty;
	mtmp->mhp += edog->mhpmax_penalty;	/* heal it */
	edog->mhpmax_penalty = 0;
    }

    if (edog && (edog->killed_by_u == 1 || edog->abuse > 2)) {
	mtmp->mpeaceful = mtmp->mtame = 0;
	if (edog->abuse >= 0 && edog->abuse < 10)
	    if (!rn2(edog->abuse + 1)) mtmp->mpeaceful = 1;
	if(!quietly && cansee(mtmp->mx, mtmp->my)) {
	    if (haseyes(youmonst.data)) {
		if (haseyes(mtmp->data))
			pline("%s %s to look you in the %s.",
				Monnam(mtmp),
				mtmp->mpeaceful ? "seems unable" :
					    "refuses",
				body_part(EYE));
		else 
			pline("%s avoids your gaze.",
				Monnam(mtmp));
	    }
	}
    } else {
	/* chance it goes wild anyway - Pet Semetary */
	if (!rn2(mtmp->mtame)) {
	    mtmp->mpeaceful = mtmp->mtame = 0;
	}
    }
    if (!mtmp->mtame) {
	newsym(mtmp->mx, mtmp->my);
	/* a life-saved monster might be leashed;
	   don't leave it that way if it's no longer tame */
	if (mtmp->mleashed) m_unleash(mtmp, TRUE);
    }

    /* if its still a pet, start a clean pet-slate now */
    if (edog && mtmp->mtame) {
	edog->revivals++;
	edog->killed_by_u = 0;
	edog->abuse = 0;
	edog->ogoal.x = edog->ogoal.y = -1;
	if (was_dead || edog->hungrytime < monstermoves + 500L)
	    edog->hungrytime = monstermoves + 500L;
	if (was_dead) {
	    edog->droptime = 0L;
	    edog->dropdist = 10000;
	    edog->whistletime = 0L;
	    edog->apport = 5;
	} /* else lifesaved, so retain current values */
    }
}

void
abuse_dog(mtmp)
struct monst *mtmp;
{
	if (!mtmp->mtame) return;

	if (Aggravate_monster || Conflict) mtmp->mtame /=2;
	else mtmp->mtame--;

	if (mtmp->mtame && !mtmp->isminion)
	    EDOG(mtmp)->abuse++;

	if (Role_if(PM_CRUEL_ABUSER)) {
		adjalign(5);
		pline(Hallucination ? "Let's whip that bitch some more!" : "You feel empowered."); /* Christian Grey likes to be needlessly cruel */
	}

	if (!mtmp->mtame && mtmp->mleashed)
	    m_unleash(mtmp, TRUE);

	/* don't make a sound if pet is in the middle of leaving the level */
	/* newsym isn't necessary in this case either */
	if (mtmp->mx != 0) {
	    if (mtmp->mtame && rn2(mtmp->mtame)) yelp(mtmp);
	    else growl(mtmp);	/* give them a moment's worry */

	    /* Give monster a chance to betray you now */
	    if (mtmp->mtame) betrayed(mtmp);
	
	    if (!mtmp->mtame) newsym(mtmp->mx, mtmp->my);
	}
}

#endif /* OVLB */

/*dog.c*/
