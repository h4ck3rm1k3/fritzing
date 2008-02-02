/*
 * (c) Fachhochschule Potsdam
 */

package org.fritzing.fritzing.diagram.view.factories;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.gmf.runtime.diagram.ui.view.factories.AbstractShapeViewFactory;
import org.eclipse.gmf.runtime.notation.NotationFactory;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.gmf.runtime.notation.PropertiesSetStyle;
import org.eclipse.emf.ecore.EcorePackage;

public class PartShapeViewFactory extends AbstractShapeViewFactory {

	public static final String PARTS_PROPERTIES_STYLE_NAME = "parts_properties_style";
	public static final String PARTS_ROTATION_PROPERTY_NAME = "parts_rotation_property";
	
	protected List createStyles(View view) {
		List styles = new ArrayList();
		styles.add(NotationFactory.eINSTANCE.createShapeStyle());
		PropertiesSetStyle properties = NotationFactory.eINSTANCE.createPropertiesSetStyle();
		properties.setName(PARTS_PROPERTIES_STYLE_NAME);
		properties.createProperty(PARTS_ROTATION_PROPERTY_NAME,
								EcorePackage.eINSTANCE.getEIntegerObject(), null);
		styles.add(properties);
		return styles;
	}

}
