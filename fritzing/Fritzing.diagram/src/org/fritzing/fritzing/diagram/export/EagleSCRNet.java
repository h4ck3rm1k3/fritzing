package org.fritzing.fritzing.diagram.export;

import org.fritzing.fritzing.Terminal;


public class EagleSCRNet {	
	public String netName = "";
	public Terminal source;
	public Terminal target;
	
	public CoordPair sourcePos = new CoordPair();
	public CoordPair targetPos = new CoordPair();
	

	public EagleSCRNet(String netName) {
		super();
		this.netName = netName;
	}	
	
	public EagleSCRNet(String netName, Terminal source, Terminal target) {
		this.netName = netName;
		this.source = source;
		this.target = target;
	}
}
