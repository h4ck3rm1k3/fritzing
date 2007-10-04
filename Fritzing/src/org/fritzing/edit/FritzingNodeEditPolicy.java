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

import org.eclipse.draw2d.Connection;
import org.eclipse.draw2d.ConnectionAnchor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PolylineConnection;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.LayerConstants;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.CreateConnectionRequest;
import org.eclipse.gef.requests.ReconnectRequest;
import org.fritzing.figures.FigureFactory;
import org.fritzing.figures.NodeFigure;
import org.fritzing.model.GroundOutput;
import org.fritzing.model.LiveOutput;
import org.fritzing.model.FritzingSubpart;
import org.fritzing.model.Wire;
import org.fritzing.model.commands.ConnectionCommand;

public class FritzingNodeEditPolicy
	extends org.eclipse.gef.editpolicies.GraphicalNodeEditPolicy
{

protected Connection createDummyConnection(Request req) {
	PolylineConnection conn = FigureFactory.createNewWire(null);
	return conn;
}

protected Command getConnectionCompleteCommand(CreateConnectionRequest request) {	
	ConnectionCommand command = (ConnectionCommand)request.getStartCommand();
	command.setTarget(getFritzingSubpart());
	ConnectionAnchor ctor = getFritzingEditPart().getTargetConnectionAnchor(request);
	if (ctor == null)
		return null;
	command.setTargetTerminal(getFritzingEditPart().mapConnectionAnchorToTerminal(ctor));
	return command;
}

protected Command getConnectionCreateCommand(CreateConnectionRequest request) {
	ConnectionCommand command = new ConnectionCommand();
	command.setWire(new Wire());
	command.setSource(getFritzingSubpart());
	ConnectionAnchor ctor = getFritzingEditPart().getSourceConnectionAnchor(request);
	command.setSourceTerminal(getFritzingEditPart().mapConnectionAnchorToTerminal(ctor));
	request.setStartCommand(command);
	return command;
}

/**
 * Feedback should be added to the scaled feedback layer.
 * @see org.eclipse.gef.editpolicies.GraphicalEditPolicy#getFeedbackLayer()
 */
protected IFigure getFeedbackLayer() {
	/*
	 * Fix for Bug# 66590
	 * Feedback needs to be added to the scaled feedback layer
	 */
	return getLayer(LayerConstants.SCALED_FEEDBACK_LAYER);
}



protected FritzingEditPart getFritzingEditPart() {
	return (FritzingEditPart) getHost();
}

protected FritzingSubpart getFritzingSubpart() {
	return (FritzingSubpart) getHost().getModel();
}

protected Command getReconnectTargetCommand(ReconnectRequest request) {
	if (getFritzingSubpart() instanceof LiveOutput || 
		getFritzingSubpart() instanceof GroundOutput)
			return null;
	
	ConnectionCommand cmd = new ConnectionCommand();
	cmd.setWire((Wire)request.getConnectionEditPart().getModel());

	ConnectionAnchor ctor = getFritzingEditPart().getTargetConnectionAnchor(request);
	cmd.setTarget(getFritzingSubpart());
	cmd.setTargetTerminal(getFritzingEditPart().mapConnectionAnchorToTerminal(ctor));
	return cmd;
}

protected Command getReconnectSourceCommand(ReconnectRequest request) {
	ConnectionCommand cmd = new ConnectionCommand();
	cmd.setWire((Wire)request.getConnectionEditPart().getModel());

	ConnectionAnchor ctor = getFritzingEditPart().getSourceConnectionAnchor(request);
	cmd.setSource(getFritzingSubpart());
	cmd.setSourceTerminal(getFritzingEditPart().mapConnectionAnchorToTerminal(ctor));
	return cmd;
}

protected NodeFigure getNodeFigure() {
	return (NodeFigure)((GraphicalEditPart)getHost()).getFigure();
}

}
