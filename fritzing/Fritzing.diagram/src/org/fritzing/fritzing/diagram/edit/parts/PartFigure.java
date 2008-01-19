package org.fritzing.fritzing.diagram.edit.parts;

import java.util.Collections;
import java.util.Hashtable;
import java.util.Vector;

import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.eclipse.swt.graphics.Image;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated NOT
 */
public class PartFigure extends RectangleFigure implements IZoomableFigure {

	protected Image image;
	private double zoom = -1;
	protected PartLoader partLoader;
	protected String  contentsPath;

	/**
	 * @generated NOT
	 */
	public PartFigure(PartLoader partLoader) {
		this.partLoader = partLoader;
		this.setSize(partLoader.getSize());
		this.setPreferredSize(new Dimension(partLoader.getSize()));
		this.setMaximumSize(new Dimension(partLoader.getSize()));
		this.setMinimumSize(new Dimension(partLoader.getSize()));
		createContents();
	}

	/**
	 * @generated NOT
	 */
	protected void createContents() {
		setOutline(false);

		// TODO: SVGs
		/*
		 url = new URL("file", null, partLoader.getContentsPath() + partLoader.getSvgFilename());
		 RenderedImage ri = RenderedImageFactory.getInstance(url);
		 RenderInfo rinfo = ri.getRenderInfo();
		 image = ri.getSWTImage();
		 org.eclipse.swt.graphics.Rectangle r = image.getBounds();
		 rinfo.setValues((int) (r.width * multiplier), (int) (r.height * multiplier),
		 rinfo.shouldMaintainAspectRatio(), true,
		 rinfo.getBackgroundColor(), rinfo.getForegroundColor());
		 ri = ri.getNewRenderedImage(rinfo);
		 try {
			 image = ri.getSWTImage();
		 } catch (Exception ex) {
			 ex.printStackTrace();
		 }
		 baseFigure = new ScalableImageFigure(ri, true, true, false);
		 this.add(baseFigure);
		 */
	}
	
	public void setZoom(double newZoom) {
		zoom = newZoom;
		updateImage();
	}
	
	/*
	 * Loads the best image for the current zoom level
	 */
	private void updateImage() {
		double bestAvailableLevel = zoom;
		if (!partLoader.getBitmapFilenames().containsKey(bestAvailableLevel)) {
			Vector<Double> levels = getSortedImageLevels();
			bestAvailableLevel = levels.lastElement();
		    for (Double level: levels) {
		    	if (level.compareTo(zoom) >= 0) {
		    		bestAvailableLevel = level;
		    		break;
		    	}
		    }
		}
		try {
			String imageSrc = partLoader.getBitmapFilenames().get(
				new Double(bestAvailableLevel));
			image = FritzingElementTypes.getImageRegistry().get(
				partLoader.getContentsPath() + imageSrc);
			
			// buffer the scaled image
			Dimension figureSize = getSize();
			if (figureSize.width > 0) {
			    figureSize.width = MapModeUtil.getMapMode().LPtoDP((int)Math.round(figureSize.width * zoom));
			    figureSize.height = MapModeUtil.getMapMode().LPtoDP((int)Math.round(figureSize.height * zoom));
			    
			    /* 'normal' GC scaling doesn't preserve transparency, 
			     * so we use ImageData.scaledTo():
			     */
//				    Image scaledImage = new Image(null, figureSize.width, figureSize.height);
//				    GC gc = new GC(scaledImage);
//					org.eclipse.swt.graphics.Rectangle imageSize = image.getBounds();
//				    gc.drawImage(image, 0, 0, imageSize.width, imageSize.height, 
//				    		0, 0, figureSize.width, figureSize.height);
//				    gc.dispose();
			    Image scaledImage = new Image(null,
			    		image.getImageData().scaledTo(figureSize.width, figureSize.height));
			    image = scaledImage;
			}
		} catch (Exception ex) {
			// inform the user?
			ex.printStackTrace();
		}
	}
	
	protected Vector<Double> getSortedImageLevels() {
		Hashtable<Double,String> images = partLoader.getBitmapFilenames();
		// find the next biggest available image zoom level
		Vector<Double> levels = new Vector<Double>(images.keySet());
	    Collections.sort(levels);
	    return levels;
	}
	
	/**
	 * @generated NOT
	 */
	public void paintFigure(Graphics g) {
//		 super.paintFigure(g);
		if (image != null) {
			Rectangle r = getBounds();
			g.pushState();
			g.translate(r.x, r.y);
			g.scale(1/zoom); // to compensate for image scaling
			g.drawImage(image, 0, 0);
			g.popState();
		}
	}

}
