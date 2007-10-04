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
import org.eclipse.gef.EditPartFactory;

import org.fritzing.model.Circuit;
import org.fritzing.model.Gate;
import org.fritzing.model.LED;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingFlowContainer;
import org.fritzing.model.FritzingLabel;
import org.fritzing.model.SimpleOutput;
import org.fritzing.model.Wire;

public class GraphicalPartFactory
	implements EditPartFactory
{

public EditPart createEditPart(EditPart context, Object model) {
	EditPart child = null;
	
	if (model instanceof FritzingFlowContainer)
		child = new FritzingFlowContainerEditPart();
	else if (model instanceof Wire)
		child = new WireEditPart();
	else if (model instanceof LED)
		child = new LEDEditPart();
	else if (model instanceof FritzingLabel)
		child = new FritzingLabelEditPart();
	else if (model instanceof Circuit)
		child = new CircuitEditPart();
	else if (model instanceof Gate)
		child = new GateEditPart();
	else if (model instanceof SimpleOutput)
		child = new OutputEditPart();
	//Note that subclasses of FritzingDiagram have already been matched above, like Circuit
	else if (model instanceof FritzingDiagram)
		child = new FritzingDiagramEditPart();
	child.setModel(model);
	return child;
}

}
