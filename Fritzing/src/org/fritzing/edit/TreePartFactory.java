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

import org.fritzing.model.LED;
import org.fritzing.model.FritzingDiagram;

public class TreePartFactory
	implements EditPartFactory
{

public EditPart createEditPart(EditPart context, Object model) {
	if (model instanceof LED)
		return new FritzingTreeEditPart(model);
	if (model instanceof FritzingDiagram)
		return new FritzingContainerTreeEditPart(model);
	return new FritzingTreeEditPart(model);
}

}
