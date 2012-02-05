/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include "graphutils.h"
#include "../fsvgrenderer.h"
#include "../items/wire.h"
#include "../items/jumperitem.h"

#ifdef _MSC_VER 
#pragma warning(push) 
#pragma warning(disable:4100)			// disable scary-looking compiler warnings in Boost library
#pragma warning(disable:4181)	
#pragma warning(disable:4503)
#endif

#include <boost/config.hpp>
#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
// #include <boost/graph/kolmogorov_max_flow.hpp>  // kolmogorov_max_flow is probably more efficient, but it doesn't compile
#include <boost/graph/edmonds_karp_max_flow.hpp>
#include <boost/graph/adjacency_list.hpp>


#ifdef _MSC_VER 
#pragma warning(pop)					// restore warning state
#endif

struct SimpleEdge {
	int head;
	int tail;
	int weight;
};

void GraphUtils::minCut(QList<ConnectorItem *> & partConnectorItems, QVector< QVector< QList<ConnectorItem *> > > & buses, ConnectorItem * source, ConnectorItem * sink, QList<LongPair> & minCut) 
{
	// this helped:  http://boost.2283326.n4.nabble.com/graph-edmund-karp-max-flow-vs-kolmogorov-max-flow-color-map-td2565611.html
	
	using namespace boost;

	typedef adjacency_list_traits < vecS, vecS, directedS > Traits;
	typedef property < vertex_index_t, long,
			property < vertex_color_t, boost::default_color_type > > VERTEX;
	typedef property < edge_capacity_t, long,										
			property < edge_residual_capacity_t, long,								
			property < edge_reverse_t, Traits::edge_descriptor > > > EDGE;
	typedef adjacency_list < listS, vecS, directedS, VERTEX, EDGE > Graph;

	Graph g;

	property_map < Graph, edge_capacity_t >::type capacity = get(edge_capacity, g);
	property_map < Graph, edge_residual_capacity_t >::type residual_capacity = get(edge_residual_capacity, g);
	property_map < Graph, edge_reverse_t >::type reverse = get(edge_reverse, g);

	property_map < Graph, vertex_color_t >::type color = get(vertex_color, g);
	property_map < Graph, vertex_index_t >::type index = get(vertex_index, g);

	Traits::vertex_descriptor s, t;

	int count = partConnectorItems.count();
	int counts[3];
	counts[0] = 0;
	ViewIdentifierClass::ViewIdentifier viewIdentifier = partConnectorItems.at(0)->attachedToViewIdentifier();
	int ix = 1;
	for (int i = 1; i < count; i++) {
		if (partConnectorItems.at(i)->attachedToViewIdentifier() != viewIdentifier) {
			counts[ix++] = i;
			if (ix == 3) break;
			viewIdentifier = partConnectorItems.at(i)->attachedToViewIdentifier();
		}
	}

	std::vector<Traits::vertex_descriptor> verts(count);
	for (int i = 0; i < count; ++i) {
		verts[i] = add_vertex(g);
		partConnectorItems.at(i)->debugInfo(QString("%1 part").arg(i));
	}

	QVector<SimpleEdge> edges;

	int l[3][2];
	l[0][0] = 0;
	l[0][1] = counts[1];
	l[1][0] = counts[1];
	l[1][1] = counts[2];
	l[2][0] = counts[2];
	l[2][1] = count;

	// connect same connectors across views
	for (long i = 0; i < counts[1]; i++) {
		ConnectorItem * ci = partConnectorItems.at(i);
		for (int lix = 1; lix < 3; lix++) {
			for (long j = l[lix][0]; j < l[lix][1]; j++) {
				ConnectorItem * cj = partConnectorItems.at(j);
				if (ci->attachedToID() != cj->attachedToID()) continue;
				if (ci->connectorSharedID().compare(cj->connectorSharedID()) == 0) {
					SimpleEdge se;
					se.head = i;
					se.tail = j;
					se.weight = 1000;
					edges.append(se);
					break;
				}
			}
		}
	}

	for (long lix = 0; lix < 3; lix++) {
		long start = l[lix][0];
		long end = l[lix][1];
		for (long i = start; i < end; i++) {
			ConnectorItem * ci = partConnectorItems[i];
			if (ci == source) {
				s = verts[i];
				ci->debugInfo(QString("source %1").arg(i));
			}
			else if (ci == sink) {
				t = verts[i];
				ci->debugInfo(QString("sink %1").arg(i));
			}
			for (long j = i + 1; j < end; j++) {
				ConnectorItem * cj = partConnectorItems[j];
				int weight = 0;
				if (ci->getCrossLayerConnectorItem() == cj) {
					weight = 1000;
				}
				else if (ci->attachedTo() == cj->attachedTo()) {
					if (ci->bus() != NULL) {
						if (ci->bus() == cj->bus()) {
							weight = 1000;
						}
					}
				}
				if (weight == 0) {
					if (ci->connectedDirectlyTo(cj, buses[j][i])) {
						weight = ((start == 0) ? 1000 : 1);			// privilege this view over the others, so only they get cut
						ci->debugInfo(QString("conn %1").arg(weight));
						cj->debugInfo("       ");
					}
				}
				if (weight) {
					SimpleEdge se;
					se.head = i;
					se.tail = j;
					se.weight = weight;
					edges.append(se);
				}
				buses[i][j] = buses[j][i];
			}
		}
	}

	foreach(SimpleEdge se, edges) {
		Traits::edge_descriptor e1, e2;
		bool in1, in2;
		tie(e1, in1) = add_edge(verts[se.head], verts[se.tail], g);
		tie(e2, in2) = add_edge(verts[se.tail], verts[se.head], g);
		capacity[e1] = se.weight;
		capacity[e2] = se.weight;
		reverse[e1] = e2;
		reverse[e2] = e1;
		partConnectorItems.at(se.head)->debugInfo(QString("head %1").arg(se.weight));
		partConnectorItems.at(se.tail)->debugInfo("\ttail");
	}

	
	// if color_map parameter not specified, colors are not set
    long flow = edmonds_karp_max_flow(g, s, t, color_map(color)); 
	DebugDialog::debug(QString("flow %1, s%2, t%3").arg(flow).arg(index(s)).arg(index(t)));
	for (int i = 0; i < count; ++i) {
		DebugDialog::debug(QString("index %1 %2").arg(index(verts[i])).arg(color(verts[i])));
	}

	typedef property_traits<property_map < Graph, vertex_color_t >::type>::value_type tColorValue;
    typedef boost::color_traits<tColorValue> tColorTraits; 
	foreach (SimpleEdge se, edges) {
		bool addIt = false;
		if (color(verts[se.head]) == tColorTraits::white() && color(verts[se.tail]) != tColorTraits::white()) {
			addIt = true;
		}
		else if (color(verts[se.head]) != tColorTraits::white() && color(verts[se.tail]) == tColorTraits::white()) {
			addIt = true;
		}
		if (addIt) {
			LongPair lp(se.head, se.tail);
			minCut << lp;
			DebugDialog::debug(QString("edge %1 %2 w:%3").arg(se.head).arg(se.tail).arg(se.weight));

		}
	}     
}


