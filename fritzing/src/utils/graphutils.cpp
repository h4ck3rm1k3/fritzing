/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision: 4228 $:
$Author: cohen@irascible.com $:
$Date: 2010-06-09 00:55:55 +0200 (Wed, 09 Jun 2010) $

********************************************************************/

#include "graphutils.h"
#include "../fsvgrenderer.h"

#ifdef _MSC_VER 
#pragma warning(push) 
#pragma warning(disable:4100)			// disable scary-looking compiler warning in Boost library
#endif

#include <boost/config.hpp>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

#ifdef _MSC_VER 
#pragma warning(pop)					// restore warning state
#endif

bool GraphUtils::chooseRatsnestGraph(const QList<ConnectorItem *> & connectorItems, ConnectorPairHash & result) {
  using namespace boost;
  typedef adjacency_list < vecS, vecS, undirectedS,
    property<vertex_distance_t, double>, property < edge_weight_t, double > > Graph;
  typedef std::pair < int, int >E;

  if (connectorItems.count() < 2) return false;

  QList <ConnectorItem *> temp = connectorItems;

  int i = 0;
  while (i < temp.count()) {
	  ConnectorItem * connectorItem = temp[i++];
	  ConnectorItem * crossConnectorItem = connectorItem->getCrossLayerConnectorItem();
	  if (crossConnectorItem) {
		  temp.removeOne(crossConnectorItem);
	  }
  }

  QList<QPointF> locs;
  foreach (ConnectorItem * connectorItem, temp) {
	  locs << connectorItem->sceneAdjustedTerminalPoint(NULL);
  }

  int num_nodes = temp.count();
  int num_edges = num_nodes * (num_nodes - 1) / 2;
  E * edges = new E[num_edges];
  double * weights = new double[num_edges];
  int ix = 0;
  for (int i = 0; i < num_nodes; i++) {
		ConnectorItem * c1 = temp.at(i);
		for (int j = i + 1; j < num_nodes; j++) {
			ConnectorItem * c2 = temp.at(j);
			if ((c1->attachedTo() == c2->attachedTo()) && (c1->bus() != NULL) && (c1->bus() == c2->bus())) {
				num_edges--;
				continue;
			}

			edges[ix].first = i;
			edges[ix].second = j;
			double dx = locs[i].x() - locs[j].x();
			double dy = locs[i].y() - locs[j].y();
			weights[ix++] = (dx * dx) + (dy * dy);
		}
  }

  Graph g(edges, edges + num_edges, weights, num_nodes);
  property_map<Graph, edge_weight_t>::type weightmap = get(edge_weight, g);

  std::vector < graph_traits < Graph >::vertex_descriptor > p(num_vertices(g));

  prim_minimum_spanning_tree(g, &p[0]);

  delete edges;
  delete weights;

  for (std::size_t i = 0; i != p.size(); ++i) {
	  if (i == p[i]) continue;

	  result.insert(temp[i], temp[p[i]]);
  }

  return true;
}
