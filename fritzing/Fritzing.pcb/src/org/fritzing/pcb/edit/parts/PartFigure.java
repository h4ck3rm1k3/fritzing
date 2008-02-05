package org.fritzing.pcb.edit.parts;

import java.util.Collections;
import java.util.Hashtable;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;

import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.PrecisionRectangle;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.figures.IBorderItemLocator;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.ImageLoader;
import org.eclipse.swt.graphics.ImageData;
import org.eclipse.swt.graphics.PaletteData;
import org.eclipse.swt.graphics.RGB;
import org.fritzing.pcb.edit.PartDefinition;
import org.fritzing.pcb.edit.PartDefinitionRegistry;
import org.fritzing.pcb.edit.PartLoader;
import org.fritzing.pcb.edit.policies.RotatableNonresizableShapeEditPolicy;
import org.fritzing.pcb.providers.FritzingElementTypes;
import org.fritzing.pcb.utils.RotateDing;
import org.eclipse.swt.graphics.Transform;

public class PartFigure extends RectangleFigure implements IZoomableFigure {

	protected Image image;
	protected Image rotatedImage;
	private double zoom = -1;
	protected PartDefinition partDefinition;
	protected String  contentsPath;
	protected int rotation;

	
	/*
	 * @generated NOT
	 */
	public PartFigure(PartDefinition partDefinition) {
		this.partDefinition = partDefinition;
		this.rotation = 0;
		this.rotatedImage = null;
		this.setSize(partDefinition.getSize());
		Dimension d = partDefinition.getSize();
		this.setPreferredSize(d);
		
		// set the min and max to accomodate rotation
		int maxD = Math.max(d.width, d.height);
		this.setMaximumSize(new Dimension(maxD, maxD));
		int minD = Math.min(d.width, d.height);
		this.setMinimumSize(new Dimension(minD, minD));
		createContents();
	}
	
	public void setRotation(int degrees) {
		if (degrees != rotation) {
			rotation = degrees;
			setRotationAux();
		}
	}
	
