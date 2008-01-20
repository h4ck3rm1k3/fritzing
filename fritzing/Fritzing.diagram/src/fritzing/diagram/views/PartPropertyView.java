package fritzing.diagram.views;


import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.part.*;
import org.eclipse.ui.views.contentoutline.ContentOutline;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.notation.impl.DiagramImpl;
import org.eclipse.gmf.runtime.notation.impl.EdgeImpl;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.eclipse.jface.viewers.*;
import org.eclipse.swt.graphics.Image;
import org.eclipse.jface.action.*;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.ui.*;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.SWT;
import org.fritzing.fritzing.ILegConnection;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditor;
import org.fritzing.fritzing.diagram.part.ModelElementSelectionPage;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.parts.LegEditPart;
import org.fritzing.fritzing.diagram.edit.parts.PartEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.impl.LegImpl;
import org.fritzing.fritzing.impl.SketchImpl;
import org.fritzing.fritzing.impl.WireImpl;

/**
 * This sample class demonstrates how to plug-in a new
 * workbench view. The view shows data obtained from the
 * model. The sample creates a dummy model on the fly,
 * but a real implementation would connect to the model
 * available either in this or another plug-in (e.g. the workspace).
 * The view is connected to the model using a content provider.
 * <p>
 * The view uses a label provider to define how model
 * objects should be presented in the view. Each
 * view can present the same model objects using
 * different labels and icons, if needed. Alternatively,
 * a single label provider can be shared between views
 * in order to ensure that objects of the same type are
 * presented in the same way everywhere.
 * <p>
 */

public class PartPropertyView extends ViewPart implements ISelectionListener {
	private TableViewer viewer;
	private Action action1;
	private Action action2;
	private Action doubleClickAction;

	/*
	 * The content provider class is responsible for
	 * providing objects to the view. It can wrap
	 * existing objects in adapters or simply return
	 * objects as-is. These objects may be sensitive
	 * to the current input of the view, or ignore
	 * it and always show the same content 
	 * (like Task List, for example).
	 */
	 
	class ViewContentProvider implements IStructuredContentProvider {
		ArrayList<EditPart> parts;
		
		public ViewContentProvider() {
			parts = new ArrayList<EditPart>();
		}
		
		public Object[] getElements(Object parent) {
			int len = parts.size();
			if (len == 0) {
				return new String[0];
			}
			
			if (len == 1) {
				EditPart part = parts.get(0);
				if (part instanceof PartEditPart) {
					String[] strings = new String[3];
					int index = 0;
					Part notation = (Part) ((NodeImpl) ((EditPart) part).getModel()).getElement();
					strings[index++] = notation.getGenus();
					strings[index++] = notation.getSpecies();
					strings[index++] = notation.getVersion();	
					return strings;
				}
				else if (part instanceof LegEditPart) {
					LegImpl leg = (LegImpl) ((EdgeImpl) ((EditPart) part).getModel()).getElement();		
					String[] strings = new String[1];
					ILegConnection ilc = leg.getTarget();
					strings[0] = "Leg from " + leg.getSource().getName();
					if (!(ilc instanceof SketchImpl)) {
						strings[0] += " to " + ilc.getName();
					}
					return strings;									
				}
				else if (part instanceof WireEditPart) {
					WireImpl wire = (WireImpl) ((EdgeImpl) ((EditPart) part).getModel()).getElement();
					String[] strings = new String[1];
					strings[0] = "Wire connecting " + wire.getSource().getName() + " to " + wire.getTarget().getName();
					return strings;									
					
				}
				else if (part instanceof SketchEditPart) {
					SketchImpl sketch = (SketchImpl) ((DiagramImpl) ((EditPart) part).getModel()).getElement();					
					String[] strings = new String[1];
					strings[0] = "Sketch";
					return strings;									
				}
			}
			else {
				String[] strings = new String[1];
				strings[0] = "You have " + parts.size() + " items selected";
				return strings;
			}
			
			
			
						
			return new String[0];
		}

		public void dispose() {
			
		}

