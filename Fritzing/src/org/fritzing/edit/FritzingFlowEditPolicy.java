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


import java.util.Iterator;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.RequestConstants;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.ChangeBoundsRequest;
import org.eclipse.gef.requests.CreateRequest;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingFlowContainer;
import org.fritzing.model.FritzingSubpart;
import org.fritzing.model.commands.AddCommand;
import org.fritzing.model.commands.CloneCommand;
import org.fritzing.model.commands.CreateCommand;
import org.fritzing.model.commands.ReorderPartCommand;

public class FritzingFlowEditPolicy
	extends org.eclipse.gef.editpolicies.FlowLayoutEditPolicy
{

/**
 * Override to return the <code>Command</code> to perform an {@link
 * RequestConstants#REQ_CLONE CLONE}. By default, <code>null</code> is
 * returned.
 * @param request the Clone Request
 * @return A command to perform the Clone.
 */
protected Command getCloneCommand(ChangeBoundsRequest request) {
	CloneCommand clone = new CloneCommand();
	clone.setParent((FritzingDiagram)getHost().getModel());
	
	EditPart after = getInsertionReference(request);
	int index = getHost().getChildren().indexOf(after);
	
	Iterator i = request.getEditParts().iterator();
	GraphicalEditPart currPart = null;
	
	while (i.hasNext()) {
		currPart = (GraphicalEditPart)i.next();
		clone.addPart((FritzingSubpart)currPart.getModel(), index++);
	}
	
	return clone;
}
	
protected Command createAddCommand(EditPart child, EditPart after) {
	AddCommand command = new AddCommand();
	command.setChild((FritzingSubpart)child.getModel());
	command.setParent((FritzingFlowContainer)getHost().getModel());
	int index = getHost().getChildren().indexOf(after);
	command.setIndex(index);
	return command;
}

/**
 * @see org.eclipse.gef.editpolicies.LayoutEditPolicy#createChildEditPolicy(org.eclipse.gef.EditPart)
 */
protected EditPolicy createChildEditPolicy(EditPart child) {
	FritzingResizableEditPolicy policy = new FritzingResizableEditPolicy();
	policy.setResizeDirections(0);
	return policy;
}

protected Command createMoveChildCommand(EditPart child, EditPart after) {
	FritzingSubpart childModel = (FritzingSubpart)child.getModel();
	FritzingDiagram parentModel = (FritzingDiagram)getHost().getModel();
	int oldIndex = getHost().getChildren().indexOf(child);
	int newIndex = getHost().getChildren().indexOf(after);
	if (newIndex > oldIndex)
		newIndex--;
	ReorderPartCommand command = new ReorderPartCommand(childModel, parentModel, newIndex);
	return command;
}

protected Command getCreateCommand(CreateRequest request) {
	CreateCommand command = new CreateCommand();
	EditPart after = getInsertionReference(request);
	command.setChild((FritzingSubpart)request.getNewObject());
	command.setParent((FritzingFlowContainer)getHost().getModel());
	int index = getHost().getChildren().indexOf(after);
	command.setIndex(index);
	return command;
}

}
