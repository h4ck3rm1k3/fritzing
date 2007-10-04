/*******************************************************************************
 * Copyright (c) 2001, 2005 IBM Corporation and others.
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
import java.util.List;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.XYLayout;
import org.eclipse.draw2d.geometry.Insets;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.LayerConstants;
import org.eclipse.gef.Request;
import org.eclipse.gef.RequestConstants;
import org.eclipse.gef.SnapToGuides;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gef.editpolicies.ResizableEditPolicy;
import org.eclipse.gef.requests.ChangeBoundsRequest;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gef.rulers.RulerProvider;
import org.fritzing.FritzingMessages;
import org.fritzing.figures.CircuitFeedbackFigure;
import org.fritzing.figures.LabelFeedbackFigure;
import org.fritzing.figures.FritzingColorConstants;
import org.fritzing.figures.FritzingFlowFeedbackFigure;
import org.fritzing.model.Circuit;
import org.fritzing.model.LED;
import org.fritzing.model.FritzingDiagram;
import org.fritzing.model.FritzingFlowContainer;
import org.fritzing.model.FritzingGuide;
import org.fritzing.model.FritzingLabel;
import org.fritzing.model.FritzingSubpart;
import org.fritzing.model.commands.AddCommand;
import org.fritzing.model.commands.ChangeGuideCommand;
import org.fritzing.model.commands.CloneCommand;
import org.fritzing.model.commands.CreateCommand;
import org.fritzing.model.commands.SetConstraintCommand;

public class FritzingXYLayoutEditPolicy
	extends org.eclipse.gef.editpolicies.XYLayoutEditPolicy
{

public FritzingXYLayoutEditPolicy(XYLayout layout) {
	super();
	setXyLayout(layout);
}

protected Command chainGuideAttachmentCommand(
		Request request, FritzingSubpart part, Command cmd, boolean horizontal) {
	Command result = cmd;
	
	// Attach to guide, if one is given
	Integer guidePos = (Integer)request.getExtendedData()
			.get(horizontal ? SnapToGuides.KEY_HORIZONTAL_GUIDE
			                : SnapToGuides.KEY_VERTICAL_GUIDE);
	if (guidePos != null) {
		int alignment = ((Integer)request.getExtendedData()
				.get(horizontal ? SnapToGuides.KEY_HORIZONTAL_ANCHOR
				                : SnapToGuides.KEY_VERTICAL_ANCHOR)).intValue();
		ChangeGuideCommand cgm = new ChangeGuideCommand(part, horizontal);
		cgm.setNewGuide(findGuideAt(guidePos.intValue(), horizontal), alignment);
		result = result.chain(cgm);
	}

	return result;
}

protected Command chainGuideDetachmentCommand(Request request, FritzingSubpart part,
		Command cmd, boolean horizontal) {
	Command result = cmd;
	
	// Detach from guide, if none is given
	Integer guidePos = (Integer)request.getExtendedData()
			.get(horizontal ? SnapToGuides.KEY_HORIZONTAL_GUIDE
			                : SnapToGuides.KEY_VERTICAL_GUIDE);
	if (guidePos == null)
		result = result.chain(new ChangeGuideCommand(part, horizontal));

	return result;
}

protected Command createAddCommand(Request request, EditPart childEditPart, 
		Object constraint) {
	FritzingSubpart part = (FritzingSubpart)childEditPart.getModel();
	Rectangle rect = (Rectangle)constraint;

	AddCommand add = new AddCommand();
	add.setParent((FritzingDiagram)getHost().getModel());
	add.setChild(part);
	add.setLabel(FritzingMessages.FritzingXYLayoutEditPolicy_AddCommandLabelText);
	add.setDebugLabel("FritzingXYEP add subpart");//$NON-NLS-1$

	SetConstraintCommand setConstraint = new SetConstraintCommand();
	setConstraint.setLocation(rect);
	setConstraint.setPart(part);
	setConstraint.setLabel(FritzingMessages.FritzingXYLayoutEditPolicy_AddCommandLabelText);
	setConstraint.setDebugLabel("FritzingXYEP setConstraint");//$NON-NLS-1$
	
	Command cmd = add.chain(setConstraint);
	cmd = chainGuideAttachmentCommand(request, part, cmd, true);
	cmd = chainGuideAttachmentCommand(request, part, cmd, false);
	cmd = chainGuideDetachmentCommand(request, part, cmd, true);
	return chainGuideDetachmentCommand(request, part, cmd, false);
}

/**
 * @see org.eclipse.gef.editpolicies.ConstrainedLayoutEditPolicy#createChangeConstraintCommand(org.eclipse.gef.EditPart, java.lang.Object)
 */
