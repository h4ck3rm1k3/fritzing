package org.fritzing.fritzing.diagram.export;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.Reader;
import java.util.List;

import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.ShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.parts.IDiagramGraphicalViewer;
import org.eclipse.gmf.runtime.emf.core.util.EMFCoreUtil;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.Element;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;

public class Fritzing2Eagle {

	
	public static String createEagleScript(IDiagramGraphicalViewer viewer) {
		String result = "";
		SketchEditPart sketchEP = (SketchEditPart) viewer.getContents();
		Sketch sketch = (Sketch) ((View) sketchEP.getModel()).getElement();
		
		// TODO: implement transformation here
		for (Part p: sketch.getParts()) {
			result += getLayoutInfo(viewer, p).getLocation().x;
		}
		
		// just for testing until real transformation is in place
		System.out.println(result);
		try {
			Reader r = new FileReader("C:/Users/andre/Fritzing/default.scr");
			StringBuffer sb = new StringBuffer();
			char[] b = new char[8192];
			int n;
			while ((n= r.read(b)) > 0) {
				sb.append(b,0,n);
			}
			result = sb.toString();
			r.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		return result;
	}
	

	private static ShapeEditPart getLayoutInfo(IDiagramGraphicalViewer viewer, Element e) {
		List<EditPart> editParts = viewer.findEditPartsForElement(
				EMFCoreUtil.getProxyID(e), ShapeEditPart.class);
		return (ShapeEditPart) editParts.get(0);
	}
	

}
