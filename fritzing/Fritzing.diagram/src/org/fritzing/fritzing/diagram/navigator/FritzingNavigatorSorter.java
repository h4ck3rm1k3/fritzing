/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.navigator;

import org.eclipse.jface.viewers.ViewerSorter;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;

/**
 * @generated
 */
public class FritzingNavigatorSorter extends ViewerSorter {

	/**
	 * @generated
	 */
	private static final int GROUP_CATEGORY = 4003;

	/**
	 * @generated
	 */
	public int category(Object element) {
		if (element instanceof FritzingNavigatorItem) {
			FritzingNavigatorItem item = (FritzingNavigatorItem) element;
			return FritzingVisualIDRegistry.getVisualID(item.getView());
		}
		return GROUP_CATEGORY;
	}

}
