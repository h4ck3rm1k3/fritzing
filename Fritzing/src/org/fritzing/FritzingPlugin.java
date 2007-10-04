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
package org.fritzing;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.gef.palette.CombinedTemplateCreationEntry;
import org.eclipse.gef.palette.ConnectionCreationToolEntry;
import org.eclipse.gef.palette.MarqueeToolEntry;
import org.eclipse.gef.palette.PaletteContainer;
import org.eclipse.gef.palette.PaletteDrawer;
import org.eclipse.gef.palette.PaletteEntry;
import org.eclipse.gef.palette.PaletteGroup;
import org.eclipse.gef.palette.PaletteRoot;
import org.eclipse.gef.palette.PaletteSeparator;
import org.eclipse.gef.palette.PaletteStack;
import org.eclipse.gef.palette.PanningSelectionToolEntry;
import org.eclipse.gef.palette.ToolEntry;
import org.eclipse.gef.requests.SimpleFactory;
import org.eclipse.gef.tools.MarqueeSelectionTool;
import org.eclipse.jface.resource.ImageDescriptor;
import org.fritzing.model.AndGate;
import org.fritzing.model.Circuit;
import org.fritzing.model.GroundOutput;
import org.fritzing.model.LED;
import org.fritzing.model.LiveOutput;
import org.fritzing.model.FritzingDiagramFactory;
import org.fritzing.model.FritzingFlowContainer;
import org.fritzing.model.FritzingLabel;
import org.fritzing.model.OrGate;
import org.fritzing.model.XORGate;
import org.osgi.framework.BundleContext;

