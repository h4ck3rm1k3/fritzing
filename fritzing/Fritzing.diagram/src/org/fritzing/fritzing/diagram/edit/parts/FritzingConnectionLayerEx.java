package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.AutomaticRouter;
import org.eclipse.draw2d.ConnectionRouter;
import org.eclipse.gmf.runtime.draw2d.ui.internal.figures.ConnectionLayerEx;
import org.eclipse.gmf.runtime.draw2d.ui.internal.routers.FanRouter;
import org.eclipse.gmf.runtime.draw2d.ui.internal.routers.ObliqueRouter;

public class FritzingConnectionLayerEx extends ConnectionLayerEx {

	protected AutomaticRouter newObliqueRouter;
	
	public FritzingConnectionLayerEx() {
		super();
	}
	
	/**
	 * this allows us to swap in our own ObliqueRouter subclass
	 * which is the only way we've found (so far)
	 * that gets rid of wire contortions when endpoints are too close to each other
	 *
	 */
	public ConnectionRouter getObliqueRouter() {
		if (newObliqueRouter == null) {
			AutomaticRouter router = new FanRouter();
			router.setNextRouter(new FritzingObliqueRouter());
			newObliqueRouter = router;
		}

		return newObliqueRouter;
	}

}
