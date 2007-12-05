package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.FreeformLayer;
import org.eclipse.draw2d.IFigure;
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

	/* (non-Javadoc)
	 * @see org.eclipse.gef.editparts.FreeformGraphicalRootEditPart#createFigure()
	 */
	@Override
	protected IFigure createFigure() {
		IFigure fig = super.createFigure();
		fig.setBackgroundColor(THIS_BACK);
		fig.setOpaque(true);
		return fig;
	}

	/**
	 * @generated NOT
	 */
	static final Color THIS_BACK = new Color(null, 204, 204, 204);

}
