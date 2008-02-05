package org.fritzing.pcb.export;

import org.fritzing.fritzing.IWireConnection;


public class EagleSCRNet {	
	public String netName = "";
	public IWireConnection source;
	public IWireConnection target;
	
	
	public EagleSCRNet(String netName) {
		super();
		this.netName = netName;
	}	
	
	public EagleSCRNet(String netName, IWireConnection source, IWireConnection target) {
		this.netName = netName;
		this.source = source;
		this.target = target;
	}
}
