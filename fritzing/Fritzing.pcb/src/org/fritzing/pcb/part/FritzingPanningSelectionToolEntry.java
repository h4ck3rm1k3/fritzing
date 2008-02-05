/*
 * (c) Fachhochschule Potsdam
 */

package org.fritzing.pcb.part;

import org.eclipse.gef.palette.PanningSelectionToolEntry;
import org.eclipse.gef.tools.PanningSelectionTool;

public class FritzingPanningSelectionToolEntry extends
		PanningSelectionToolEntry {
	

	public FritzingPanningSelectionToolEntry() {
		this(null);
	}

	public FritzingPanningSelectionToolEntry(String label) {
		this(label, null);
	}
	
	public FritzingPanningSelectionToolEntry(String label, String shortDesc) {
		super(label, shortDesc);
		setToolClass(FritzingPanningSelectionTool.class);
	}

}