	protected void setRotationAux() {	
		org.eclipse.swt.graphics.Rectangle bounds = image.getBounds();
		org.eclipse.draw2d.geometry.Rectangle r = new org.eclipse.draw2d.geometry.Rectangle(bounds);
		PrecisionRectangle pr = new PrecisionRectangle(r);
		RotateDing rotateDing = RotateDing.rotatePrecisionRect(rotation, pr);

		double minx = Math.min(rotateDing.p1x, Math.min(rotateDing.p2x, Math.min(rotateDing.p3x, rotateDing.p4x)));
		double maxx = Math.max(rotateDing.p1x, Math.max(rotateDing.p2x, Math.max(rotateDing.p3x, rotateDing.p4x)));
		double miny = Math.min(rotateDing.p1y, Math.min(rotateDing.p2y, Math.min(rotateDing.p3y, rotateDing.p4y)));
		double maxy = Math.max(rotateDing.p1y, Math.max(rotateDing.p2y, Math.max(rotateDing.p3y, rotateDing.p4y)));	
		double dw = maxx - minx;
		double dh = maxy - miny;	
		
		try {
			ImageData targetImageData = new ImageData((int) dw, (int) dh, 24, image.getImageData().palette);
															
		    Transform transform = new Transform(null);
		    transform.translate((float) -minx, (float) -miny);
		    transform.rotate(360 - rotation);
							
		    Image targetImage = new Image(null, targetImageData);	
		    GC gc = new GC(targetImage);
		    
		    // in theory, the only thing we should have needed to do here 
		    // is set the transform and draw the source into the target
		    
		    gc.setTransform(transform);
		    gc.drawImage(image, 0, 0);
		    gc.dispose();		    
		    
		    // unfortunately, SWT doesn't seem to copy the alpha channel
		    // so the rest of this is a hack in order to copy the alpha channel
		    			    			    
		    // make a target alpha channel image
			ImageData targetAlphaImageData = new ImageData((int) dw, (int) dh, 8, AlphaPaletteData.getPaletteData());
			Image targetAlphaImage = new Image(null, targetAlphaImageData);
										
			// get the alpha from the original source image and create a source alpha image
			// note that the alpha from the image is packed (i.e. the lines aren't padded with bytes)
			// but when you use this data as an image, you have to pad the data
			ImageData sourceAlphaImageData = new ImageData(r.width, r.height, 8, AlphaPaletteData.getPaletteData());
			byte[] sourceData = sourceAlphaImageData.data;
			byte[] data = image.getImageData().alphaData;
			int lx = 0;
			int ix = 0;
			for (int y = 0; y < r.height; y++) {
				for (int x = 0; x < r.width; x++) {
					sourceData[lx + x] = data[ix++];
				}
				lx += sourceAlphaImageData.bytesPerLine;
			}
			sourceAlphaImageData.data = sourceData;
			Image sourceAlphaImage = new Image(null, sourceAlphaImageData);
			GC agc = new GC(targetAlphaImage);
									
			// now blt the source alpha to the target alpha
			agc.setTransform(transform);
		    agc.drawImage(sourceAlphaImage, 0, 0);			
			agc.dispose();
				
			// now to use the resulting alpha image as alpha data
			// remove the line padding (i.e. pack it)
			
			// trueDepth is needed because the mac makes a 32-bit image instead of an 8-bit image
			int trueDepth = targetAlphaImage.getImageData().depth;
			
			data = targetAlphaImage.getImageData().data;
			byte[] neoData = new byte[(int) dw * (int) dh];
			lx = 0;
			ix = 0;
			
			if (trueDepth == 8) {
				for (int y = 0; y < (int) dh; y++) {
					for (int x = 0; x < (int) dw; x++) {
						neoData[ix++] = data[lx + x];
					}
					lx += targetAlphaImage.getImageData().bytesPerLine;
				}
			}
			else if (trueDepth == 32) {
				for (int y = 0; y < (int) dh; y++) {
					for (int x = 0; x < (int) dw; x++) {
						neoData[ix++] = data[lx + (x << 2) + 1];
					}
					lx += targetAlphaImage.getImageData().bytesPerLine;
				}				
			}
				
			// you can't modify the imageData of a pre-existing image
			// so make a new image using the copied rgb and the copied alpha
			// and use the new image to draw on screen
			ImageData newTargetImageData = targetImage.getImageData();
			newTargetImageData.alphaData = neoData;
			Image neoTargetImage = new Image(null, newTargetImageData);			
			
			targetAlphaImage.dispose();
			sourceAlphaImage.dispose();
			targetImage.dispose();
						
		    transform.dispose();
		    		    
		    if (rotatedImage != null) {
		    	rotatedImage.dispose();
		    }
		    
		    rotatedImage = neoTargetImage;	    
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
			
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
	
	public double getZoom() {
		return zoom;
	}
	
	/*
	 * Loads the best image for the current zoom level
	 */
	private void updateImage() {
		double bestAvailableLevel = zoom;
		if (!partDefinition.getBitmapFilenames().containsKey(bestAvailableLevel)) {
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
			String imageSrc = partDefinition.getBitmapFilenames().get(
				new Double(bestAvailableLevel));
			image = FritzingElementTypes.getImageRegistry().get(
					partDefinition.getContentsPath() + imageSrc);
			
			// buffer the scaled image
			Dimension figureSize = partDefinition.getSize().getCopy();			
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
		    
			    if (rotation != 0) {
//				    ImageLoader imageLoader = new ImageLoader();
//				    imageLoader.data = new ImageData[] { image.getImageData() };
//				    imageLoader.save("image.jpg",SWT.IMAGE_JPEG);
				    setRotationAux();
			    }		    	
			}
		} catch (Exception ex) {
			// inform the user?
			ex.printStackTrace();
		}
	}
	
	protected Vector<Double> getSortedImageLevels() {
		Hashtable<Double,String> images = partDefinition.getBitmapFilenames();
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
			g.drawImage((rotatedImage != null) ? rotatedImage : image, 0, 0);
			g.popState();
		}
	}
	
	public static class AlphaPaletteData {
		
		static PaletteData paletteData;
		
		/**
		 * Singleton instance.
		 */
		private static AlphaPaletteData singletonInstance = new AlphaPaletteData();
		
		private AlphaPaletteData() {
			RGB[] rgb = new RGB[256];
			for (int i = 0; i < 256; i++) {
				rgb[i] = new RGB(i, i, i);
			}	
		    paletteData = new PaletteData(rgb);
		}

		/**
		 * Return singleton instance.
		 * 
		 */
		public static PaletteData getPaletteData() {
			return singletonInstance.paletteData;
		}
				
	}
	
	
	


}
