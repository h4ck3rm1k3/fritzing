package org.fritzing.fritzing.diagram.edit.parts;

import java.net.URL;
import java.util.List;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.Path;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.ImageFigure;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.StackLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.draw2d.ui.render.RenderedImage;
import org.eclipse.gmf.runtime.draw2d.ui.render.factory.RenderedImageFactory;
import org.eclipse.gmf.runtime.draw2d.ui.render.figures.ScalableImageFigure;
import org.eclipse.swt.graphics.Image;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;

/**
 * @generated NOT
 */
public class PartFigure extends RectangleFigure {
	/**
	 * @generated NOT
	 */
	protected float multiplier = 1f;		/**
	 * @generated NOT
	 */
	protected ScalableImageFigure baseFigure;
	/**
	 * @generated NOT
	 */
	protected ImageFigure imageFigure;
	/**
	 * @generated NOT
	 */
	protected Image image;
	/**
	 * @generated NOT
	 */
	protected RenderedImage renderedImage;
	/**
	 * @generated NOT
	 */
	protected PartLoader partLoader;
	/**
	 * @generated NOT
	 */
	protected String  contentsPath;
	
	
	public PartFigure(PartLoader partLoader) {
		this.partLoader = partLoader;
		createContents();
		
	//	if (image != null) {
	//		org.eclipse.swt.graphics.Rectangle r = image.getBounds();
	//		int w = (int) (r.width / multiplier);
	//		int h = (int) (r.height / multiplier);
	//		this.setPreferredSize(new Dimension(getMapMode().DPtoLP(w),
	//				getMapMode().DPtoLP(h)));
	//	} else if (renderedImage != null) {
	//		RenderInfo rinfo = renderedImage.getRenderInfo();
	//		this.setPreferredSize(new Dimension(getMapMode().DPtoLP(
	//				rinfo.getWidth()), getMapMode().DPtoLP(
	//				rinfo.getHeight())));
	//	}
	
		this.setPreferredSize(new Dimension(partLoader.getSize()));
	}

	/**
	 * @generated NOT
	 */
	protected void createContents() {
		setLayoutManager(new StackLayout() {
			public void layout(IFigure figure) {
				Rectangle r = figure.getClientArea();
				List children = figure.getChildren();
				IFigure child;
				Dimension d;
				for (int i = 0; i < children.size(); i++) {
					child = (IFigure) children.get(i);
					d = child.getPreferredSize(r.width, r.height);
					d.width = Math.min(d.width, r.width);
					d.height = Math.min(d.height, r.height);
					Rectangle childRect = new Rectangle(r.x
							+ (r.width - d.width) / 2, r.y
							+ (r.height - d.height) / 2, d.width, d.height);
					child.setBounds(childRect);
				}
			}
		});

		try {
			String filename = partLoader.getBitmapFilename();
			if (filename != null && filename != "") {
//				image = FritzingDiagramEditorPlugin.getInstance()
//						.getBundledImage("icons/parts/" + filename);
//				URL url = new URL("File://" + FritzingDiagramEditorUtil.getFritzingLocation() + path + partLoader.getBitmapFilename());
//				RenderedImage ri = RenderedImageFactory.getInstance(url);
//				image = ri.getSWTImage();
									
				image = new Image(null, partLoader.getContentsPath() + partLoader.getBitmapFilename());
			
				// stick image on the registry
			} 
			else {
				filename = partLoader.getSvgFilename();
				if (filename != null && filename != "") {
					URL url = FileLocator
							.find(
									FritzingDiagramEditorPlugin.getInstance()
											.getBundle(),
									new Path(
											"icons/parts/" + partLoader.getSvgFilename()), null); //$NON-NLS-1$
					url = new URL("file", "", -1, FritzingDiagramEditorUtil.getFritzingLocation() + contentsPath + partLoader.getSvgFilename());
											
					RenderedImage ri = RenderedImageFactory.getInstance(url);
					image = ri.getSWTImage();

			            
					// stick image on the registry
				}
			}

			// URL url = FileLocator.find(FritzingDiagramEditorPlugin
			// .getInstance().getBundle(), new Path(
			// "icons/parts/arduino_all.png"), null); //$NON-NLS-1$
			// RenderedImage ri = RenderedImageFactory.getInstance(url);
			// baseFigure = new ScalableImageFigure(ri, true, true, false);
			// this.add(baseFigure);
			// Rectangle s = new Rectangle(0,0,295,206);
			// baseFigure.setBounds(s);

			// URL url = FileLocator.find(FritzingDiagramEditorPlugin
			// .getInstance().getBundle(), new Path(
			// "icons/parts/arduino_all.svg"), null); //$NON-NLS-1$
			// RenderedImage ri = RenderedImageFactory.getInstance(url);
			// RenderInfo rinfo = ri.getRenderInfo();
			// image = ri.getSWTImage();
			// org.eclipse.swt.graphics.Rectangle r = image.getBounds();
			// rinfo.setValues((int) (r.width * multiplier), (int) (r.height *
			// multiplier),
			// rinfo.shouldMaintainAspectRatio(), true,
			// rinfo.getBackgroundColor(), rinfo.getForegroundColor());
			// ri = ri.getNewRenderedImage(rinfo);
			// try {
			// image = ri.getSWTImage();
			// }
			// catch (Exception ex) {
			// ex.printStackTrace();
			// }
			// baseFigure = new ScalableImageFigure(ri, true, true, false);
			// this.add(baseFigure);
		}
		catch (Exception ex) {
			// inform the user?
			ex.printStackTrace();
		}
		
	}
	
	/**
	 * @generated NOT
	 */
	public void paintFigure(Graphics g) {
		// super.paintFigure(g);
		if (image != null) {
			Rectangle r = getBounds().getCopy();
			// g.drawImage(image, new org.eclipse.draw2d.geometry.Point(r.x,
			// r.y));
			org.eclipse.draw2d.geometry.Rectangle s = new org.eclipse.draw2d.geometry.Rectangle(
					image.getBounds());
			// org.eclipse.swt.graphics.Rectangle t = image.getBounds();
			// s.x = t.x;
			// s.y = t.y;
			// s.height = t.height;
			// s.width = t.width;
			// s.width = getMapMode().DPtoLP(s.width);
			// s.height = getMapMode().DPtoLP(s.height);
			// g.drawImage(image, r.x, r.y);

			g.drawImage(image, 0, 0, s.width, s.height, r.x, r.y, r.width,
					r.height);
		}

		// if (renderedImage != null) {
		// Rectangle r = getBounds().getCopy();
		// RenderInfo info = renderedImage.getRenderInfo();
		// info.setValues(getMapMode().LPtoDP(r.width),
		// getMapMode().LPtoDP(r.height), info.shouldMaintainAspectRatio(),
		// false, info.getBackgroundColor(), info.getForegroundColor());
		// g.drawImage(renderedImage.getNewRenderedImage(info).getSWTImage(),
		// r.x, r.y);
		//
		// }

	}

}