protected Command createChangeConstraintCommand(EditPart child, Object constraint) {
	return null;
}

protected Command createChangeConstraintCommand(ChangeBoundsRequest request, 
                                                EditPart child, Object constraint) {
	SetConstraintCommand cmd = new SetConstraintCommand();
	FritzingSubpart part = (FritzingSubpart)child.getModel();
	cmd.setPart(part);
	cmd.setLocation((Rectangle)constraint);
	Command result = cmd;

	if ((request.getResizeDirection() & PositionConstants.NORTH_SOUTH) != 0) {
		Integer guidePos = (Integer)request.getExtendedData()
				.get(SnapToGuides.KEY_HORIZONTAL_GUIDE);
		if (guidePos != null) {
			result = chainGuideAttachmentCommand(request, part, result, true);
		} else if (part.getHorizontalGuide() != null) {
			// SnapToGuides didn't provide a horizontal guide, but this part is attached
			// to a horizontal guide.  Now we check to see if the part is attached to
			// the guide along the edge being resized.  If that is the case, we need to
			// detach the part from the guide; otherwise, we leave it alone.
			int alignment = part.getHorizontalGuide().getAlignment(part);
			int edgeBeingResized = 0;
			if ((request.getResizeDirection() & PositionConstants.NORTH) != 0)
				edgeBeingResized = -1;
			else
				edgeBeingResized = 1;
			if (alignment == edgeBeingResized)
				result = result.chain(new ChangeGuideCommand(part, true));
		}
	}
	
	if ((request.getResizeDirection() & PositionConstants.EAST_WEST) != 0) {
		Integer guidePos = (Integer)request.getExtendedData()
				.get(SnapToGuides.KEY_VERTICAL_GUIDE);
		if (guidePos != null) {
			result = chainGuideAttachmentCommand(request, part, result, false);
		} else if (part.getVerticalGuide() != null) {
			int alignment = part.getVerticalGuide().getAlignment(part);
			int edgeBeingResized = 0;
			if ((request.getResizeDirection() & PositionConstants.WEST) != 0)
				edgeBeingResized = -1;
			else
				edgeBeingResized = 1;
			if (alignment == edgeBeingResized)
				result = result.chain(new ChangeGuideCommand(part, false));
		}
	}
	
	if (request.getType().equals(REQ_MOVE_CHILDREN)
			|| request.getType().equals(REQ_ALIGN_CHILDREN)) {
		result = chainGuideAttachmentCommand(request, part, result, true);
		result = chainGuideAttachmentCommand(request, part, result, false);
		result = chainGuideDetachmentCommand(request, part, result, true);
		result = chainGuideDetachmentCommand(request, part, result, false);
	}

	return result;
}

protected EditPolicy createChildEditPolicy(EditPart child) {
	if (child instanceof LEDEditPart 
			|| child instanceof OutputEditPart) {
		ResizableEditPolicy policy = new FritzingResizableEditPolicy();
		policy.setResizeDirections(0);
		return policy;
	} else if (child instanceof FritzingLabelEditPart) {
		ResizableEditPolicy policy = new FritzingResizableEditPolicy();
		policy.setResizeDirections(PositionConstants.EAST | PositionConstants.WEST);
		return policy;
	}
	
	return new FritzingResizableEditPolicy();
}


/* (non-Javadoc)
 * @see org.eclipse.gef.editpolicies.LayoutEditPolicy#createSizeOnDropFeedback(org.eclipse.gef.requests.CreateRequest)
 */
protected IFigure createSizeOnDropFeedback(CreateRequest createRequest) {
	IFigure figure;
	
	if (createRequest.getNewObject() instanceof Circuit)
		figure = new CircuitFeedbackFigure();
	else if (createRequest.getNewObject() instanceof FritzingFlowContainer)
		figure = new FritzingFlowFeedbackFigure();		
	else if (createRequest.getNewObject() instanceof FritzingLabel)
		figure = new LabelFeedbackFigure();
	else {
		figure = new RectangleFigure();
		((RectangleFigure)figure).setXOR(true);
		((RectangleFigure)figure).setFill(true);
		figure.setBackgroundColor(FritzingColorConstants.ghostFillColor);
		figure.setForegroundColor(ColorConstants.white);
	}
	
	addFeedback(figure);
	
	return figure;
}

