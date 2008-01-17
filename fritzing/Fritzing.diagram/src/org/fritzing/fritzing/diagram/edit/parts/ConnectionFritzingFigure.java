package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.gmf.runtime.draw2d.ui.figures.PolylineConnectionEx;

public class ConnectionFritzingFigure extends PolylineConnectionEx implements IZoomableFigure {

	protected int connectionWidth = 3; // zoom-dependent connection thickness
	
	public void zoomFigure(double zoom) {
		setLineWidth(new Long(Math.round(connectionWidth * zoom)).intValue());
	}

}
