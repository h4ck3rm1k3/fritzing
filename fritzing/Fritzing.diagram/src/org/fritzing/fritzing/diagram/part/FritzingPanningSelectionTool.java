package org.fritzing.fritzing.diagram.part;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.eclipse.draw2d.IFigure;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPartViewer;
import org.eclipse.gef.RootEditPart;
import org.eclipse.gef.tools.PanningSelectionTool;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;

public class FritzingPanningSelectionTool extends PanningSelectionTool {

	protected Collection getExclusionSet() {
		ArrayList<IFigure> terminals = new ArrayList<IFigure>();
		EditPartViewer viewer = getCurrentViewer();
		RootEditPart rootEditPart = viewer.getRootEditPart();
		List children = rootEditPart.getContents().getChildren();
		for (Iterator it = children.iterator(); it.hasNext(); ) {
			Object obj = it.next();
			if (obj instanceof EditPart) {
				for (Iterator it2 = ((EditPart) obj).getChildren().iterator(); it2.hasNext(); ) {
					Object obj2 = it2.next();
					if (obj2 instanceof Terminal2EditPart) {
						if (((Terminal2EditPart) obj2).hasLeg()) {
							terminals.add(((Terminal2EditPart) obj2).getMainFigure());
						}
					}
				}
			}
		}
			
		return terminals;
	}

}
