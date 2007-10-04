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

import org.eclipse.gef.commands.Command;
import org.fritzing.FritzingMessages;
import org.fritzing.model.FritzingGuide;
import org.fritzing.model.FritzingRuler;

/**
 * @author Pratik Shah
 */
public class CreateGuideCommand extends Command {

private FritzingGuide guide;
private FritzingRuler parent;
private int position;

public CreateGuideCommand(FritzingRuler parent, int position) {
	super(FritzingMessages.CreateGuideCommand_Label);
	this.parent = parent;
	this.position = position;
}

public boolean canUndo() {
	return true;
}

public void execute() {
	if (guide == null)
		guide = new FritzingGuide(!parent.isHorizontal());
	guide.setPosition(position);
	parent.addGuide(guide);
}

public void undo() {
	parent.removeGuide(guide);
}

}
