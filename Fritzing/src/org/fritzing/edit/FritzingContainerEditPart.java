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

import java.util.List;

import org.eclipse.swt.accessibility.AccessibleEvent;

import org.eclipse.gef.AccessibleEditPart;
import org.eclipse.gef.EditPolicy;

import org.fritzing.model.FritzingDiagram;


/**
 * Provides support for Container EditParts.
 */
abstract public class FritzingContainerEditPart
	extends FritzingEditPart
{
protected AccessibleEditPart createAccessible() {
	return new AccessibleGraphicalEditPart(){
		public void getName(AccessibleEvent e) {
			e.result = getFritzingDiagram().toString();
		}
	};
}

/**
 * Installs the desired EditPolicies for this.
 */
protected void createEditPolicies() {
	super.createEditPolicies();
	installEditPolicy(EditPolicy.CONTAINER_ROLE, new FritzingContainerEditPolicy());
}

/**
 * Returns the model of this as a FritzingDiagram.
 *
 * @return  FritzingDiagram of this.
 */
protected FritzingDiagram getFritzingDiagram() {
	return (FritzingDiagram)getModel();
}

/**
 * Returns the children of this through the model.
 *
 * @return  Children of this as a List.
 */
protected List getModelChildren() {
	return getFritzingDiagram().getChildren();
}

}
