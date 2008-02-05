package org.fritzing.pcb.edit.parts;

import org.eclipse.draw2d.Connection;
import org.eclipse.gef.RootEditPart;
import org.eclipse.gef.editparts.ZoomListener;
import org.eclipse.gmf.runtime.diagram.ui.editparts.ConnectionNodeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.pcb.edit.policies.WireItemSemanticEditPolicy;

public class ConnectionFritzingEditPart extends ConnectionNodeEditPart {
	
	public ConnectionFritzingEditPart(View view) {
		super(view);
	}
	
	protected Connection createConnectionFigure() {
		return new ConnectionFritzingFigure();
	}

	protected void createDefaultEditPolicies() {
		super.createDefaultEditPolicies();
		installEditPolicy(EditPolicyRoles.SEMANTIC_ROLE,
				new WireItemSemanticEditPolicy());

		addZoomListener(); // needed for zoom-dependent figure
	}

	protected void addZoomListener() {
		RootEditPart root = getRoot();
		if (root instanceof FritzingDiagramRootEditPart) {
			((FritzingDiagramRootEditPart)root).getZoomManager().addZoomListener(
				new ZoomListener() {
					public void zoomChanged(double zoom) {
						((IZoomableFigure)getPrimaryShape()).setZoom(zoom);
					}
				});
		}
		getPrimaryShape().setZoom(
				((FritzingDiagramRootEditPart)getRoot()).getZoomManager().getZoom());
	}

	public ConnectionFritzingFigure getPrimaryShape() {
		return (ConnectionFritzingFigure) getFigure();
	}
}
