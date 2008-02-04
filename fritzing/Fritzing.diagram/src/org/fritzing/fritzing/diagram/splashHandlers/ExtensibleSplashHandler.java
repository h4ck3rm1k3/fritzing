
package org.fritzing.fritzing.diagram.splashHandlers;

import org.eclipse.core.runtime.IProduct;
import org.eclipse.core.runtime.Platform;
import org.eclipse.jface.resource.StringConverter;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.internal.splash.EclipseSplashHandler;
import org.osgi.framework.Bundle;

/**
 * @since 3.3
 *
 */
public class ExtensibleSplashHandler extends EclipseSplashHandler {
	
	Composite fIconPanel;
	Label label;
	
	protected Composite getContent() {
		String versionString = null;
		String versionRectString = null;
		String versionForegroundColorString = null;
		String versionBackgroundColorString = null;
		String versionFontSizeString = null;
		
		IProduct product = Platform.getProduct();
		
		if (product != null) {
			Bundle bundle = product.getDefiningBundle();
			Object bv = bundle.getHeaders().get("Bundle-Version");
			if (bv instanceof String) {
				versionString = (String) bv;
			}
			versionRectString = product
					.getProperty("startupVersionRect");
			versionForegroundColorString = product
				.getProperty("startupVersionForegroundColor");
			versionBackgroundColorString = product
				.getProperty("startupVersionBackgroundColor");
			versionFontSizeString = product
				.getProperty("startupVersionFontSize");
		}

		int versionForegroundColorInteger;
		try {
			versionForegroundColorInteger = Integer
					.parseInt(versionForegroundColorString, 16);
		} catch (Exception ex) {
			versionForegroundColorInteger = 0xffffff; 
		}

		int versionBackgroundColorInteger;
		try {
			versionBackgroundColorInteger = Integer
					.parseInt(versionBackgroundColorString, 16);
		} catch (Exception ex) {
			versionBackgroundColorInteger = 0x000000; 
		}

		int versionFontSizeInteger;
		try {
			versionFontSizeInteger = Integer
					.parseInt(versionFontSizeString);
		} catch (Exception ex) {
			versionFontSizeInteger = 14; 
		}

		Shell splash = getSplash();
		fIconPanel = new Composite(splash, SWT.NONE);
		fIconPanel.setBackgroundMode(SWT.INHERIT_DEFAULT);
        Rectangle versionRect = StringConverter.asRectangle(
				versionRectString, new Rectangle(10, 10, 300, 15));
		fIconPanel.setBounds(versionRect);
		GridLayout layout = new GridLayout(1, true);
		layout.horizontalSpacing = 0;
		layout.verticalSpacing = 0;
		layout.marginHeight = 0;
		layout.marginWidth = 0;
		fIconPanel.setLayout(layout);		
        label = new Label(fIconPanel, SWT.NONE);
        label.setFont(new Font(Display.getCurrent(), Display
				.getDefault().getSystemFont().getFontData()[0].getName(), versionFontSizeInteger,
				SWT.NORMAL));
        	              
        label.setForeground(new Color(getSplash().getShell().getDisplay(), 
				(versionForegroundColorInteger & 0xFF0000) >> 16,
				(versionForegroundColorInteger & 0xFF00) >> 8,
				versionForegroundColorInteger & 0xFF));
        label.setBackground(new Color(getSplash().getShell().getDisplay(), 
				(versionBackgroundColorInteger & 0xFF0000) >> 16,
				(versionBackgroundColorInteger & 0xFF00) >> 8,
				versionBackgroundColorInteger & 0xFF));
        label.setBounds(new Rectangle(0,0,versionRect.width,versionRect.height));
        label.setAlignment(SWT.RIGHT);
        label.setText(versionString);
	        		
	    return super.getContent();
	}

	
	
}
