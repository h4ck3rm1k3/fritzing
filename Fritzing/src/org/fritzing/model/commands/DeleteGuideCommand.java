/*******************************************************************************
 * Copyright (c) 2003, 2005 IBM Corporation and others.
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
import java.util.Map;

import org.eclipse.gef.commands.Command;
import org.fritzing.FritzingMessages;
import org.fritzing.model.FritzingGuide;
import org.fritzing.model.FritzingRuler;
import org.fritzing.model.FritzingSubpart;

/**
 * @author Pratik Shah
 */
public class DeleteGuideCommand 
	extends Command 
{

private FritzingRuler parent;
private FritzingGuide guide;
private Map oldParts;

public DeleteGuideCommand(FritzingGuide guide, FritzingRuler parent) {
	super(FritzingMessages.DeleteGuideCommand_Label);
	this.guide = guide;
	this.parent = parent;
}

public boolean canUndo() {
	return true;
}

public void execute() {
	oldParts = new HashMap(guide.getMap());
	Iterator iter = oldParts.keySet().iterator();
	while (iter.hasNext()) {
		guide.detachPart((FritzingSubpart)iter.next());
	}
	parent.removeGuide(guide);
}
public void undo() {
	parent.addGuide(guide);
	Iterator iter = oldParts.keySet().iterator();
	while (iter.hasNext()) {
		FritzingSubpart part = (FritzingSubpart)iter.next();
		guide.attachPart(part, ((Integer)oldParts.get(part)).intValue());
	}
}
}
