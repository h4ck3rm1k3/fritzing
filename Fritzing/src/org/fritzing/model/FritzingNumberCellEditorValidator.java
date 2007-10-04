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

import org.eclipse.jface.viewers.ICellEditorValidator;
import org.fritzing.FritzingMessages;

public class FritzingNumberCellEditorValidator
	implements ICellEditorValidator {

private static FritzingNumberCellEditorValidator instance;

public static FritzingNumberCellEditorValidator instance() {
	if (instance == null) 
		instance = new FritzingNumberCellEditorValidator();
	return instance;
}

public String isValid(Object value) {
	try {
		new Integer((String)value);
		return null;
	} catch (NumberFormatException exc) {
		return FritzingMessages.CellEditorValidator_NotANumberMessage;
	}
}

}
