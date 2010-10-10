/*
 * DBcell.c --
 *
 * Place and Delete subcells
 *
 *     ********************************************************************* 
 *     * Copyright (C) 1985, 1990 Regents of the University of California. * 
 *     * Permission to use, copy, modify, and distribute this              * 
 *     * software and its documentation for any purpose and without        * 
 *     * fee is hereby granted, provided that the above copyright          * 
 *     * notice appear in all copies.  The University of California        * 
 *     * makes no representations about the suitability of this            * 
 *     * software for any purpose.  It is provided "as is" without         * 
 *     * express or implied warranty.  Export of this software outside     * 
 *     * of the United States of America may require an export license.    * 
 *     *********************************************************************
 */


#include <sys/types.h>
#include <stdio.h>

#include "tile.h"

int placeCellFunc(Tile *, ClientData);
int deleteCellFunc(Tile *, ClientData);
Tile * clipCellTile(Tile * tile, Plane * plane, TileRect * rect);
void cellTileMerge(Tile  * tile, Plane * plane, int direction); 
bool ctbListMatch (Tile *tp1, Tile *tp2);
void dupTileBody(Tile * oldtp, Tile * newtp);

struct searchArg
{
    TileRect * rect;
    Plane * plane;
	ClientData body;
};

#define		TOPLEFT			10
#define		TOPLEFTRIGHT		11
#define		TOPBOTTOM		12
#define         TOPBOTTOMLEFT   	14
#define 	TOPBOTTOMLEFTRIGHT	15


/*
 * ----------------------------------------------------------------------------
 *
 * DBPlaceCell --
 *
 * Add a CellUse to the subcell tile plane of a CellDef.
 * Assumes prior check that the new CellUse is not an exact duplicate
 *     of one already in place.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Modifies the subcell tile plane of the given CellDef.
 *	Resets the plowing delta of the CellUse to 0.  Sets the
 *	CellDef's parent pointer to point to the parent def.
 *
 * ----------------------------------------------------------------------------
 */

void
DBPlaceCell (Plane * plane, TileRect * rect, ClientData body)
/* argument to TiSrArea(), placeCellFunc() */
/* argument to TiSrArea(), placeCellFunc() */
{
    struct searchArg arg;       /* argument to placeCellFunc() */
    arg.rect = rect;
    arg.plane = plane;
	arg.body = body;

    (void) TiSrArea((Tile *) NULL, plane, rect, placeCellFunc, (ClientData) &arg);

}

/*
 * ----------------------------------------------------------------------------
 * DBDeleteCell --
 *
 * Remove a CellUse from the subcell tile plane of a CellDef.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Modifies the subcell tile plane of the CellDef, sets the
 * 	parent pointer of the deleted CellUse to NULL.
 * ----------------------------------------------------------------------------
 */

void
DBDeleteCell (Plane  * plane, TileRect * rect)
/* argument to TiSrArea(), deleteCellFunc() */
/* argument to TiSrArea(), deleteCellFunc() */
{
    struct searchArg arg;	/* argument to deleteCellFunc() */

    (void) TiSrArea((Tile *) NULL, plane, rect, deleteCellFunc, (ClientData) &arg);

}

/*
 * ----------------------------------------------------------------------------
 * placeCellFunc --
 *
 * Add a new subcell to a tile.
 * Clip the tile with respect to the subcell's bounding box.
 * Insert the new CellTileBody into the linked list in ascending order
 *      based on the celluse pointer.
 * This function is passed to TiSrArea.
 *
 * Results:
 *	0 is always returned.
 *
 * Side effects:
 *	Modifies the subcell tile plane of the appropriate CellDef.
 *      Allocates a new CellTileBody.
 * ----------------------------------------------------------------------------
 */

int
placeCellFunc (Tile * tile, ClientData data)
    /* target tile */
    /* celluse, rect, plane */
{
	struct searchArg * arg = (struct searchArg *) data;
    Tile * tp = clipCellTile (tile, arg->plane, arg->rect);
	TiSetBody(tp, arg->body);

/* merge tiles back into the the plane */
/* requires that TiSrArea visit tiles in NW to SE wavefront */

    if ( RIGHT(tp) == arg->rect->xmax)
    {
        if (BOTTOM(tp) == arg->rect->ymin)
            cellTileMerge (tp, arg->plane, TOPBOTTOMLEFTRIGHT);
        else
            cellTileMerge (tp, arg->plane, TOPLEFTRIGHT);
    }
    else if (BOTTOM(tp) == arg->rect->ymin)
            cellTileMerge (tp, arg->plane, TOPBOTTOMLEFT);
    else
    	    cellTileMerge (tp, arg->plane, TOPLEFT);
    return 0;
}

/*
 * ----------------------------------------------------------------------------
 * deleteCellFunc --
 *
 * Remove a subcell from a tile.
 * This function is passed to TiSrArea.
 *
 * Results:
 *	Always returns 0.
 *
 * Side effects:
 *	Modifies the subcell tile plane of the appropriate CellDef.
 *      Deallocates a CellTileBody.
 * ----------------------------------------------------------------------------
 */

