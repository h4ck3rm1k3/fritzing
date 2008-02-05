package org.fritzing.pcb.utils;

import org.eclipse.draw2d.geometry.PrecisionRectangle;

public class RotateDing {
	
	public double p1x;
	public double p1y;
	public double p2x;
	public double p2y;
	public double p3x;
	public double p3y;
	public double p4x;
	public double p4y;
	
	public double dcx;
	public double dcy;
	
	public static RotateDing rotatePrecisionRect(int degrees, PrecisionRectangle r) {
		RotateDing rotateDing = new RotateDing();
		double rad = Math.toRadians(degrees);
		
		rotateDing.p1x = r.x * Math.cos(rad) + r.y * Math.sin(rad);
		rotateDing.p1y = -r.x * Math.sin(rad) + r.y * Math.cos(rad);
		rotateDing.p2x = r.x * Math.cos(rad) + r.bottom() * Math.sin(rad);
		rotateDing.p2y = -r.x * Math.sin(rad) + r.bottom() * Math.cos(rad);
		rotateDing.p3x = r.right() * Math.cos(rad) + r.bottom() * Math.sin(rad);
		rotateDing.p3y = -r.right() * Math.sin(rad) + r.bottom() * Math.cos(rad);
		rotateDing.p4x = r.right() * Math.cos(rad) + r.y * Math.sin(rad);
		rotateDing.p4y = -r.right() * Math.sin(rad) + r.y * Math.cos(rad);

		double dx = (rotateDing.p1x + rotateDing.p3x) / 2;
		double dy = (rotateDing.p1y + rotateDing.p3y) / 2;
		double cx = r.x + (r.width / 2.0);
		double cy = r.y + (r.height / 2.0);
		rotateDing.dcx = dx - cx;
		rotateDing.dcy = dy - cy;

		return rotateDing;
	}
	
	public void adjustCenter() {
		p1x -= dcx;
		p1y -= dcy;
		p2x -= dcx;
		p2y -= dcy;
		p3x -= dcx;
		p3y -= dcy;
		p4x -= dcx;
		p4y -= dcy;
	}

}
