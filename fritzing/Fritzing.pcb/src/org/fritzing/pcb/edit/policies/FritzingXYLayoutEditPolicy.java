/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.policies;

import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.emf.transaction.TransactionalEditingDomain;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.ChangeBoundsRequest;
import org.eclipse.gmf.runtime.common.core.command.ICommand;
import org.eclipse.gmf.runtime.diagram.ui.commands.ICommandProxy;
import org.eclipse.gmf.runtime.diagram.ui.commands.SetBoundsCommand;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.diagram.ui.l10n.DiagramUIMessages;
import org.eclipse.gmf.runtime.emf.core.util.EObjectAdapter;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.pcb.edit.commands.SetRotationCommand;
import org.fritzing.pcb.edit.requests.FritzingRotateShapeRequestEx;
import org.fritzing.pcb.edit.requests.RotateShapeRequestEx;

import org.eclipse.gmf.runtime.diagram.ui.editpolicies.XYLayoutEditPolicy;


public class FritzingXYLayoutEditPolicy extends XYLayoutEditPolicy {

	protected FritzingRotateShapeRequestEx request;
	
	protected Command createChangeConstraintCommand(ChangeBoundsRequest request, 
			EditPart child, Object constraint) {
		
		// intercept this so we can hold on to the request for the later call to 
		// createChangeConstraintCommand(child, constraint);
		
		if (request instanceof FritzingRotateShapeRequestEx) {
			this.request = (FritzingRotateShapeRequestEx) request;	
			Rectangle newBounds = ((FritzingRotateShapeRequestEx) request).getNewBounds();
			if (newBounds != null) {
				constraint = (Object) newBounds;
			}

		}
		else {
			this.request = null;
		}
		return super.createChangeConstraintCommand(request, child, constraint);
	}
	
	protected Command createChangeConstraintCommand(						
		EditPart child,
		Object constraint) {
		
		Command cmd = super.createChangeConstraintCommand(child, constraint);		
		if (this.request == null) {
			// not a rotate request so don't add a rotate command
			return cmd;			
		}
		
		Rectangle bounds = (Rectangle) constraint;
		View shapeView = (View) child.getModel();
						
	    TransactionalEditingDomain editingDomain = ((IGraphicalEditPart) getHost())
	        .getEditingDomain();
	    	    	    
		ICommand boundsCommand = 
			new SetRotationCommand(editingDomain,
				DiagramUIMessages.SetLocationCommand_Label_Resize,
				new EObjectAdapter(shapeView),
				bounds,
				request.getResizeDirection(),
				request.getRotationDirection()); 
		
		this.request = null;
		
		// let it do the normal change bounds stuff, but add a rotate command
		cmd = cmd.chain(new ICommandProxy(boundsCommand));
		return cmd;
	}
}