int
deleteCellFunc (Tile * tile, ClientData data)
{

	struct searchArg * arg = (struct searchArg *) data;

/* merge tiles back into the the plane */
/* requires that TiSrArea visit tiles in NW to SE wavefront */

    if ( RIGHT(tile) == arg->rect->xmax)
    {
        if (BOTTOM(tile) == arg->rect->ymin)
            cellTileMerge (tile, arg->plane, TOPBOTTOMLEFTRIGHT);
        else
            cellTileMerge (tile, arg->plane, TOPLEFTRIGHT);
    }
    else if (BOTTOM(tile) == arg->rect->ymin)
            cellTileMerge (tile, arg->plane, TOPBOTTOMLEFT);
    else
    	    cellTileMerge (tile, arg->plane, TOPLEFT);
    return (0);
}

/*
 * ----------------------------------------------------------------------------
 * clipCellTile -- 
 *
 * Clip the given tile against the given rectangle.
 *
 * Results:
 *	Returns a pointer to the clipped tile.
 *
 * Side effects:
 *	Modifies the database plane that contains the given tile.
 * ----------------------------------------------------------------------------
 */

Tile *
clipCellTile (Tile * tile, Plane * plane, TileRect * rect)
{
    Tile     * newtile;

    if (TOP(tile) > rect->ymax)
    {

        newtile = TiSplitY (tile, rect->ymax);	/* no merge */
		dupTileBody (tile, newtile);
    }
    if (BOTTOM(tile) < rect->ymin)
    {

	newtile = tile;
        tile = TiSplitY (tile, rect->ymin);		/* no merge */
		dupTileBody (newtile, tile);
    }
    if (RIGHT(tile) > rect->xmax)
    {
        newtile = TiSplitX (tile, rect->xmax);
		dupTileBody (tile, newtile);
        cellTileMerge (newtile, plane, TOPBOTTOM);
    }
    if (LEFT(tile) < rect->xmin)
    {
		newtile = tile;
        tile = TiSplitX (tile, rect->xmin);
		dupTileBody (newtile, tile);
        cellTileMerge (newtile, plane, TOPBOTTOM);
    }
    return (tile);
} /* clipCellTile */


/*
 * ----------------------------------------------------------------------------
 * cellTileMerge -- 
 *
 * Merge the given tile with its plane in the directions specified.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Modifies the database plane that contains the given tile.
 * ----------------------------------------------------------------------------
 */