bool GraphUtils::chooseRatsnestGraph(const QList<ConnectorItem *> * partConnectorItems, ViewGeometry::WireFlags flags, ConnectorPairHash & result) {
	using namespace boost;
	typedef adjacency_list < vecS, vecS, undirectedS, property<vertex_distance_t, double>, property < edge_weight_t, double > > Graph;
	typedef std::pair < int, int >E;

	if (partConnectorItems->count() < 2) return false;

	QList <ConnectorItem *> temp(*partConnectorItems);

	//DebugDialog::debug("__________________");
	int tix = 0;
	while (tix < temp.count()) {
		ConnectorItem * connectorItem = temp[tix++];
		//connectorItem->debugInfo("check cross");
		ConnectorItem * crossConnectorItem = connectorItem->getCrossLayerConnectorItem();
		if (crossConnectorItem) {
			// it doesn't matter which one  on which layer we remove
			// when we check equal potential both of them will be returned
			//crossConnectorItem->debugInfo("\tremove cross");
			temp.removeOne(crossConnectorItem);
		}
	}

	QList<QPointF> locs;
	foreach (ConnectorItem * connectorItem, temp) {
		locs << connectorItem->sceneAdjustedTerminalPoint(NULL);
	}

	QList < QList<ConnectorItem *> > wiredTo;

	int num_nodes = temp.count();
	int num_edges = num_nodes * (num_nodes - 1) / 2;
	E * edges = new E[num_edges];
	double * weights = new double[num_edges];
	int ix = 0;
	QVector< QVector<double> > reverseWeights(num_nodes, QVector<double>(num_nodes, 0));
	for (int i = 0; i < num_nodes; i++) {
		ConnectorItem * c1 = temp.at(i);
		//c1->debugInfo("c1");
		for (int j = i + 1; j < num_nodes; j++) {
			edges[ix].first = i;
			edges[ix].second = j;
			ConnectorItem * c2 = temp.at(j);
			if ((c1->attachedTo() == c2->attachedTo()) && (c1->bus() != NULL) && (c1->bus() == c2->bus())) {
				weights[ix++] = 0;
				continue;
			}

			bool already = false;
			bool checkWiredTo = true;
			foreach (QList<ConnectorItem *> list, wiredTo) {
				if (list.contains(c1)) {
					checkWiredTo = false;
					if (list.contains(c2)) {
						weights[ix++] = 0;
						already = true;
					}
					break;
				}
			}
			if (already) continue;

			//c2->debugInfo("\tc2");

			if (checkWiredTo) {
				QList<ConnectorItem *> cwConnectorItems;
				cwConnectorItems.append(c1);
				ConnectorItem::collectEqualPotential(cwConnectorItems, true, flags);
				wiredTo.append(cwConnectorItems);
				//foreach (ConnectorItem * cx, cwConnectorItems) {
					//cx->debugInfo("\t\tcx");
				//}
				if (cwConnectorItems.contains(c2)) {
					weights[ix++] = 0;
					continue;
				}
			}

			//DebugDialog::debug("c2 not eliminated");
			double dx = locs[i].x() - locs[j].x();
			double dy = locs[i].y() - locs[j].y();
			weights[ix++] = reverseWeights[i][j] = reverseWeights[j][i] = (dx * dx) + (dy * dy);
		}
	}

	Graph g(edges, edges + num_edges, weights, num_nodes);
	property_map<Graph, edge_weight_t>::type weightmap = get(edge_weight, g);

	std::vector < graph_traits < Graph >::vertex_descriptor > p(num_vertices(g));

	prim_minimum_spanning_tree(g, &p[0]);

	for (std::size_t i = 0; i != p.size(); ++i) {
		if (i == p[i]) continue;
		if (reverseWeights[i][p[i]] == 0) continue;

		result.insert(temp[i], temp[p[i]]);
	}

	delete edges;
	delete weights;

	return true;
}

