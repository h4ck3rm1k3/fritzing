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

int placeCellFunc(Tile *, UserData);
int deleteCellFunc(Tile *, UserData);
Tile * clipCellTile(Tile * tile, Plane * plane, TileRect * rect);
void cellTileMerge(Tile  * tile, Plane * plane, int direction); 
bool ctbListMatch (Tile *tp1, Tile *tp2);
void dupTileBody(Tile * oldtp, Tile * newtp);

struct searchArg
{
    TileRect * rect;
    Plane * plane;
	QGraphicsItem * body;
	int type;
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
DBPlaceCell (Plane * plane, TileRect * rect, QGraphicsItem * body, int type)
/* argument to TiSrArea(), placeCellFunc() */
/* argument to TiSrArea(), placeCellFunc() */
{
    struct searchArg arg;       /* argument to placeCellFunc() */
    arg.rect = rect;
    arg.plane = plane;
	arg.body = body;
	arg.type = type;

    (void) TiSrArea((Tile *) NULL, plane, rect, placeCellFunc, (UserData) &arg);

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

    (void) TiSrArea((Tile *) NULL, plane, rect, deleteCellFunc, (UserData) &arg);

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
placeCellFunc (Tile * tile, UserData data)
    /* target tile */
    /* celluse, rect, plane */
{
	struct searchArg * arg = (struct searchArg *) data;
    Tile * tp = clipCellTile (tile, arg->plane, arg->rect);
	TiSetType(tp, arg->type);
	TiSetBody(tp, arg->body);

/* merge tiles back into the the plane */
/* requires that TiSrArea visit tiles in NW to SE wavefront */

    if ( RIGHT(tp) == arg->rect->xmax)
    {
        if (YMIN(tp) == arg->rect->ymin)
            cellTileMerge (tp, arg->plane, TOPBOTTOMLEFTRIGHT);
        else
            cellTileMerge (tp, arg->plane, TOPLEFTRIGHT);
    }
    else if (YMIN(tp) == arg->rect->ymin)
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
deleteCellFunc (Tile * tile, UserData data)
{

	struct searchArg * arg = (struct searchArg *) data;

/* merge tiles back into the the plane */
/* requires that TiSrArea visit tiles in NW to SE wavefront */

    if ( RIGHT(tile) == arg->rect->xmax)
    {
        if (YMIN(tile) == arg->rect->ymin)
            cellTileMerge (tile, arg->plane, TOPBOTTOMLEFTRIGHT);
        else
            cellTileMerge (tile, arg->plane, TOPLEFTRIGHT);
    }
    else if (YMIN(tile) == arg->rect->ymin)
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

    if (YMAX(tile) > rect->ymax)
    {

        newtile = TiSplitY (tile, rect->ymax);	/* no merge */
		dupTileBody (tile, newtile);
    }
    if (YMIN(tile) < rect->ymin)
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
/* YMAX = 8, YMIN = 4, LEFT = 2, RIGHT = 1 */
{
    TilePoint       topleft, bottomright;
    Tile      * dummy, * tpleft, * tpright, * tp1, * tp2;


    topleft.x = LEFT(tile);
    topleft.y = YMAX(tile);
    bottomright.x = RIGHT(tile);
    bottomright.y = YMIN(tile);

    if ((direction >> 1) % 2)			/* LEFT */
    {
	tpright = tile;
        tpleft = BL(tpright);

	while (YMIN(tpleft) < topleft.y)	/* go up left edge */
	{
	    if (ctbListMatch (tpleft, tpright))
	    {
	        if (YMIN(tpleft) < YMIN(tpright))
		{
		    dummy = tpleft;
		    tpleft = TiSplitY (tpleft, YMIN (tpright));
		}
		else if (YMIN(tpleft) > YMIN(tpright))
		{
		    dummy = tpright;
		    tpright = TiSplitY (tpright, YMIN (tpleft));
		}

		if (YMAX(tpleft) > YMAX(tpright))
		{
		    dummy = TiSplitY (tpleft, YMAX(tpright));
		}
		else if (YMAX(tpright) > YMAX(tpleft))
		{
		    dummy = TiSplitY (tpright, YMAX(tpleft));
		}

		// if (plane->pl_hint == tpright) plane->pl_hint = tpleft;
		TiJoinX (tpleft, tpright, plane);  /* tpright disappears */

		tpright = RT(tpleft);
		if (YMIN(tpright) < topleft.y) tpleft = BL(tpright);
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


	while (YMIN(tpright) < topleft.y)	/* go up right edge */
	{
	    if (ctbListMatch (tpleft, tpright))
	    {
	        if (YMIN(tpright) < YMIN(tpleft))
		{
		    dummy = tpright;
		    tpright = TiSplitY (tpright, YMIN(tpleft));
		}
		else if (YMIN(tpleft) < YMIN(tpright))
		{
		    dummy = tpleft;
		    tpleft = TiSplitY (tpleft, YMIN(tpright));
		}

		if (YMAX(tpright) > YMAX(tpleft))
		{
		    dummy = TiSplitY (tpright, YMAX(tpleft));
		}
		else if (YMAX(tpleft) > YMAX(tpright))
		{
   		    dummy = TiSplitY (tpleft, YMAX(tpright));
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

    if ((direction >> 3) % 2)			/* YMAX */
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

    if ((direction >> 2) % 2)			/* YMIN */
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
	return (tp1->ti_body == tp2->ti_body) && (tp1->ti_type == tp2->ti_type);
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
	TiSetType(newtp, TiGetType(oldtp));
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

Tile* TiInsertTile(Plane * plane, TileRect * rect, QGraphicsItem * body, int type) {

	DBPlaceCell(plane, rect, body, type);
	return TiSrPoint(NULL, plane, rect->xmin, rect->ymin);
}