public class FritzingPlugin
extends org.eclipse.ui.plugin.AbstractUIPlugin
{
	// The plug-in ID
	public static final String PLUGIN_ID = "org.fritzing";

	// The shared instance
	private static FritzingPlugin singleton;

	/**
	 * The constructor
	 */
	public FritzingPlugin(){
		if( singleton == null ){
			singleton = this;
		}
	}
	
	/**
	 * Returns the shared instance
	 *
	 * @return the shared instance
	 */
	public static FritzingPlugin getDefault(){
		return singleton;
	}

	/*
	 * (non-Javadoc)
	 * @see org.eclipse.ui.plugin.AbstractUIPlugin#start(org.osgi.framework.BundleContext)
	 */
	public void start(BundleContext context) throws Exception {
		super.start(context);
		singleton = this;
	}

	/*
	 * (non-Javadoc)
	 * @see org.eclipse.ui.plugin.AbstractUIPlugin#stop(org.osgi.framework.BundleContext)
	 */
	public void stop(BundleContext context) throws Exception {
		singleton = null;
		super.stop(context);
	}

	static private List createCategories(PaletteRoot root){
		List categories = new ArrayList();

		categories.add(createControlGroup(root));
		categories.add(createComponentsDrawer());
		categories.add(createComplexPartsDrawer());
//		categories.add(createTemplateComponentsDrawer());
//		categories.add(createComplexTemplatePartsDrawer());

		return categories;
	}

	static private PaletteContainer createComplexPartsDrawer(){
		PaletteDrawer drawer = new PaletteDrawer(FritzingMessages.FritzingPlugin_Category_ComplexParts_Label, ImageDescriptor.createFromFile(Circuit.class, "icons/can.gif")); //$NON-NLS-1$

		List entries = new ArrayList();

		CombinedTemplateCreationEntry combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_HalfAdder_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_HalfAdder_Description,
				FritzingDiagramFactory.getHalfAdderFactory(),
				ImageDescriptor.createFromFile(Circuit.class, "icons/halfadder16.gif"), //$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/halfadder24.gif") //$NON-NLS-1$
		);
		entries.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_FullAdder_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_FullAdder_Description,
				FritzingDiagramFactory.getFullAdderFactory(),
				ImageDescriptor.createFromFile(Circuit.class, "icons/fulladder16.gif"), //$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/fulladder24.gif") //$NON-NLS-1$
		);
		entries.add(combined);

		drawer.addAll(entries);
		return drawer;
	}

	static private PaletteContainer createComponentsDrawer(){

		PaletteDrawer drawer = new PaletteDrawer(
				FritzingMessages.FritzingPlugin_Category_Components_Label,
				ImageDescriptor.createFromFile(Circuit.class, "icons/comp.gif"));//$NON-NLS-1$

		List entries = new ArrayList();

		CombinedTemplateCreationEntry combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_FlowContainer_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_FlowContainer_Description,
				new SimpleFactory(FritzingFlowContainer.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/fritzingflow16.gif"), //$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/fritzingflow24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_Circuit_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_Circuit_Description,
				new SimpleFactory(Circuit.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/circuit16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/circuit24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		entries.add(new PaletteSeparator());

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_Label_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_Label_Description,
				new SimpleFactory(FritzingLabel.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/label16.gif"), //$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/label24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_LED_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_LED_Description,
				new SimpleFactory(LED.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/ledicon16.gif"), //$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/ledicon24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_ORGate_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_ORGate_Description,
				new SimpleFactory(OrGate.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/or16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/or24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_XORGate_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_XORGate_Description,
				new SimpleFactory(XORGate.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/xor16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/xor24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_ANDGate_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_ANDGate_Description,
				new SimpleFactory(AndGate.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/and16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/and24.gif")//$NON-NLS-1$
		);
		entries.add(combined);

		PaletteStack liveGroundStack = new PaletteStack(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_LiveGroundStack_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_LiveGroundStack_Description, null);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_LiveOutput_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_LiveOutput_Description,
				new SimpleFactory(LiveOutput.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/live16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/live24.gif")//$NON-NLS-1$
		);
		liveGroundStack.add(combined);

		combined = new CombinedTemplateCreationEntry(
				FritzingMessages.FritzingPlugin_Tool_CreationTool_Ground_Label,
				FritzingMessages.FritzingPlugin_Tool_CreationTool_Ground_Description,
				new SimpleFactory(GroundOutput.class),
				ImageDescriptor.createFromFile(Circuit.class, "icons/ground16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/ground24.gif")//$NON-NLS-1$
		);
		liveGroundStack.add(combined);

		entries.add(liveGroundStack);

		drawer.addAll(entries);
		return drawer;
	}

	static private PaletteContainer createControlGroup(PaletteRoot root){
		PaletteGroup controlGroup = new PaletteGroup(
				FritzingMessages.FritzingPlugin_Category_ControlGroup_Label);

		List entries = new ArrayList();

		ToolEntry tool = new PanningSelectionToolEntry();
		entries.add(tool);
		root.setDefaultEntry(tool);

		PaletteStack marqueeStack = new PaletteStack(FritzingMessages.Marquee_Stack, "", null); //$NON-NLS-1$
		marqueeStack.add(new MarqueeToolEntry());
		MarqueeToolEntry marquee = new MarqueeToolEntry();
		marquee.setToolProperty(MarqueeSelectionTool.PROPERTY_MARQUEE_BEHAVIOR, 
				new Integer(MarqueeSelectionTool.BEHAVIOR_CONNECTIONS_TOUCHED));
		marqueeStack.add(marquee);
		marquee = new MarqueeToolEntry();
		marquee.setToolProperty(MarqueeSelectionTool.PROPERTY_MARQUEE_BEHAVIOR, 
				new Integer(MarqueeSelectionTool.BEHAVIOR_CONNECTIONS_TOUCHED 
						| MarqueeSelectionTool.BEHAVIOR_NODES_CONTAINED));
		marqueeStack.add(marquee);
		marqueeStack.setUserModificationPermission(PaletteEntry.PERMISSION_NO_MODIFICATION);
		entries.add(marqueeStack);

		tool = new ConnectionCreationToolEntry(
				FritzingMessages.FritzingPlugin_Tool_ConnectionCreationTool_ConnectionCreationTool_Label,
				FritzingMessages.FritzingPlugin_Tool_ConnectionCreationTool_ConnectionCreationTool_Description,
				null,
				ImageDescriptor.createFromFile(Circuit.class, "icons/connection16.gif"),//$NON-NLS-1$
				ImageDescriptor.createFromFile(Circuit.class, "icons/connection24.gif")//$NON-NLS-1$
		);
		entries.add(tool);
		controlGroup.addAll(entries);
		return controlGroup;
	}

	static PaletteRoot createPalette() {
		PaletteRoot fritzingPalette = new PaletteRoot();
		fritzingPalette.addAll(createCategories(fritzingPalette));
		return fritzingPalette;
	}
	
	/**
	 * Returns an image descriptor for the image file at the given
	 * plug-in relative path
	 *
	 * @param path the path
	 * @return the image descriptor
	 */
	public static ImageDescriptor getImageDescriptor(String path) {
		return imageDescriptorFromPlugin(PLUGIN_ID, path);
	}

}