bool GraphUtils::scoreOneNet(QList<ConnectorItem *> & partConnectorItems, ViewGeometry::WireFlags myTrace, RoutingStatus & routingStatus) {
	using namespace boost;

	int num_nodes = partConnectorItems.count();

	typedef property < vertex_index_t, std::size_t > Index;
	typedef adjacency_list < listS, listS, directedS, Index > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_t;
	typedef graph_traits < graph_t >::edge_descriptor edge_t;

	graph_t G;
	std::vector < vertex_t > verts(num_nodes);
	for (int i = 0; i < num_nodes; ++i) {
		verts[i] = add_vertex(Index(i), G);
	}

	//std::pair<int, int> pair;
	bool gotUserConnection = false;
	for (int i = 0; i < num_nodes; i++) {
		add_edge(verts[i], verts[i], G);
		ConnectorItem * from = partConnectorItems[i];
		for (int j = i + 1; j < num_nodes; j++) {
			ConnectorItem * to = partConnectorItems[j];

			if (from->isCrossLayerConnectorItem(to)) {
				add_edge(verts[i], verts[j], G);
				add_edge(verts[j], verts[i], G);
				continue;
			}

			if (to->attachedTo() != from->attachedTo()) {
				gotUserConnection = true;
				continue;
			}

			if ((to->bus() != NULL) && (to->bus() == from->bus())) {	
				add_edge(verts[i], verts[j], G);
				add_edge(verts[j], verts[i], G);
				continue;
			}

			gotUserConnection = true;
		}
	}

	if (!gotUserConnection) {
		return false;
	}

	routingStatus.m_netCount++;

	for (int i = 0; i < num_nodes; i++) {
		ConnectorItem * fromConnectorItem = partConnectorItems[i];
		if (fromConnectorItem->attachedToItemType() == ModelPart::Jumper) {
			routingStatus.m_jumperItemCount++;				
		}
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
				continue;
			}

			Wire * wire = qobject_cast<Wire *>(toConnectorItem->attachedTo());
			if (wire == NULL) continue;

			if (!(wire->getViewGeometry().wireFlags() & myTrace)) {
				// don't add edge if the connection isn't traced with my kind of trace
				continue;
			}

			QList<Wire *> wires;
			QList<ConnectorItem *> ends;
			wire->collectChained(wires, ends);
			foreach (ConnectorItem * end, ends) {
				if (end == fromConnectorItem) continue;

				int j = partConnectorItems.indexOf(end);
				if (j >= 0) {
					add_edge(verts[i], verts[j], G);
					add_edge(verts[j], verts[i], G);
				}
			}
		}
	}

	adjacency_list <> TC;
	transitive_closure(G, TC);

	QVector<bool> check(num_nodes, true);
	bool anyMissing = false;
	for (int i = 0; i < num_nodes - 1; i++) {
		if (!check[i]) continue;

		check[i] = false;
		bool missingOne = false;
		for (int j = i + 1; j < num_nodes; j++) {
			if (!check[j]) continue;

			if (edge(i, j, TC).second) {
				check[j] = false;
			}
			else {
				// we can minimally span the set with n-1 wires, so even if multiple connections are missing from a given connector, count it as one
				anyMissing = missingOne = true;
				
				//ConnectorItem * ci = partConnectorItems.at(i);
				//ConnectorItem * cj = partConnectorItems.at(j);
				//ci->debugInfo("score one ci");
				//cj->debugInfo("\t\tcj");
				
			}
		}
		if (missingOne) {
			routingStatus.m_connectorsLeftToRoute++;
		}
	}

	if (!anyMissing) {
		routingStatus.m_netRoutedCount++;
	}

	return true;
}


