/*******************************************************************************
 * Copyright (c) 2005 Chris Aniszczyk
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Chris Aniszczyk - initial API and implementation
 *    IBM Corporation
 *******************************************************************************/

package org.fritzing;

import org.eclipse.core.runtime.IPath;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.ui.IPathEditorInput;
import org.eclipse.ui.IPersistableElement;
import org.fritzing.model.FritzingDiagram;

public class FritzingEditorInput implements IPathEditorInput {
	
	private IPath path;
	
	public FritzingEditorInput(IPath path) {
		this.path = path;
	}
	
	public boolean exists() {
		return path.toFile().exists();
	}
	
	public ImageDescriptor getImageDescriptor() {
		return FritzingPlugin.imageDescriptorFromPlugin("org.fritzing.rcp","fritzing.gif");
	}
	
	public String getName() {
		return path.toString();
	}
	
	public IPersistableElement getPersistable() {
		return null;
	}
	
	public String getToolTipText() {
		return path.toString();
	}
	
	public Object getAdapter(Class adapter) {
		return null;
	}
	
	public int hashCode() {
		return path.hashCode();
	}
	
	public FritzingDiagram getFritzingDiagram() {
		return new FritzingDiagram();
	}
	
	public IPath getPath() {
		return path;
	}
	
}
