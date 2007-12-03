package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.FreeformLayer;
import org.eclipse.draw2d.ScalableFreeformLayeredPane;
import org.eclipse.gef.editparts.GridLayer;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramRootEditPart;
import org.eclipse.gmf.runtime.diagram.ui.internal.editparts.GridLayerEx;
import org.eclipse.gmf.runtime.notation.MeasurementUnit;
import org.eclipse.swt.graphics.Color;

public class FritzingDiagramRootEditPart extends DiagramRootEditPart {

	public FritzingDiagramRootEditPart() {
		super();
	}

	public FritzingDiagramRootEditPart(MeasurementUnit mu) {
		super(mu);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramRootEditPart#createGridLayer()
	 */
	@Override
	protected GridLayer createGridLayer() {
		return new SketchGridLayer();
	}

	/* (non-Javadoc)
	 * @see org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramRootEditPart#createGridLayer(int, int, int)
	 */
	@Override
	protected GridLayer createGridLayer(int r, int g, int b) {
		return new SketchGridLayer(new Color(null,r,g,b));
	}

}
