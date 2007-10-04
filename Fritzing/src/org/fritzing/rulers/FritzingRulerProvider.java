/*******************************************************************************
 * Copyright (c) 2003, 2005 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package org.fritzing.rulers;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.gef.commands.Command;
import org.eclipse.gef.rulers.RulerChangeListener;
import org.eclipse.gef.rulers.RulerProvider;
import org.fritzing.model.FritzingGuide;
import org.fritzing.model.FritzingRuler;
import org.fritzing.model.commands.CreateGuideCommand;
import org.fritzing.model.commands.DeleteGuideCommand;
import org.fritzing.model.commands.MoveGuideCommand;

/**
 * @author Pratik Shah
 */
public class FritzingRulerProvider
	extends RulerProvider
{

private FritzingRuler ruler;
private PropertyChangeListener rulerListener = new PropertyChangeListener() {
	public void propertyChange(PropertyChangeEvent evt) {
		if (evt.getPropertyName().equals(FritzingRuler.PROPERTY_CHILDREN)) {
			FritzingGuide guide = (FritzingGuide)evt.getNewValue();
			if (getGuides().contains(guide)) {
				guide.addPropertyChangeListener(guideListener);
			} else {
				guide.removePropertyChangeListener(guideListener);
			}
			for (int i = 0; i < listeners.size(); i++) {
				((RulerChangeListener)listeners.get(i))
						.notifyGuideReparented(guide);
			}
		} else {
			for (int i = 0; i < listeners.size(); i++) {
				((RulerChangeListener)listeners.get(i))
						.notifyUnitsChanged(ruler.getUnit());
			}
		}
	}
};
private PropertyChangeListener guideListener = new PropertyChangeListener() {
	public void propertyChange(PropertyChangeEvent evt) {
		if (evt.getPropertyName().equals(FritzingGuide.PROPERTY_CHILDREN)) {
			for (int i = 0; i < listeners.size(); i++) {
				((RulerChangeListener)listeners.get(i))
						.notifyPartAttachmentChanged(evt.getNewValue(), evt.getSource());
			}
		} else {
			for (int i = 0; i < listeners.size(); i++) {
				((RulerChangeListener)listeners.get(i))
						.notifyGuideMoved(evt.getSource());
			}
		}
	}
};

public FritzingRulerProvider(FritzingRuler ruler) {
	this.ruler = ruler;
	this.ruler.addPropertyChangeListener(rulerListener);
	List guides = getGuides();
	for (int i = 0; i < guides.size(); i++) {
		((FritzingGuide)guides.get(i)).addPropertyChangeListener(guideListener);
	}
}

public List getAttachedModelObjects(Object guide) {
	return new ArrayList(((FritzingGuide)guide).getParts());
}

public Command getCreateGuideCommand(int position) {
	return new CreateGuideCommand(ruler, position);
}

public Command getDeleteGuideCommand(Object guide) {
	return new DeleteGuideCommand((FritzingGuide)guide, ruler);
}

public Command getMoveGuideCommand(Object guide, int pDelta) {
	return new MoveGuideCommand((FritzingGuide)guide, pDelta);
}

public int[] getGuidePositions() {
	List guides = getGuides();
	int[] result = new int[guides.size()];
	for (int i = 0; i < guides.size(); i++) {
		result[i] = ((FritzingGuide)guides.get(i)).getPosition();
	}
	return result;
}

public Object getRuler() {
	return ruler;
}

public int getUnit() {
	return ruler.getUnit();
}

public void setUnit(int newUnit) {
	ruler.setUnit(newUnit);
}

public int getGuidePosition(Object guide) {
	return ((FritzingGuide)guide).getPosition();
}

public List getGuides() {
	return ruler.getGuides();
}

}
