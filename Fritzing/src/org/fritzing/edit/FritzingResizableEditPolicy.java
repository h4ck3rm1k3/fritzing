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

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.geometry.Rectangle;

import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.LayerConstants;
import org.eclipse.gef.editpolicies.ResizableEditPolicy;

import org.fritzing.figures.AndGateFeedbackFigure;
import org.fritzing.figures.CircuitFeedbackFigure;
import org.fritzing.figures.GroundFeedbackFigure;
import org.fritzing.figures.LEDFeedbackFigure;
import org.fritzing.figures.LabelFeedbackFigure;
import org.fritzing.figures.LiveOutputFeedbackFigure;
import org.fritzing.figures.FritzingColorConstants;
import org.fritzing.figures.FritzingFlowFeedbackFigure;
import org.fritzing.figures.OrGateFeedbackFigure;
import org.fritzing.figures.XOrGateFeedbackFigure;
import org.fritzing.model.AndGate;
import org.fritzing.model.Circuit;
import org.fritzing.model.GroundOutput;
import org.fritzing.model.LED;
import org.fritzing.model.LiveOutput;
import org.fritzing.model.FritzingFlowContainer;
import org.fritzing.model.FritzingLabel;
import org.fritzing.model.OrGate;
import org.fritzing.model.XORGate;


/**
 * 
 */
public class FritzingResizableEditPolicy
	extends ResizableEditPolicy
{

/**
 * Creates the figure used for feedback.
 * @return the new feedback figure
 */
protected IFigure createDragSourceFeedbackFigure() {
	IFigure figure = createFigure((GraphicalEditPart)getHost(), null);
	
	figure.setBounds(getInitialFeedbackBounds());
	addFeedback(figure);
	return figure;
}

protected IFigure createFigure(GraphicalEditPart part, IFigure parent) {
	IFigure child = getCustomFeedbackFigure(part.getModel());
		
	if (parent != null)
		parent.add(child);

	Rectangle childBounds = part.getFigure().getBounds().getCopy();
	
	IFigure walker = part.getFigure().getParent();
	
	while (walker != ((GraphicalEditPart)part.getParent()).getFigure()) {
		walker.translateToParent(childBounds);
		walker = walker.getParent();
	}
	
	child.setBounds(childBounds);
	
	Iterator i = part.getChildren().iterator();
	
	while (i.hasNext())
		createFigure((GraphicalEditPart)i.next(), child);
	
	return child;
}

protected IFigure getCustomFeedbackFigure(Object modelPart) {
	IFigure figure; 
	
	if (modelPart instanceof Circuit)
		figure = new CircuitFeedbackFigure();
	else if (modelPart instanceof FritzingFlowContainer)
		figure = new FritzingFlowFeedbackFigure();		
	else if (modelPart instanceof FritzingLabel)
		figure = new LabelFeedbackFigure();
	else if (modelPart instanceof LED) 
		figure = new LEDFeedbackFigure();
	else if (modelPart instanceof OrGate)
		figure = new OrGateFeedbackFigure();
	else if (modelPart instanceof XORGate)
		figure = new XOrGateFeedbackFigure();
	else if (modelPart instanceof GroundOutput)
		figure = new GroundFeedbackFigure();
	else if (modelPart instanceof LiveOutput)
		figure = new LiveOutputFeedbackFigure();
	else if (modelPart instanceof AndGate) 
		figure = new AndGateFeedbackFigure();
	else {
		figure = new RectangleFigure();
		((RectangleFigure)figure).setXOR(true);
		((RectangleFigure)figure).setFill(true);
		figure.setBackgroundColor(FritzingColorConstants.ghostFillColor);
		figure.setForegroundColor(ColorConstants.white);
	}
	
	return figure;
}

/**
 * Returns the layer used for displaying feedback.
 *  
 * @return the feedback layer
 */
protected IFigure getFeedbackLayer() {
	return getLayer(LayerConstants.SCALED_FEEDBACK_LAYER);
}

/**
 * @see org.eclipse.gef.editpolicies.NonResizableEditPolicy#getInitialFeedbackBounds()
 */
protected Rectangle getInitialFeedbackBounds() {
	return getHostFigure().getBounds();
}

}
