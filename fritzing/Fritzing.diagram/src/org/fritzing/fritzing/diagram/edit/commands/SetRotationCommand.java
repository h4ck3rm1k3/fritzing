package org.fritzing.fritzing.diagram.edit.commands;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.emf.transaction.TransactionalEditingDomain;
import org.eclipse.gmf.runtime.common.core.command.CommandResult;
import org.eclipse.gmf.runtime.diagram.core.util.ViewUtil;
import org.eclipse.gmf.runtime.diagram.ui.commands.SetBoundsCommand;
import org.eclipse.gmf.runtime.notation.NotationPackage;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.diagram.edit.parts.PartEditPart;
import org.eclipse.emf.ecore.EObject;
import org.fritzing.fritzing.impl.PartImpl;
import org.eclipse.gmf.runtime.notation.PropertiesSetStyle;
import org.fritzing.fritzing.diagram.view.factories.PartShapeViewFactory;
import org.eclipse.draw2d.PositionConstants;

public class SetRotationCommand extends SetBoundsCommand {
	
	int handleDirection;
	int rotationDirection;
	protected IAdaptable adapter2;
	protected Point location2;
	protected Dimension size2;
	
	public SetRotationCommand(TransactionalEditingDomain editingDomain, String label,IAdaptable adapter, Rectangle bounds, 
			int handleDirection, int rotationDirection) {
		super(editingDomain, label, adapter, bounds);
		this.adapter2 = adapter;
		this.handleDirection = handleDirection;
		this.rotationDirection = rotationDirection;
		this.location2 = bounds.getLocation();
		this.size2 = bounds.getSize();
	}
	
	protected CommandResult doExecuteWithResult(IProgressMonitor monitor, IAdaptable info)
    throws ExecutionException {
				
		View view = (View)adapter2.getAdapter(View.class);	
		EObject element = view.getElement();		
		
		int fromAngle = getAngle(handleDirection);
		int toAngle = getAngle(rotationDirection);
		int diff = toAngle - fromAngle;
		if (diff < 0) diff += 360;
					
		PropertiesSetStyle style = (PropertiesSetStyle) view.getNamedStyle(
						NotationPackage.eINSTANCE.getPropertiesSetStyle(),
						PartShapeViewFactory.PARTS_PROPERTIES_STYLE_NAME);
		if (style != null) {
			if (style
					.hasProperty(PartShapeViewFactory.PARTS_ROTATION_PROPERTY_NAME)) {
				Integer value = (Integer) style.getProperty(PartShapeViewFactory.PARTS_ROTATION_PROPERTY_NAME);
				if (value != null) {
					diff += value.intValue();
					diff = diff % 360;
				}
				style.setProperty(
						PartShapeViewFactory.PARTS_ROTATION_PROPERTY_NAME, diff);
				

				return CommandResult.newOKCommandResult();
			}
		}

		return CommandResult.newCancelledCommandResult();
	
	}
	
	static public int getAngle(int positionConstant) {
		switch (positionConstant) {
		case PositionConstants.NORTH_EAST:
			return 45;
		case PositionConstants.NORTH_WEST:
			return 135;
		case PositionConstants.SOUTH_WEST:
			return 225;
		case PositionConstants.SOUTH_EAST:
			return 315;
		case PositionConstants.EAST:
			return 0;
		case PositionConstants.NORTH:
			return 90;
		case PositionConstants.WEST:
			return 180;			
		case PositionConstants.SOUTH:
			return 270;
		}
		
		return 0;
	}
}
