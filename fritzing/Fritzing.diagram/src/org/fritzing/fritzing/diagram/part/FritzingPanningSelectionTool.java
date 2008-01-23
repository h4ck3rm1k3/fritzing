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
import org.fritzing.fritzing.diagram.edit.parts.LegEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;

public class FritzingPanningSelectionTool extends PanningSelectionTool {

	protected Collection getExclusionSet() {
		ArrayList<IFigure> exclude = new ArrayList<IFigure>();
		EditPartViewer viewer = getCurrentViewer();
		RootEditPart rootEditPart = viewer.getRootEditPart();		
		for (Object obj: ((SketchEditPart) rootEditPart.getContents()).getConnections() ) {
			if (obj instanceof LegEditPart) {
				EditPart part = ((LegEditPart) obj).getSource();
				if (part instanceof Terminal2EditPart) {
					IFigure figure = ((Terminal2EditPart) part).getMainFigure();
					exclude.add(figure);
				}
			}
			else if (obj instanceof WireEditPart) {
				IFigure figure = ((WireEditPart) obj).getFigure();
				exclude.add(figure);
			}
		}
					
		return exclude;
	}

}
