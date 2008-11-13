#!/usr/bin/env groovy

class BreadboardFixer {
	/*** geometry ***/
	def longerRows = ("A".."J").toList()
	def rows = ["W","X",*longerRows,"Y","Z"]
	def shorterRows = rows - longerRows
	def midRowConn = longerRows[longerRows.size().intdiv(2)]

	def specificShorterCols = [63,64,30,31,61,62]
	def cols = [63,64,(1..62).toList()].flatten()
	def midColConn = specificShorterCols[specificShorterCols.size().intdiv(2)-1]

	def genericShorterCols = cols.findAll{ col ->
		/* Translate the cols in 1..60 to an unified coordinate system,
		 * where all cols mod 6 == 0 are short */
		int trans = col < /*ssc.mid*/ midColConn ? 0 : 1
		!specificShorterCols.contains(col) /* to avoid duplicates */ && ((col-trans)%6) == 0
	}
	def shorterCols = [*specificShorterCols,*genericShorterCols].sort()


	boolean isConnectorTag(Node g) {
		return g.children().size()==4 //&& g.linearGradient.size() == 2 && g.circle.size() == 2
	}

	boolean isHole(String row, int col) {
		!(shorterRows.contains(row) && shorterCols.contains(col))
	}

	Node newConnector(String id) {
		new XmlParser().parseText("""
			<connector type="female" id="$id" name="$id" >
			    <description>breadboard socket</description>
			    <views>
				<breadboardView>
				<pin layer="breadboardbreadboard" svgId="${id}pin" />
				</breadboardView>
			       <schematicView>
			       <pin layer="unknown" svgId="${id}pin" />
				 </schematicView>
				<pcbView>
				<pin layer="unknown" svgId="${id}pin" />
				</pcbView>
			    </views>
			</connector>""")
	}

	Node newBusNodeMember(String id) {
		new XmlParser().parseText("<nodeMember connectorId='${id}' />")
	}
	
	String busName(String i, int j) {
		def name
		int number
		if(shorterRows.contains(i)) { //W X Y Z
			name = i
			if(cols.indexOf(j) < cols.indexOf(midColConn)) {
				return "$name-1"
			} else {
				return "$name-2"
			}
		} else {
			name = j
			if(rows.indexOf(i) < rows.indexOf(midRowConn)) {
				return "$name-1"
			} else {
				return "$name-2"
			}
		}
		return null
	}

	Node newBus(String name) {
		new XmlParser().parseText("<bus id='bus$name' />")
	}

	void writeFile(Node node, String filePath) {
		def writer = new File(filePath).newWriter()
		new XmlNodePrinter(new PrintWriter(writer)).print(node)
	}

	/*** help structure ***/
	/* boolean matrix[i][j] where i in rows & j in cols, with -1 == false else true
	 * and if matrix[i][j] > -1 then matrix[i][j] is also the index of the
	 * <g/> tag in the svg file that belongs to the connector == breadboard(rows[i],cols[j]) */
	def matrix() {
		def idxMatrix = [:]
		int gTagIdx = 0

		/* It's important to iterate first through the cols, because
		 * that's how the svg was generated */
		for(j in cols) {
			for(i in rows) {
				if(!idxMatrix[i]) idxMatrix[i] = [:]
				idxMatrix[i][j] = isHole(i,j) ? gTagIdx++ : -1
			}
		}
		// cross your fingers
		assert gTagIdx == svgConns.size()
		//all conectors indexed

		return idxMatrix
	}

	def busesMatrix() {
		def busesMatrix = [:]

		for(i in rows) {
			busesMatrix[i] = [:]
			for(j in cols) {
				String bname = busName(i,j)
				if(!busesMatrix[bname]) {
					busesMatrix[bname] = newBus(bname)
				}
			}
		}

		return busesMatrix
	}


	def svgConns

	public BreadboardFixer() {
		Node svgDom = new XmlParser().parse(new File("parts/svg/core/breadboard/breadboard.svg"))
		Node fzDom = new XmlParser().parse(new File("parts/core/breadboard.fz"))

		svgConns = svgDom.g[0].g.findAll{isConnectorTag(it)}
		assert svgConns.size() > 0

		def fzConns = fzDom.connectors[0]
		/* It's easier to remove the existing connectors from the fz file
		 * instead of making a special case for some arbitrary columns */
		assert fzConns.children().size() == 0

		def fzBuses = fzDom.buses[0]
		/* idem fzConns */
		assert fzBuses.children().size() == 0

		def idxMatrix = matrix()
		def buses = busesMatrix()
		
		for(j in cols) {
			for(i in rows) {
				String id = "$i$j"
				int tagIdx = idxMatrix[i][j]
				if(tagIdx > -1) {
					// a previous assertion should ensure that
					assert svgConns[tagIdx] != null

					svgConns[tagIdx]['@id']= "${id}pin"
					fzConns.children() << newConnector(id)

					// Buses already created
					buses[busName(i,j)].children() << newBusNodeMember(id)
				}
			}
		}

		for(busName in buses.keySet()) {
			if(buses[busName]) {
				fzBuses.children() << buses[busName]
			}
		}


		writeFile(svgDom,"breadboard.svg")
		writeFile(fzDom,"breadboard.fz")
	}
}

new BreadboardFixer()