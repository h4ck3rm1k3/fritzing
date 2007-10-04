/*******************************************************************************
 * Copyright (c) 2000, 2005 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package org.fritzing.model.commands;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.commands.Command;
import org.fritzing.FritzingMessages;
import org.fritzing.model.AndGate;
import org.fritzing.model.Circuit;
import org.fritzing.model.GroundOutput;
import org.fritzing.model.LED;
import org.fritzing.model.LiveOutput;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingFlowContainer;
import org.fritzing.model.FritzingGuide;
import org.fritzing.model.FritzingLabel;
import org.fritzing.model.FritzingSubpart;
import org.fritzing.model.OrGate;
import org.fritzing.model.Wire;
import org.fritzing.model.WireBendpoint;
import org.fritzing.model.XORGate;

public class CloneCommand
	extends Command
{

private List parts, newTopLevelParts, newConnections;
private FritzingDiagram parent;
private Map bounds, indices, connectionPartMap;
private ChangeGuideCommand vGuideCommand, hGuideCommand;
private FritzingGuide hGuide, vGuide;
private int hAlignment, vAlignment;

public CloneCommand() {
	super(FritzingMessages.CloneCommand_Label);
	parts = new LinkedList();
}

public void addPart(FritzingSubpart part, Rectangle newBounds) {
	parts.add(part);
	if (bounds == null) {
		bounds = new HashMap();
	}
	bounds.put(part, newBounds);
}

public void addPart(FritzingSubpart part, int index) {
	parts.add(part);
	if (indices == null) {
		indices = new HashMap();
	}
	indices.put(part, new Integer(index));
}

protected void clonePart(FritzingSubpart oldPart, FritzingDiagram newParent, Rectangle newBounds,
					  List newConnections, Map connectionPartMap, int index) {
	FritzingSubpart newPart = null;
	
	if (oldPart instanceof AndGate) {
		newPart = new AndGate();
	} else if (oldPart instanceof Circuit) {
		newPart = new Circuit();
	} else if (oldPart instanceof GroundOutput) {
		newPart = new GroundOutput();
	} else if (oldPart instanceof LED) {
		newPart = new LED();
		newPart.setPropertyValue(LED.P_VALUE, oldPart.getPropertyValue(LED.P_VALUE));
	} else if (oldPart instanceof LiveOutput) {
		newPart = new LiveOutput();
	} else if (oldPart instanceof FritzingLabel) {
		newPart = new FritzingLabel();
		((FritzingLabel)newPart).setLabelContents(((FritzingLabel)oldPart).getLabelContents());
	} else if (oldPart instanceof OrGate) {
		newPart = new OrGate();
	} else if (oldPart instanceof FritzingFlowContainer) {
		newPart = new FritzingFlowContainer();
	} else if (oldPart instanceof XORGate) {
		newPart = new XORGate();
	}
	
	if (oldPart instanceof FritzingDiagram) {
		Iterator i = ((FritzingDiagram)oldPart).getChildren().iterator();
		while (i.hasNext()) {
			// for children they will not need new bounds
			clonePart((FritzingSubpart)i.next(), (FritzingDiagram)newPart, null, 
					newConnections, connectionPartMap, -1);
		}
	}
	
	Iterator i = oldPart.getTargetConnections().iterator();
	while (i.hasNext()) {
		Wire connection = (Wire)i.next();
		Wire newConnection = new Wire();
		newConnection.setValue(connection.getValue());
		newConnection.setTarget(newPart);
		newConnection.setTargetTerminal(connection.getTargetTerminal());
		newConnection.setSourceTerminal(connection.getSourceTerminal());
		newConnection.setSource(connection.getSource());
	
		Iterator b = connection.getBendpoints().iterator();
		Vector newBendPoints = new Vector();
		
		while (b.hasNext()) {
			WireBendpoint bendPoint = (WireBendpoint)b.next();
			WireBendpoint newBendPoint = new WireBendpoint();
			newBendPoint.setRelativeDimensions(bendPoint.getFirstRelativeDimension(), 
					bendPoint.getSecondRelativeDimension());
			newBendPoint.setWeight(bendPoint.getWeight());
			newBendPoints.add(newBendPoint);
		}
		
		newConnection.setBendpoints(newBendPoints);
		newConnections.add(newConnection);
	}
	
	
	if (index < 0) {
		newParent.addChild(newPart);
	} else {
		newParent.addChild(newPart, index);
	}
	
	newPart.setSize(oldPart.getSize());

	
	if (newBounds != null) {
		newPart.setLocation(newBounds.getTopLeft());
	} else {
		newPart.setLocation(oldPart.getLocation());
	}
	
	// keep track of the new parts so we can delete them in undo
	// keep track of the oldpart -> newpart map so that we can properly attach
	// all connections.
	if (newParent == parent)
		newTopLevelParts.add(newPart);
	connectionPartMap.put(oldPart, newPart);
}

public void execute() {
	connectionPartMap = new HashMap();
	newConnections = new LinkedList();
	newTopLevelParts = new LinkedList();

	Iterator i = parts.iterator();
	
	FritzingSubpart part = null;
	while (i.hasNext()) {
		part = (FritzingSubpart)i.next();
		if (bounds != null && bounds.containsKey(part)) {
			clonePart(part, parent, (Rectangle)bounds.get(part), 
					newConnections, connectionPartMap, -1);	
		} else if (indices != null && indices.containsKey(part)) {
			clonePart(part, parent, null, newConnections, 
					connectionPartMap, ((Integer)indices.get(part)).intValue());
		} else {
			clonePart(part, parent, null, newConnections, connectionPartMap, -1);
		}
	}
	
	// go through and set the source of each connection to the proper source.
	Iterator c = newConnections.iterator();
	
	while (c.hasNext()) {
		Wire conn = (Wire)c.next();
		FritzingSubpart source = conn.getSource();
		if (connectionPartMap.containsKey(source)) {
			conn.setSource((FritzingSubpart)connectionPartMap.get(source));
			conn.attachSource();
			conn.attachTarget();
		}
	}
	
	if (hGuide != null) {
		hGuideCommand = new ChangeGuideCommand(
				(FritzingSubpart)connectionPartMap.get(parts.get(0)), true);
		hGuideCommand.setNewGuide(hGuide, hAlignment);
		hGuideCommand.execute();
	}
		
	if (vGuide != null) {
		vGuideCommand = new ChangeGuideCommand(
				(FritzingSubpart)connectionPartMap.get(parts.get(0)), false);
		vGuideCommand.setNewGuide(vGuide, vAlignment);
		vGuideCommand.execute();
	}
}

public void setParent(FritzingDiagram parent) {
	this.parent = parent;
}

public void redo() {
	for (Iterator iter = newTopLevelParts.iterator(); iter.hasNext();)
		parent.addChild((FritzingSubpart)iter.next());
	for (Iterator iter = newConnections.iterator(); iter.hasNext();) {
		Wire conn = (Wire) iter.next();
		FritzingSubpart source = conn.getSource();
		if (connectionPartMap.containsKey(source)) {
			conn.setSource((FritzingSubpart)connectionPartMap.get(source));
			conn.attachSource();
			conn.attachTarget();
		}
	}
	if (hGuideCommand != null)
		hGuideCommand.redo();
	if (vGuideCommand != null)
		vGuideCommand.redo();
}

public void setGuide(FritzingGuide guide, int alignment, boolean isHorizontal) {
	if (isHorizontal) {
		hGuide = guide;
		hAlignment = alignment;
	} else {
		vGuide = guide;
		vAlignment = alignment;
	}
}

public void undo() {
	if (hGuideCommand != null)
		hGuideCommand.undo();
	if (vGuideCommand != null)
		vGuideCommand.undo();
	for (Iterator iter = newTopLevelParts.iterator(); iter.hasNext();)
		parent.removeChild((FritzingSubpart)iter.next());
}

}
