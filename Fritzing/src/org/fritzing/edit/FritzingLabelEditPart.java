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

import java.beans.PropertyChangeEvent;

import org.eclipse.draw2d.IFigure;
import org.eclipse.gef.AccessibleEditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.RequestConstants;
import org.eclipse.swt.accessibility.AccessibleControlEvent;
import org.eclipse.swt.accessibility.AccessibleEvent;
import org.fritzing.FritzingMessages;
import org.fritzing.figures.StickyNoteFigure;
import org.fritzing.model.FritzingLabel;

public class FritzingLabelEditPart
	extends FritzingEditPart
{
	
protected AccessibleEditPart createAccessible() {
	return new AccessibleGraphicalEditPart(){
		public void getValue(AccessibleControlEvent e) {
			e.result = getFritzingLabel().getLabelContents();
		}

		public void getName(AccessibleEvent e) {
			e.result = FritzingMessages.FritzingPlugin_Tool_CreationTool_FritzingLabel;
		}
	};
}

protected void createEditPolicies(){
	super.createEditPolicies();
	installEditPolicy(EditPolicy.GRAPHICAL_NODE_ROLE, null);		
	installEditPolicy(EditPolicy.DIRECT_EDIT_ROLE, new LabelDirectEditPolicy());
	installEditPolicy(EditPolicy.COMPONENT_ROLE,new FritzingLabelEditPolicy()); 
}

protected IFigure createFigure() {
	StickyNoteFigure label = new StickyNoteFigure();
	return label;
}

private FritzingLabel getFritzingLabel(){
	return (FritzingLabel)getModel();
}

private void performDirectEdit(){
	new FritzingLabelEditManager(this,
			new LabelCellEditorLocator((StickyNoteFigure)getFigure())).show();
}

public void performRequest(Request request){
	if (request.getType() == RequestConstants.REQ_DIRECT_EDIT)
		performDirectEdit();
}

public void propertyChange(PropertyChangeEvent evt){
	if (evt.getPropertyName().equalsIgnoreCase("labelContents"))//$NON-NLS-1$
		refreshVisuals();
	else
		super.propertyChange(evt);
}

protected void refreshVisuals() {
	((StickyNoteFigure)getFigure()).setText(getFritzingLabel().getLabelContents());
	super.refreshVisuals();
}

}
