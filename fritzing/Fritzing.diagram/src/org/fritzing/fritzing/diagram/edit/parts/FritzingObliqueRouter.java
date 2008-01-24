package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.AutomaticRouter;
import org.eclipse.draw2d.Connection;
import org.eclipse.draw2d.geometry.PointList;

import org.eclipse.gmf.runtime.draw2d.ui.internal.routers.ObliqueRouter;

public class FritzingObliqueRouter extends ObliqueRouter {
		
	
	public FritzingObliqueRouter(AutomaticRouter router) {
		super();
		router.setNextRouter(this);
	}
	
	
	public FritzingObliqueRouter() {
		super();
	}

	protected boolean checkShapesIntersect(Connection conn, PointList newLine) {
		// a  to prevent leg contortions when the connection source and target are too close
		return false;
	}
	
}