void
cellTileMerge (Tile  * tile, Plane * plane, int direction) 
/* TOP = 8, BOTTOM = 4, LEFT = 2, RIGHT = 1 */
{
    TilePoint       topleft, bottomright;
    Tile      * dummy, * tpleft, * tpright, * tp1, * tp2;


    topleft.x = LEFT(tile);
    topleft.y = TOP(tile);
    bottomright.x = RIGHT(tile);
    bottomright.y = BOTTOM(tile);

    if ((direction >> 1) % 2)			/* LEFT */
    {
	tpright = tile;
        tpleft = BL(tpright);

	while (BOTTOM(tpleft) < topleft.y)	/* go up left edge */
	{
	    if (ctbListMatch (tpleft, tpright))
	    {
	        if (BOTTOM(tpleft) < BOTTOM(tpright))
		{
		    dummy = tpleft;
		    tpleft = TiSplitY (tpleft, BOTTOM (tpright));
		}
		else if (BOTTOM(tpleft) > BOTTOM(tpright))
		{
		    dummy = tpright;
		    tpright = TiSplitY (tpright, BOTTOM (tpleft));
		}

		if (TOP(tpleft) > TOP(tpright))
		{
		    dummy = TiSplitY (tpleft, TOP(tpright));
		}
		else if (TOP(tpright) > TOP(tpleft))
		{
		    dummy = TiSplitY (tpright, TOP(tpleft));
		}

		// if (plane->pl_hint == tpright) plane->pl_hint = tpleft;
		TiJoinX (tpleft, tpright, plane);  /* tpright disappears */

		tpright = RT(tpleft);
		if (BOTTOM(tpright) < topleft.y) tpleft = BL(tpright);
		else tpleft = tpright;	/* we're off the top of the tile */
					/* this will break the while loop */
	    } /* if (ctbListMatch (tpleft, tpright)) */

	    else tpleft = RT(tpleft);
	} /* while */
	tile = tpleft;		/* for TiSrPoint in next IF statement */
    }

    if (direction % 2)				/* RIGHT */
    {
	tpright = TiSrPoint (tile, plane, bottomright.x, bottomright.y);
	tpleft = TiSrPoint (tpright, plane, bottomright.x - MINDIFF, bottomright.y);


	while (BOTTOM(tpright) < topleft.y)	/* go up right edge */
	{
	    if (ctbListMatch (tpleft, tpright))
	    {
	        if (BOTTOM(tpright) < BOTTOM(tpleft))
		{
		    dummy = tpright;
		    tpright = TiSplitY (tpright, BOTTOM(tpleft));
		}
		else if (BOTTOM(tpleft) < BOTTOM(tpright))
		{
		    dummy = tpleft;
		    tpleft = TiSplitY (tpleft, BOTTOM(tpright));
		}

		if (TOP(tpright) > TOP(tpleft))
		{
		    dummy = TiSplitY (tpright, TOP(tpleft));
		}
		else if (TOP(tpleft) > TOP(tpright))
		{
   		    dummy = TiSplitY (tpleft, TOP(tpright));
		}

		// if (plane->pl_hint == tpright) plane->pl_hint = tpleft;
		TiJoinX (tpleft, tpright, plane);  /* tpright disappears */

		tpright = RT(tpleft);
		while (LEFT(tpright) > bottomright.x) tpright = BL(tpright);

		/* tpleft can be garbage if we're off the top of the loop, */
		/* but it doesn't matter since the expression tests tpright */

		tpleft = BL(tpright);
	    } /* if (ctbListMatch (tpleft, tpright)) */

	    else
	    {
		tpright = RT(tpright);
	        while (LEFT(tpright) > bottomright.x) tpright = BL(tpright);
		tpleft = BL(tpright);		/* left side merges may have */
						/* created more tiles */
	    }
	} /* while */
	tile = tpright;		/* for TiSrPoint in next IF statement */
    }

    if ((direction >> 3) % 2)			/* TOP */
    {
	tp1 = TiSrPoint (tile, plane, topleft.x, topleft.y);	/* merge across top */
	tp2 = TiSrPoint (tile, plane, topleft.x, topleft.y - MINDIFF);/* top slice of original tile */


	if ((LEFT(tp1) == LEFT(tp2)  ) &&
	    (RIGHT(tp1) == RIGHT(tp2)) &&
	    (ctbListMatch (tp1, tp2) ))
	{
	    // if (plane->pl_hint == tp2) plane->pl_hint = tp1;
	    TiJoinY (tp1, tp2, plane);
	}

	tile = tp1;		/* for TiSrPoint in next IF statement */
    }

    if ((direction >> 2) % 2)			/* BOTTOM */
    {
	                                       /* bottom slice of orig tile */
	tp1 = TiSrPoint (tile, plane, bottomright.x - MINDIFF, bottomright.y);
	tp2 = TiSrPoint (tile, plane, bottomright.x - MINDIFF, bottomright.y - MINDIFF); /* merge across bottom */

	if ((LEFT(tp1) == LEFT(tp2)  ) &&
	    (RIGHT(tp1) == RIGHT(tp2)) &&
	    (ctbListMatch (tp1, tp2) ))
	{
	    // if (plane->pl_hint == tp2) plane->pl_hint = tp1;
	    TiJoinY (tp1, tp2, plane);
	}
    }
}

/*
 * ----------------------------------------------------------------------------
 * ctbListMatch -- 
 *
 * Compare two linked lists of CellTileBodies, assuming that they are
 * sorted in ascending order by celluse pointers.
 *
 * Results:
 *	True if the tiles have identical lists of CellTileBodies.
 *
 * Side effects:
 *      None.
 * ----------------------------------------------------------------------------
 */

bool
ctbListMatch (Tile *tp1, Tile *tp2)

{
	return tp1->ti_body == tp2->ti_body;
}

/*
 * ----------------------------------------------------------------------------
 * dupTileBody -- 
 *
 * Duplicate the body of an old tile as the body for a new tile.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Allocates new CellTileBodies unless the old tile was a space tile.
 * ----------------------------------------------------------------------------
 */

void
dupTileBody (Tile * oldtp, Tile * newtp)
{
	TiSetBody(newtp, TiGetBody(oldtp));
}
	
	
/*
 * ----------------------------------------------------------------------------
 * TiInsertTile -- 
 *
 * create a tile with the given rect and insert it into the plane.
 *
 * Results:
 *	the new Tile.
 *
 * Side effects:
 *	Modifies the database plane that contains the given tile.
 * ----------------------------------------------------------------------------
 */

	
struct AlreadyThing {
	TileCallback otherCallback;
	ClientData data;
	bool found;
};

int checkAlready(Tile * tile, ClientData data) {
	AlreadyThing * alreadyThing = (AlreadyThing *) data;
	alreadyThing->found = alreadyThing->found || (tile->ti_client != NULL);
	if (alreadyThing->otherCallback) {
		return (*alreadyThing->otherCallback)(tile, alreadyThing->data);
	}
	return 0;
}

Tile* TiInsertTile(Plane * plane, TileRect * rect, TileCallback ifAlready, ClientData data, ClientData body) {
	AlreadyThing alreadyThing;
	alreadyThing.found = false;
	alreadyThing.data = data;
	alreadyThing.otherCallback = ifAlready;
	TiSrArea(NULL, plane, rect, checkAlready, &alreadyThing);
	if (alreadyThing.found) return NULL;

	DBPlaceCell(plane, rect, body);
	return TiSrPoint(NULL, plane, rect->xmin, rect->ymin);
}
