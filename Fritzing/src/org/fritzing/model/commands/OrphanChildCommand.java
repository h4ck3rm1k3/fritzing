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

import java.util.List;

import org.eclipse.draw2d.geometry.Point;
import org.eclipse.gef.commands.Command;
import org.fritzing.FritzingMessages;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingSubpart;

public class OrphanChildCommand
	extends Command
{

private Point oldLocation;
private FritzingDiagram diagram;
private FritzingSubpart child;
private int index;

public OrphanChildCommand () {
	super(FritzingMessages.OrphanChildCommand_Label);
}

public void execute() {
	List children = diagram.getChildren();
	index = children.indexOf(child);
	oldLocation = child.getLocation();
	diagram.removeChild(child);
}

public void redo() {
	diagram.removeChild(child);
}

public void setChild(FritzingSubpart child) {
	this.child = child;
}

public void setParent(FritzingDiagram parent) { 
	diagram = parent;
}

public void undo() {
	child.setLocation(oldLocation);
	diagram.addChild(child, index);
}

}
