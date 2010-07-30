/**
 *   //////////////////
 *   // MAXIMUM FLOW //
 *   //////////////////
 *
 * This file is part of my library of algorithms found here:
 *      http://www.palmcommander.com:8081/tools/
 * LICENSE:
 *      http://www.palmcommander.com:8081/tools/LICENSE.html
 * Copyright (c) 2004
 * Contact author:
 *      igor at cs.ubc.ca
 **/

/**
 * jrc: 23 july 2010: license and original code from http://shygypsy.com/tools/
 * jrc: 23 july 2010: made arrays dynamic instead of static
**/

/****************
 * Maximum flow * (Ford-Fulkerson on an adjacency matrix)
 ****************
 * Takes a weighted directed graph of edge capacities as an adjacency 
 * matrix 'cap' and returns the maximum flow from s to t.
 *
 * PARAMETERS:
 *      - cap (global): adjacency matrix where cap[u][v] is the capacity
 *          of the edge u->v. cap[u][v] is 0 for non-existent edges.
 *      - n: the number of vertices ([0, n-1] are considered as vertices).
 *      - s: source vertex.
 *      - t: sink.
 * RETURNS:
 *      - the flow
 *      - fnet contains the flow network. Careful: both fnet[u][v] and
 *          fnet[v][u] could be positive. Take the difference.
 *      - prev contains the minimum cut. If prev[v] == -1, then v is not
 *          reachable from s; otherwise, it is reachable.
 * DETAILS:
 * FIELD TESTING:
 *      - Valladolid 10330: Power Transmission
 *      - Valladolid 653:   Crimewave
 *      - Valladolid 753:   A Plug for UNIX
 *      - Valladolid 10511: Councilling
 *      - Valladolid 820:   Internet Bandwidth
 *      - Valladolid 10779: Collector's Problem
 * #include <string.h>
 * #include <queue>
 **/

#include <string.h>
#include "flow.h"

int fordFulkerson( QVector< QVector<int> > & cap, QVector<int> & prev, int n, int s, int t )
{
	// flow network
	QVector< QVector<int> > fnet(n, QVector<int>(n, 0));  // init the flow network to zero

	// BFS
	int qf, qb;
	QVector<int> q(n);

    int flow = 0;

    while( true )
    {
        // find an augmenting path
        prev.fill(-1);
        qf = qb = 0;
        prev[q[qb++] = s] = -2;
		while( qb > qf && prev[t] == -1 ) {
			for( int u = q[qf++], v = 0; v < n; v++ ) {
				if( prev[v] == -1 && fnet[u][v] - fnet[v][u] < cap[u][v] ) {
                    prev[q[qb++] = v] = u;
				}
			}
		}

        // see if we're done
        if( prev[t] == -1 ) break;

        // get the bottleneck capacity
        int bot = 0x7FFFFFFF;
		for( int v = t, u = prev[v]; u >= 0; v = u, u = prev[v] ) {
			int m = cap[u][v] - fnet[u][v] + fnet[v][u];    // jrc 23 July 2010, instead of <?= (min operator)
            if (m < bot) bot = m;
		}

        // update the flow network
        for( int v = t,u = prev[v]; u >= 0; v = u, u = prev[v] )
            fnet[u][v] += bot;

        flow += bot;
    }

    return flow;
}


/*
//----------------- EXAMPLE USAGE -----------------
int main()
{
    int numVertices = 100;
	int ** cap = (int *) malloc(sizeof(int) * numVertices * numVertices);
	int * prev = (int *) malloc(sizeof(int) * numVertices);
    memset( cap, 0, sizeof( cap ) );
    
    // ... fill up cap with existing edges.
    // if the edge u->v has capacity 6, set cap[u][v] = 6.        
    
    cout << fordFulkerson( numVertices, cap, prev, s, t ) << endl;
    
    return 0;
}
*/
