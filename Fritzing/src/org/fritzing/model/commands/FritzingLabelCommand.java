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

import org.eclipse.gef.commands.Command;

import org.fritzing.model.FritzingLabel;

public class FritzingLabelCommand
	extends Command
{

private String newName, oldName;
private FritzingLabel label;

public FritzingLabelCommand(FritzingLabel l, String s) {
	label = l;
	if (s != null)
		newName = s;
	else
		newName = "";  //$NON-NLS-1$
}

public void execute() {
	oldName = label.getLabelContents();
	label.setLabelContents(newName);
}

public void undo() {
	label.setLabelContents(oldName);
}

}
