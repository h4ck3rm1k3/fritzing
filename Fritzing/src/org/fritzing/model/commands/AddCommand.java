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

import org.fritzing.FritzingMessages;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingSubpart;

public class AddCommand extends
	org.eclipse.gef.commands.Command
{

private FritzingSubpart child;
private FritzingDiagram parent;
private int index = -1;

public AddCommand() {
	super(FritzingMessages.AddCommand_Label);
}

public void execute() {
	if( index < 0 )
		parent.addChild(child);
	else
		parent.addChild(child,index);
}

public FritzingDiagram getParent() {
	return parent;
}

public void redo() {
	if( index < 0 )
		parent.addChild(child);
	else
		parent.addChild(child,index);
}

public void setChild(FritzingSubpart subpart) {
	child = subpart;
}

public void setIndex(int i){
	index = i;
}

public void setParent(FritzingDiagram newParent) {
	parent = newParent;
}

public void undo() {
	parent.removeChild(child);
}

}
