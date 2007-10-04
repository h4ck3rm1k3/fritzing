/*******************************************************************************
 * Copyright (c) 2000, 2005 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package org.fritzing.model;

import java.io.IOException;

import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.swt.graphics.Image;
import org.fritzing.FritzingMessages;

public class FritzingLabel
	extends FritzingSubpart
{
static final long serialVersionUID = 1;

private String text = 
	FritzingMessages.FritzingPlugin_Tool_CreationTool_FritzingLabel; 

private static Image FRITZING_LABEL_ICON = createImage(LED.class, "icons/label16.gif");  //$NON-NLS-1$

private static int count;

public FritzingLabel() {
	super();
	size.width = 50;
}

public String getLabelContents(){
	return text;
}

public Image getIconImage() {
	return FRITZING_LABEL_ICON;
}

protected String getNewID() {
	return Integer.toString(count++);
}

public Dimension getSize(){
	return new Dimension(size.width, -1);
}

private void readObject(java.io.ObjectInputStream s) throws IOException, ClassNotFoundException {
	s.defaultReadObject();
}

public void setSize(Dimension d) {
	d.height = -1;
	super.setSize(d);
}

public void setLabelContents(String s){
	text = s;
	firePropertyChange("labelContents", null, text); //$NON-NLS-2$//$NON-NLS-1$
}

public String toString() {
	return FritzingMessages.FritzingPlugin_Tool_CreationTool_FritzingLabel
					                + " #" + getID() + " " //$NON-NLS-1$ //$NON-NLS-2$
					                + FritzingMessages.PropertyDescriptor_Label_Text  
					                + "=" + getLabelContents(); //$NON-NLS-1$ 
}




}
