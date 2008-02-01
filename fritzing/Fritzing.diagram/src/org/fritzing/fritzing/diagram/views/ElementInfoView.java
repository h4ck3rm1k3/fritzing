package org.fritzing.fritzing.diagram.views;


import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.notation.impl.DiagramImpl;
import org.eclipse.gmf.runtime.notation.impl.EdgeImpl;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyleRange;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.ISelectionListener;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.part.ViewPart;
import org.eclipse.ui.views.contentoutline.ContentOutline;
import org.fritzing.fritzing.IElement;
import org.fritzing.fritzing.ILegConnection;
import org.fritzing.fritzing.IWireConnection;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.diagram.edit.PartDefinition;
import org.fritzing.fritzing.diagram.edit.parts.LegEditPart;
import org.fritzing.fritzing.diagram.edit.parts.PartEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditor;
import org.fritzing.fritzing.impl.LegImpl;
import org.fritzing.fritzing.impl.SketchImpl;
import org.fritzing.fritzing.impl.TerminalImpl;
import org.fritzing.fritzing.impl.WireImpl;

public class ElementInfoView extends ViewPart implements ISelectionListener {
	protected StyledText text;
	protected ArrayList<EditPart> parts;

	public ElementInfoView() {
		parts = new ArrayList<EditPart>();
	}

	public void createPartControl(Composite parent) {
		text = new StyledText(parent, SWT.READ_ONLY);// | SWT.WRAP);
		getViewSite().getPage().addSelectionListener(this);
	}

	public void selectionChanged(IWorkbenchPart part, ISelection selection) {
		if (!(selection instanceof StructuredSelection)) return;
		boolean passThru = false;
		if (part instanceof FritzingDiagramEditor) {
			passThru = true;
		}
		else if (part instanceof ContentOutline) {
			if (selection.isEmpty()) return;	
			passThru = true;
		}			
		if (passThru) {
			inputChanged(null, selection);
		}		
	}
	
	public void inputChanged(Object oldInput, Object newInput) {
		if (!(newInput instanceof StructuredSelection)) return;
		
		parts.clear();
		for (Iterator it = ((StructuredSelection) newInput).iterator(); it.hasNext(); ) {
			Object editPart = it.next();
			if (editPart instanceof EditPart) {
				parts.add((EditPart) editPart);					
			}
		}
		refresh();
	}
	
	/**
	 * Passing the focus request to the viewer's control.
	 */
	public void setFocus() {
		text.setFocus();
	}

	public void refresh() {
		int len = parts.size();
		if (len == 0) {
			text.setText("");
			text.setEnabled(false);
		} else {
			text.setEnabled(true);
			if (len == 1) {
				EditPart part = parts.get(0);
				if (part instanceof PartEditPart) {
					showPartInfo((PartEditPart)part);
				}
				else if (part instanceof LegEditPart) {
					showLegInfo((LegEditPart)part);							
				}
				else if (part instanceof WireEditPart) {
					showWireInfo((WireEditPart)part);
				}
				else if (part instanceof SketchEditPart) {
					showSketchInfo((SketchEditPart)part);
				}
			}
			else {
				text.setText("You have " + parts.size() + " items selected");
			}
		}
	}
	
	protected void showPartInfo(PartEditPart epPart) {
		Part notation = (Part) ((NodeImpl) ((PartEditPart) epPart).getModel()).getElement();
		PartDefinition pd = ((PartEditPart)epPart).getPartDefinition();
		String info =
				pd.getTitle() + "\n" +
				pd.getDescription() + "\n" +
				notation.getGenus() + "." + notation.getSpecies() + "\n" +
				pd.getReference().toString() + "\n" +
				"Version " + notation.getVersion();		
		Iterator<PartDefinition.Author> iAuthors = pd.getAuthors();
		if (iAuthors.hasNext()) {
			StringBuffer sbAuthors = new StringBuffer();
			sbAuthors.append("\nAuthors: ");
			while (iAuthors.hasNext()) {
				PartDefinition.Author a = iAuthors.next();
				sbAuthors.append(a.name);
				if (iAuthors.hasNext())
					sbAuthors.append(", ");
			}
			info += sbAuthors.toString();
		}
		text.setText(info);
		StyleRange style1 = new StyleRange(0, pd.getTitle().length(), null, null, SWT.BOLD);
		text.setStyleRange(style1);
//		pd.getIconLargeFilename()
	}
	

	protected void showLegInfo(LegEditPart epLeg) {
		LegImpl leg = (LegImpl) ((EdgeImpl) ((EditPart) epLeg).getModel()).getElement();		
		ILegConnection ils = leg.getSource();
		ILegConnection ilt = leg.getTarget();
		StringBuffer sb = new StringBuffer();
		sb.append("Leg ");
		appendSourceTargetNames(sb, ils, ilt);

		text.setText(sb.toString());
	}

	protected void showWireInfo(WireEditPart epWire) {
		WireImpl wire = (WireImpl) ((EdgeImpl) ((EditPart) epWire).getModel()).getElement();
		IWireConnection ils = wire.getSource();
		IWireConnection ilt = wire.getTarget();
		StringBuffer sb = new StringBuffer();
		sb.append("Wire ");
		appendSourceTargetNames(sb, ils, ilt);

		text.setText(sb.toString());
	}

	protected void showSketchInfo(SketchEditPart epSketch) {
		SketchImpl sketch = (SketchImpl) ((DiagramImpl) ((EditPart) epSketch).getModel()).getElement();					
		text.setText("Sketch");
	}
	
	protected void appendSourceTargetNames(StringBuffer sb, IElement source, IElement target) {
		appendName(sb, "from", source);
		appendName(sb, " to", target);
	}
	
	protected void appendName(StringBuffer sb, String prefix, IElement il) {
					
		if (il instanceof SketchImpl) {	
			return;
		}
								
		if (il instanceof LegImpl) {
			sb.append(prefix + " leg ");
			appendLegName(sb, (LegImpl) il);
		}
		else if (il instanceof TerminalImpl) {
			sb.append(prefix + " terminal ");
			String s = il.getName();
			if (s != null) {
				sb.append("'" + s + "'");
			}						
		}
		else if (il instanceof WireImpl) {
			sb.append(prefix + " wire ");
			String s = il.getName();
			if (s != null) {
				sb.append("'" + s + "'");
			}						
		}
	}
	
	protected void appendLegName(StringBuffer sb, LegImpl leg) {
		String s = leg.getName();
		if (s != null) {
			sb.append("'" + s + "'");
			return;
		}

		s = leg.getSource().getName();
		if (s != null) {
			sb.append("'" + s + "'");
		}
	}
}