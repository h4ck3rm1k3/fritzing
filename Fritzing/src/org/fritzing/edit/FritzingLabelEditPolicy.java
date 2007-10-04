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
package org.fritzing.edit;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.fritzing.model.FritzingLabel;
import org.fritzing.model.commands.FritzingLabelCommand;

public class FritzingLabelEditPolicy 
	extends FritzingElementEditPolicy 
{

public Command getCommand(Request request) {
	if (NativeDropRequest.ID.equals(request.getType()))
		return getDropTextCommand((NativeDropRequest)request);
	return super.getCommand(request);
}

protected Command getDropTextCommand(NativeDropRequest request) {
	FritzingLabelCommand command = new FritzingLabelCommand((FritzingLabel)getHost().getModel(), (String)request.getData());
	return command;
}

public EditPart getTargetEditPart(Request request) {
	if (NativeDropRequest.ID.equals(request.getType()))
		return getHost();
	return super.getTargetEditPart(request);
}

}
