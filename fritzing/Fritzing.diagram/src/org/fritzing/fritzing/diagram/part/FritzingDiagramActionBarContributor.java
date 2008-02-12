/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import org.eclipse.gmf.runtime.diagram.ui.parts.DiagramActionBarContributor;
import org.eclipse.jface.action.IContributionItem;
import org.eclipse.jface.action.ICoolBarManager;
import org.eclipse.jface.action.IToolBarManager;
import org.eclipse.jface.action.SubCoolBarManager;
import org.eclipse.jface.action.ToolBarContributionItem;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.internal.EditorActionBars;

/**
 * @generated
 */
public class FritzingDiagramActionBarContributor extends
		DiagramActionBarContributor {

	/**
	 * @generated
	 */
	protected Class getEditorClass() {
		return FritzingDiagramEditor.class;
	}

	/**
	 * @generated
	 */
	protected String getEditorId() {		
		return FritzingDiagramEditor.ID;
	}
	
	public void init(final IActionBars bars) {
		super.init(bars);
		
		
		// The toolbar seems to be initialized with items from the Diagram menu by some sort of default mechanism
		// that I (jrc) haven't been able to modify
		// so the strategy here is to allow the default items to be populated, then remove them
		// at least the IDs below could be shoved into some xml somewhere, so it wouldn't be hardcoded here.
		
		// by the way, org.fritzing.fritzing.diagram.application.DiagramEditorActionBarAdvisor calls fillCoolBar
		// and the contents of that function can be commented out to remove some diagram-independent items that are also added to the toolbar.
		
		if (bars instanceof EditorActionBars) {
			ICoolBarManager icbm = ((EditorActionBars) bars).getCoolBarManager();
			if (icbm instanceof SubCoolBarManager) {
				IContributionItem[] items = ((SubCoolBarManager) icbm).getParent().getItems();
				for (int i = 0; i < items.length; i++) {
					if (items[i] instanceof ToolBarContributionItem) {						
						IToolBarManager itbm = ((ToolBarContributionItem) items[i]).getToolBarManager();
						if (itbm != null) {
							// remove items by ID from all the subtoolbars
							// nothing happens if that ID doesn't exist in the subtoolbar
							itbm.remove("toolbarFontGroup");
							itbm.remove("fontNameContributionItem");
							itbm.remove("fontSizeContributionItem");
							itbm.remove("fontBoldAction");	
							itbm.remove("fontItalicAction");
							itbm.remove("toolbarColorLineGroup");
							itbm.remove("fontColorContributionItem");
							itbm.remove("fillColorContributionItem");
							itbm.remove("lineColorContributionItem");
							itbm.remove("routerMenu");
							itbm.remove("toolbarCopyAppearanceGroup");
							itbm.remove("copyAppearancePropertiesAction");
							itbm.remove("toolBarViewGroup");
							itbm.remove("selectMenu");
							itbm.remove("arrangeMenu");
							itbm.remove("alignMenu");
							itbm.remove("toolbarEditGroup");
							itbm.remove("toolbarFormatGroup");
							itbm.remove("autoSizeAction");
							itbm.remove("toolbarFilterGroup");
							itbm.remove("showConnectorLabels");
							itbm.remove("hideConnectorLabels");
							itbm.remove("compartmentMenu");
							itbm.remove("toolbarNavigateGroup");
							itbm.remove("toolbarRestGroup");
							itbm.remove("toolbarAdditions");
						}
					}
				}				
			}
		}
	}
	

}
