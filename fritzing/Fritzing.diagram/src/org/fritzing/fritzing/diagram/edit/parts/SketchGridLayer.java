/**
 * (c) FH Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.internal.editparts.GridLayerEx;
import org.eclipse.swt.graphics.Color;

public class SketchGridLayer extends GridLayerEx {

	public SketchGridLayer() {
		super();
	}

	public SketchGridLayer(Color color, Point p) {
		super(color, p);
	}

	public SketchGridLayer(Color color) {
		super(color);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editparts.GridLayer#paintGrid(org.eclipse.draw2d.Graphics)
	 */
	@Override
	protected void paintGrid(Graphics g) {
		super.paintGrid(g);

//		Rectangle clip = g.getClip(Rectangle.SINGLETON);
//
//		if (gridX > 0) {
//			if (origin.x >= clip.x)
//				while (origin.x - gridX >= clip.x)
//					origin.x -= gridX;
//			else
//				while (origin.x < clip.x)
//					origin.x += gridX;
//
//			if (gridX > 0) {
//				if (origin.y >= clip.y)
//					while (origin.y - gridX >= clip.y)
//						origin.y -= gridX;
//				else
//					while (origin.y < clip.y)
//						origin.y += gridX;
//			}
//
//			for (int j = origin.y; j < clip.y + clip.height; j += gridX)
//				for (int i = origin.x; i < clip.x + clip.width; i += gridX)
//					g.drawPoint(i, j);
//		}
	}

}