protected FritzingGuide findGuideAt(int pos, boolean horizontal) {
	RulerProvider provider = ((RulerProvider)getHost().getViewer().getProperty(
			horizontal ? RulerProvider.PROPERTY_VERTICAL_RULER 
			: RulerProvider.PROPERTY_HORIZONTAL_RULER));
	return (FritzingGuide)provider.getGuideAt(pos);
}

protected Command getAddCommand(Request generic) {
	ChangeBoundsRequest request = (ChangeBoundsRequest)generic;
	List editParts = request.getEditParts();
	CompoundCommand command = new CompoundCommand();
	command.setDebugLabel("Add in ConstrainedLayoutEditPolicy");//$NON-NLS-1$
	GraphicalEditPart childPart;
	Rectangle r;
	Object constraint;

	for (int i = 0; i < editParts.size(); i++) {
		childPart = (GraphicalEditPart)editParts.get(i);
		r = childPart.getFigure().getBounds().getCopy();
		//convert r to absolute from childpart figure
		childPart.getFigure().translateToAbsolute(r);
		r = request.getTransformedRectangle(r);
		//convert this figure to relative 
		getLayoutContainer().translateToRelative(r);
		getLayoutContainer().translateFromParent(r);
		r.translate(getLayoutOrigin().getNegated());
		constraint = getConstraintFor(r);
		command.add(createAddCommand(generic, childPart,
			translateToModelConstraint(constraint)));
	}
	return command.unwrap();
}

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
	
	Iterator i = request.getEditParts().iterator();
	GraphicalEditPart currPart = null;
	
	while (i.hasNext()) {
		currPart = (GraphicalEditPart)i.next();
		clone.addPart((FritzingSubpart)currPart.getModel(), 
				(Rectangle)getConstraintForClone(currPart, request));
	}
	
	// Attach to horizontal guide, if one is given
	Integer guidePos = (Integer)request.getExtendedData()
			.get(SnapToGuides.KEY_HORIZONTAL_GUIDE);
	if (guidePos != null) {
		int hAlignment = ((Integer)request.getExtendedData()
				.get(SnapToGuides.KEY_HORIZONTAL_ANCHOR)).intValue();
		clone.setGuide(findGuideAt(guidePos.intValue(), true), hAlignment, true);
	}
	
	// Attach to vertical guide, if one is given
	guidePos = (Integer)request.getExtendedData()
			.get(SnapToGuides.KEY_VERTICAL_GUIDE);
	if (guidePos != null) {
		int vAlignment = ((Integer)request.getExtendedData()
				.get(SnapToGuides.KEY_VERTICAL_ANCHOR)).intValue();
		clone.setGuide(findGuideAt(guidePos.intValue(), false), vAlignment, false);
	}

	return clone;
}

protected Command getCreateCommand(CreateRequest request) {
	CreateCommand create = new CreateCommand();
	create.setParent((FritzingDiagram)getHost().getModel());
	FritzingSubpart newPart = (FritzingSubpart)request.getNewObject();
	create.setChild(newPart);
	Rectangle constraint = (Rectangle)getConstraintFor(request);
	create.setLocation(constraint);
	create.setLabel(FritzingMessages.FritzingXYLayoutEditPolicy_CreateCommandLabelText);
	
	Command cmd = chainGuideAttachmentCommand(request, newPart, create, true);
	return chainGuideAttachmentCommand(request, newPart, cmd, false);
}

/* (non-Javadoc)
 * @see org.eclipse.gef.editpolicies.LayoutEditPolicy#getCreationFeedbackOffset(org.eclipse.gef.requests.CreateRequest)
 */
protected Insets getCreationFeedbackOffset(CreateRequest request) {
	if (request.getNewObject() instanceof LED 
			|| request.getNewObject() instanceof Circuit)
		return new Insets(2, 0, 2, 0);
	return new Insets();
}

/**
 * Returns the layer used for displaying feedback.
 *  
 * @return the feedback layer
 */
protected IFigure getFeedbackLayer() {
	return getLayer(LayerConstants.SCALED_FEEDBACK_LAYER);
}
	
}