		public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
			if (!(newInput instanceof StructuredSelection)) return;
			
			parts.clear();
			for (Iterator it = ((StructuredSelection) newInput).iterator(); it.hasNext(); ) {
				Object editPart = it.next();
				if (editPart instanceof EditPart) {
					parts.add((EditPart) editPart);					
				}
			}
			viewer.refresh();			
		}
	}
	class ViewLabelProvider extends LabelProvider implements ITableLabelProvider {
		public String getColumnText(Object obj, int index) {
			return getText(obj);
		}
		public Image getColumnImage(Object obj, int index) {
			return getImage(obj);
		}
		public Image getImage(Object obj) {
			return PlatformUI.getWorkbench().
					getSharedImages().getImage(ISharedImages.IMG_OBJ_ELEMENT);
		}
	}

	/**
	 * The constructor.
	 */
	public PartPropertyView() {
	}

	/**
	 * This is a callback that will allow us
	 * to create the viewer and initialize it.
	 */
	public void createPartControl(Composite parent) {
		viewer = new TableViewer(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL);
		viewer.setContentProvider(new ViewContentProvider());
		viewer.setLabelProvider(new ViewLabelProvider());
		//viewer.setSorter(new NameSorter());
		viewer.setInput(getViewSite());
		makeActions();
		hookContextMenu();
		hookDoubleClickAction();
		contributeToActionBars();
		
		getViewSite().getPage().addSelectionListener(this);
		
	}

	private void hookContextMenu() {
		MenuManager menuMgr = new MenuManager("#PopupMenu");
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager manager) {
				PartPropertyView.this.fillContextMenu(manager);
			}
		});
		Menu menu = menuMgr.createContextMenu(viewer.getControl());
		viewer.getControl().setMenu(menu);
		getSite().registerContextMenu(menuMgr, viewer);
	}

	private void contributeToActionBars() {
		IActionBars bars = getViewSite().getActionBars();
		fillLocalPullDown(bars.getMenuManager());
		fillLocalToolBar(bars.getToolBarManager());
	}

	private void fillLocalPullDown(IMenuManager manager) {
		manager.add(action1);
		manager.add(new Separator());
		manager.add(action2);
	}

	private void fillContextMenu(IMenuManager manager) {
		manager.add(action1);
		manager.add(action2);
		// Other plug-ins can contribute there actions here
		manager.add(new Separator(IWorkbenchActionConstants.MB_ADDITIONS));
	}
	
	private void fillLocalToolBar(IToolBarManager manager) {
		manager.add(action1);
		manager.add(action2);
	}

	private void makeActions() {
		action1 = new Action() {
			public void run() {
				showMessage("Action 1 executed");
			}
		};
		action1.setText("Action 1");
		action1.setToolTipText("Action 1 tooltip");
		action1.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().
			getImageDescriptor(ISharedImages.IMG_OBJS_INFO_TSK));
		
		action2 = new Action() {
			public void run() {
				showMessage("Action 2 executed");
			}
		};
		action2.setText("Action 2");
		action2.setToolTipText("Action 2 tooltip");
		action2.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().
				getImageDescriptor(ISharedImages.IMG_OBJS_INFO_TSK));
		doubleClickAction = new Action() {
			public void run() {
				ISelection selection = viewer.getSelection();
				Object obj = ((IStructuredSelection)selection).getFirstElement();
				showMessage("Double-click detected on "+obj.toString());
			}
		};
	}

	private void hookDoubleClickAction() {
		viewer.addDoubleClickListener(new IDoubleClickListener() {
			public void doubleClick(DoubleClickEvent event) {
				doubleClickAction.run();
			}
		});
	}
	private void showMessage(String message) {
		MessageDialog.openInformation(
			viewer.getControl().getShell(),
			"Part Property View",
			message);
	}

	/**
	 * Passing the focus request to the viewer's control.
	 */
	public void setFocus() {
		viewer.getControl().setFocus();
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
			viewer.getContentProvider().inputChanged(viewer, null, selection);	
		}		
	}
}