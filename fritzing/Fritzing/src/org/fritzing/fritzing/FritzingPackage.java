/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;

import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EPackage;
import org.eclipse.emf.ecore.EReference;

/**
 * <!-- begin-user-doc -->
 * The <b>Package</b> for the model.
 * It contains accessors for the meta objects to represent
 * <ul>
 *   <li>each class,</li>
 *   <li>each feature of each class,</li>
 *   <li>each enum,</li>
 *   <li>and each data type</li>
 * </ul>
 * <!-- end-user-doc -->
 * @see org.fritzing.fritzing.FritzingFactory
 * @model kind="package"
 * @generated
 */
public interface FritzingPackage extends EPackage {
	/**
	 * The package name.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	String eNAME = "fritzing";

	/**
	 * The package namespace URI.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	String eNS_URI = "http://www.fritzing.org";

	/**
	 * The package namespace name.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	String eNS_PREFIX = "fritzing";

	/**
	 * The singleton instance of the package.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	FritzingPackage eINSTANCE = org.fritzing.fritzing.impl.FritzingPackageImpl.init();

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.DocumentRootImpl <em>Document Root</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.DocumentRootImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getDocumentRoot()
	 * @generated
	 */
	int DOCUMENT_ROOT = 0;

	/**
	 * The feature id for the '<em><b>Mixed</b></em>' attribute list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int DOCUMENT_ROOT__MIXED = 0;

	/**
	 * The feature id for the '<em><b>XMLNS Prefix Map</b></em>' map.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int DOCUMENT_ROOT__XMLNS_PREFIX_MAP = 1;

	/**
	 * The feature id for the '<em><b>XSI Schema Location</b></em>' map.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int DOCUMENT_ROOT__XSI_SCHEMA_LOCATION = 2;

	/**
	 * The feature id for the '<em><b>Sketch</b></em>' containment reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int DOCUMENT_ROOT__SKETCH = 3;

	/**
	 * The number of structural features of the '<em>Document Root</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int DOCUMENT_ROOT_FEATURE_COUNT = 4;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.ElementImpl <em>Element</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.ElementImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getElement()
	 * @generated
	 */
	int ELEMENT = 2;

	/**
	 * The number of structural features of the '<em>Element</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int ELEMENT_FEATURE_COUNT = 0;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.TerminalImpl <em>Terminal</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.TerminalImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getTerminal()
	 * @generated
	 */
	int TERMINAL = 1;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TERMINAL__ID = ELEMENT_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TERMINAL__NAME = ELEMENT_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TERMINAL__PARENT = ELEMENT_FEATURE_COUNT + 2;

	/**
	 * The feature id for the '<em><b>Leg</b></em>' containment reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TERMINAL__LEG = ELEMENT_FEATURE_COUNT + 3;

	/**
	 * The number of structural features of the '<em>Terminal</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TERMINAL_FEATURE_COUNT = ELEMENT_FEATURE_COUNT + 4;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.ConnectionImpl <em>Connection</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.ConnectionImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getConnection()
	 * @generated
	 */
	int CONNECTION = 3;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int CONNECTION__NAME = ELEMENT_FEATURE_COUNT + 0;

	/**
	 * The number of structural features of the '<em>Connection</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int CONNECTION_FEATURE_COUNT = ELEMENT_FEATURE_COUNT + 1;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.WireImpl <em>Wire</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.WireImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getWire()
	 * @generated
	 */
	int WIRE = 12;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.TrackImpl <em>Track</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.TrackImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getTrack()
	 * @generated
	 */
	int TRACK = 13;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.ITrackConnection <em>ITrack Connection</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.ITrackConnection
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getITrackConnection()
	 * @generated
	 */
	int ITRACK_CONNECTION = 14;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.PartImpl <em>Part</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.PartImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getPart()
	 * @generated
	 */
	int PART = 4;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__ID = ELEMENT_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__NAME = ELEMENT_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__SPECIES = ELEMENT_FEATURE_COUNT + 2;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__GENUS = ELEMENT_FEATURE_COUNT + 3;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__VERSION = ELEMENT_FEATURE_COUNT + 4;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__FOOTPRINT = ELEMENT_FEATURE_COUNT + 5;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__TERMINALS = ELEMENT_FEATURE_COUNT + 6;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__PARENT = ELEMENT_FEATURE_COUNT + 7;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART__TRACKS = ELEMENT_FEATURE_COUNT + 8;

	/**
	 * The number of structural features of the '<em>Part</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int PART_FEATURE_COUNT = ELEMENT_FEATURE_COUNT + 9;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.ResistorImpl <em>Resistor</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.ResistorImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getResistor()
	 * @generated
	 */
	int RESISTOR = 6;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.LEDImpl <em>LED</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.LEDImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getLED()
	 * @generated
	 */
	int LED = 7;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.CompositeImpl <em>Composite</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.CompositeImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getComposite()
	 * @generated
	 */
	int COMPOSITE = 8;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.SketchImpl <em>Sketch</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.SketchImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getSketch()
	 * @generated
	 */
	int SKETCH = 9;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.BreadboardImpl <em>Breadboard</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.BreadboardImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getBreadboard()
	 * @generated
	 */
	int BREADBOARD = 11;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.ModuleImpl <em>Module</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.ModuleImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getModule()
	 * @generated
	 */
	int MODULE = 10;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.GenericPartImpl <em>Generic Part</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.GenericPartImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getGenericPart()
	 * @generated
	 */
	int GENERIC_PART = 5;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__ID = PART__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__NAME = PART__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__SPECIES = PART__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__GENUS = PART__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__VERSION = PART__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__FOOTPRINT = PART__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__TERMINALS = PART__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__PARENT = PART__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART__TRACKS = PART__TRACKS;

	/**
	 * The number of structural features of the '<em>Generic Part</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int GENERIC_PART_FEATURE_COUNT = PART_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__ID = PART__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__NAME = PART__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__SPECIES = PART__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__GENUS = PART__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__VERSION = PART__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__FOOTPRINT = PART__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__TERMINALS = PART__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__PARENT = PART__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__TRACKS = PART__TRACKS;

	/**
	 * The feature id for the '<em><b>Value</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR__VALUE = PART_FEATURE_COUNT + 0;

	/**
	 * The number of structural features of the '<em>Resistor</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int RESISTOR_FEATURE_COUNT = PART_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__ID = PART__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__NAME = PART__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__SPECIES = PART__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__GENUS = PART__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__VERSION = PART__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__FOOTPRINT = PART__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__TERMINALS = PART__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__PARENT = PART__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__TRACKS = PART__TRACKS;

	/**
	 * The feature id for the '<em><b>Color</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED__COLOR = PART_FEATURE_COUNT + 0;

	/**
	 * The number of structural features of the '<em>LED</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LED_FEATURE_COUNT = PART_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__ID = PART__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__NAME = PART__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__SPECIES = PART__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__GENUS = PART__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__VERSION = PART__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__FOOTPRINT = PART__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__TERMINALS = PART__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__PARENT = PART__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__TRACKS = PART__TRACKS;

	/**
	 * The feature id for the '<em><b>Parts</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__PARTS = PART_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Wires</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE__WIRES = PART_FEATURE_COUNT + 1;

	/**
	 * The number of structural features of the '<em>Composite</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int COMPOSITE_FEATURE_COUNT = PART_FEATURE_COUNT + 2;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__ID = COMPOSITE__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__NAME = COMPOSITE__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__SPECIES = COMPOSITE__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__GENUS = COMPOSITE__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__VERSION = COMPOSITE__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__FOOTPRINT = COMPOSITE__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__TERMINALS = COMPOSITE__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__PARENT = COMPOSITE__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__TRACKS = COMPOSITE__TRACKS;

	/**
	 * The feature id for the '<em><b>Parts</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__PARTS = COMPOSITE__PARTS;

	/**
	 * The feature id for the '<em><b>Wires</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH__WIRES = COMPOSITE__WIRES;

	/**
	 * The number of structural features of the '<em>Sketch</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int SKETCH_FEATURE_COUNT = COMPOSITE_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__ID = COMPOSITE__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__NAME = COMPOSITE__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__SPECIES = COMPOSITE__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__GENUS = COMPOSITE__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__VERSION = COMPOSITE__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__FOOTPRINT = COMPOSITE__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__TERMINALS = COMPOSITE__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__PARENT = COMPOSITE__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__TRACKS = COMPOSITE__TRACKS;

	/**
	 * The feature id for the '<em><b>Parts</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__PARTS = COMPOSITE__PARTS;

	/**
	 * The feature id for the '<em><b>Wires</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE__WIRES = COMPOSITE__WIRES;

	/**
	 * The number of structural features of the '<em>Module</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int MODULE_FEATURE_COUNT = COMPOSITE_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__ID = PART__ID;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__NAME = PART__NAME;

	/**
	 * The feature id for the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__SPECIES = PART__SPECIES;

	/**
	 * The feature id for the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__GENUS = PART__GENUS;

	/**
	 * The feature id for the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__VERSION = PART__VERSION;

	/**
	 * The feature id for the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__FOOTPRINT = PART__FOOTPRINT;

	/**
	 * The feature id for the '<em><b>Terminals</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__TERMINALS = PART__TERMINALS;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__PARENT = PART__PARENT;

	/**
	 * The feature id for the '<em><b>Tracks</b></em>' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__TRACKS = PART__TRACKS;

	/**
	 * The feature id for the '<em><b>Lines</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__LINES = PART_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Columns</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD__COLUMNS = PART_FEATURE_COUNT + 1;

	/**
	 * The number of structural features of the '<em>Breadboard</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int BREADBOARD_FEATURE_COUNT = PART_FEATURE_COUNT + 2;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int WIRE__NAME = CONNECTION__NAME;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int WIRE__PARENT = CONNECTION_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int WIRE__TARGET = CONNECTION_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int WIRE__SOURCE = CONNECTION_FEATURE_COUNT + 2;

	/**
	 * The number of structural features of the '<em>Wire</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int WIRE_FEATURE_COUNT = CONNECTION_FEATURE_COUNT + 3;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TRACK__NAME = CONNECTION__NAME;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TRACK__PARENT = CONNECTION_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TRACK__TARGET = CONNECTION_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TRACK__SOURCE = CONNECTION_FEATURE_COUNT + 2;

	/**
	 * The number of structural features of the '<em>Track</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int TRACK_FEATURE_COUNT = CONNECTION_FEATURE_COUNT + 3;

	/**
	 * The number of structural features of the '<em>ITrack Connection</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int ITRACK_CONNECTION_FEATURE_COUNT = 0;


	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.impl.LegImpl <em>Leg</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.impl.LegImpl
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getLeg()
	 * @generated
	 */
	int LEG = 15;

	/**
	 * The feature id for the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LEG__NAME = CONNECTION__NAME;

	/**
	 * The feature id for the '<em><b>Parent</b></em>' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LEG__PARENT = CONNECTION_FEATURE_COUNT + 0;

	/**
	 * The feature id for the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LEG__TARGET = CONNECTION_FEATURE_COUNT + 1;

	/**
	 * The feature id for the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LEG__SOURCE = CONNECTION_FEATURE_COUNT + 2;

	/**
	 * The number of structural features of the '<em>Leg</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int LEG_FEATURE_COUNT = CONNECTION_FEATURE_COUNT + 3;

	/**
	 * The meta object id for the '{@link org.fritzing.fritzing.ILegConnection <em>ILeg Connection</em>}' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see org.fritzing.fritzing.ILegConnection
	 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getILegConnection()
	 * @generated
	 */
	int ILEG_CONNECTION = 16;

	/**
	 * The number of structural features of the '<em>ILeg Connection</em>' class.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 * @ordered
	 */
	int ILEG_CONNECTION_FEATURE_COUNT = 0;


	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.DocumentRoot <em>Document Root</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Document Root</em>'.
	 * @see org.fritzing.fritzing.DocumentRoot
	 * @generated
	 */
	EClass getDocumentRoot();

	/**
	 * Returns the meta object for the attribute list '{@link org.fritzing.fritzing.DocumentRoot#getMixed <em>Mixed</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute list '<em>Mixed</em>'.
	 * @see org.fritzing.fritzing.DocumentRoot#getMixed()
	 * @see #getDocumentRoot()
	 * @generated
	 */
	EAttribute getDocumentRoot_Mixed();

	/**
	 * Returns the meta object for the map '{@link org.fritzing.fritzing.DocumentRoot#getXMLNSPrefixMap <em>XMLNS Prefix Map</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the map '<em>XMLNS Prefix Map</em>'.
	 * @see org.fritzing.fritzing.DocumentRoot#getXMLNSPrefixMap()
	 * @see #getDocumentRoot()
	 * @generated
	 */
	EReference getDocumentRoot_XMLNSPrefixMap();

	/**
	 * Returns the meta object for the map '{@link org.fritzing.fritzing.DocumentRoot#getXSISchemaLocation <em>XSI Schema Location</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the map '<em>XSI Schema Location</em>'.
	 * @see org.fritzing.fritzing.DocumentRoot#getXSISchemaLocation()
	 * @see #getDocumentRoot()
	 * @generated
	 */
	EReference getDocumentRoot_XSISchemaLocation();

	/**
	 * Returns the meta object for the containment reference '{@link org.fritzing.fritzing.DocumentRoot#getSketch <em>Sketch</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the containment reference '<em>Sketch</em>'.
	 * @see org.fritzing.fritzing.DocumentRoot#getSketch()
	 * @see #getDocumentRoot()
	 * @generated
	 */
	EReference getDocumentRoot_Sketch();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Terminal <em>Terminal</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Terminal</em>'.
	 * @see org.fritzing.fritzing.Terminal
	 * @generated
	 */
	EClass getTerminal();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Terminal#getId <em>Id</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Id</em>'.
	 * @see org.fritzing.fritzing.Terminal#getId()
	 * @see #getTerminal()
	 * @generated
	 */
	EAttribute getTerminal_Id();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Terminal#getName <em>Name</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Name</em>'.
	 * @see org.fritzing.fritzing.Terminal#getName()
	 * @see #getTerminal()
	 * @generated
	 */
	EAttribute getTerminal_Name();

	/**
	 * Returns the meta object for the container reference '{@link org.fritzing.fritzing.Terminal#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the container reference '<em>Parent</em>'.
	 * @see org.fritzing.fritzing.Terminal#getParent()
	 * @see #getTerminal()
	 * @generated
	 */
	EReference getTerminal_Parent();

	/**
	 * Returns the meta object for the containment reference '{@link org.fritzing.fritzing.Terminal#getLeg <em>Leg</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the containment reference '<em>Leg</em>'.
	 * @see org.fritzing.fritzing.Terminal#getLeg()
	 * @see #getTerminal()
	 * @generated
	 */
	EReference getTerminal_Leg();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Element <em>Element</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Element</em>'.
	 * @see org.fritzing.fritzing.Element
	 * @generated
	 */
	EClass getElement();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Connection <em>Connection</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Connection</em>'.
	 * @see org.fritzing.fritzing.Connection
	 * @generated
	 */
	EClass getConnection();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Connection#getName <em>Name</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Name</em>'.
	 * @see org.fritzing.fritzing.Connection#getName()
	 * @see #getConnection()
	 * @generated
	 */
	EAttribute getConnection_Name();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Wire <em>Wire</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Wire</em>'.
	 * @see org.fritzing.fritzing.Wire
	 * @generated
	 */
	EClass getWire();

	/**
	 * Returns the meta object for the reference '{@link org.fritzing.fritzing.Wire#getSource <em>Source</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the reference '<em>Source</em>'.
	 * @see org.fritzing.fritzing.Wire#getSource()
	 * @see #getWire()
	 * @generated
	 */
	EReference getWire_Source();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Track <em>Track</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Track</em>'.
	 * @see org.fritzing.fritzing.Track
	 * @generated
	 */
	EClass getTrack();

	/**
	 * Returns the meta object for the container reference '{@link org.fritzing.fritzing.Track#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the container reference '<em>Parent</em>'.
	 * @see org.fritzing.fritzing.Track#getParent()
	 * @see #getTrack()
	 * @generated
	 */
	EReference getTrack_Parent();

	/**
	 * Returns the meta object for the reference '{@link org.fritzing.fritzing.Track#getTarget <em>Target</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the reference '<em>Target</em>'.
	 * @see org.fritzing.fritzing.Track#getTarget()
	 * @see #getTrack()
	 * @generated
	 */
	EReference getTrack_Target();

	/**
	 * Returns the meta object for the reference '{@link org.fritzing.fritzing.Track#getSource <em>Source</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the reference '<em>Source</em>'.
	 * @see org.fritzing.fritzing.Track#getSource()
	 * @see #getTrack()
	 * @generated
	 */
	EReference getTrack_Source();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.ITrackConnection <em>ITrack Connection</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>ITrack Connection</em>'.
	 * @see org.fritzing.fritzing.ITrackConnection
	 * @generated
	 */
	EClass getITrackConnection();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Leg <em>Leg</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Leg</em>'.
	 * @see org.fritzing.fritzing.Leg
	 * @generated
	 */
	EClass getLeg();

	/**
	 * Returns the meta object for the container reference '{@link org.fritzing.fritzing.Leg#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the container reference '<em>Parent</em>'.
	 * @see org.fritzing.fritzing.Leg#getParent()
	 * @see #getLeg()
	 * @generated
	 */
	EReference getLeg_Parent();

	/**
	 * Returns the meta object for the reference '{@link org.fritzing.fritzing.Leg#getTarget <em>Target</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the reference '<em>Target</em>'.
	 * @see org.fritzing.fritzing.Leg#getTarget()
	 * @see #getLeg()
	 * @generated
	 */
	EReference getLeg_Target();

	/**
	 * Returns the meta object for the reference '{@link org.fritzing.fritzing.Leg#getSource <em>Source</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the reference '<em>Source</em>'.
	 * @see org.fritzing.fritzing.Leg#getSource()
	 * @see #getLeg()
	 * @generated
	 */
	EReference getLeg_Source();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.ILegConnection <em>ILeg Connection</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>ILeg Connection</em>'.
	 * @see org.fritzing.fritzing.ILegConnection
	 * @generated
	 */
	EClass getILegConnection();

	/**
	 * Returns the meta object for the reference '{@link org.fritzing.fritzing.Wire#getTarget <em>Target</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the reference '<em>Target</em>'.
	 * @see org.fritzing.fritzing.Wire#getTarget()
	 * @see #getWire()
	 * @generated
	 */
	EReference getWire_Target();

	/**
	 * Returns the meta object for the container reference '{@link org.fritzing.fritzing.Wire#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the container reference '<em>Parent</em>'.
	 * @see org.fritzing.fritzing.Wire#getParent()
	 * @see #getWire()
	 * @generated
	 */
	EReference getWire_Parent();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Part <em>Part</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Part</em>'.
	 * @see org.fritzing.fritzing.Part
	 * @generated
	 */
	EClass getPart();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Part#getId <em>Id</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Id</em>'.
	 * @see org.fritzing.fritzing.Part#getId()
	 * @see #getPart()
	 * @generated
	 */
	EAttribute getPart_Id();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Part#getName <em>Name</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Name</em>'.
	 * @see org.fritzing.fritzing.Part#getName()
	 * @see #getPart()
	 * @generated
	 */
	EAttribute getPart_Name();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Part#getSpecies <em>Species</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Species</em>'.
	 * @see org.fritzing.fritzing.Part#getSpecies()
	 * @see #getPart()
	 * @generated
	 */
	EAttribute getPart_Species();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Part#getGenus <em>Genus</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Genus</em>'.
	 * @see org.fritzing.fritzing.Part#getGenus()
	 * @see #getPart()
	 * @generated
	 */
	EAttribute getPart_Genus();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Part#getVersion <em>Version</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Version</em>'.
	 * @see org.fritzing.fritzing.Part#getVersion()
	 * @see #getPart()
	 * @generated
	 */
	EAttribute getPart_Version();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Part#getFootprint <em>Footprint</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Footprint</em>'.
	 * @see org.fritzing.fritzing.Part#getFootprint()
	 * @see #getPart()
	 * @generated
	 */
	EAttribute getPart_Footprint();

	/**
	 * Returns the meta object for the containment reference list '{@link org.fritzing.fritzing.Part#getTerminals <em>Terminals</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the containment reference list '<em>Terminals</em>'.
	 * @see org.fritzing.fritzing.Part#getTerminals()
	 * @see #getPart()
	 * @generated
	 */
	EReference getPart_Terminals();

	/**
	 * Returns the meta object for the container reference '{@link org.fritzing.fritzing.Part#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the container reference '<em>Parent</em>'.
	 * @see org.fritzing.fritzing.Part#getParent()
	 * @see #getPart()
	 * @generated
	 */
	EReference getPart_Parent();

	/**
	 * Returns the meta object for the containment reference list '{@link org.fritzing.fritzing.Part#getTracks <em>Tracks</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the containment reference list '<em>Tracks</em>'.
	 * @see org.fritzing.fritzing.Part#getTracks()
	 * @see #getPart()
	 * @generated
	 */
	EReference getPart_Tracks();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Resistor <em>Resistor</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Resistor</em>'.
	 * @see org.fritzing.fritzing.Resistor
	 * @generated
	 */
	EClass getResistor();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Resistor#getValue <em>Value</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Value</em>'.
	 * @see org.fritzing.fritzing.Resistor#getValue()
	 * @see #getResistor()
	 * @generated
	 */
	EAttribute getResistor_Value();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.LED <em>LED</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>LED</em>'.
	 * @see org.fritzing.fritzing.LED
	 * @generated
	 */
	EClass getLED();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.LED#getColor <em>Color</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Color</em>'.
	 * @see org.fritzing.fritzing.LED#getColor()
	 * @see #getLED()
	 * @generated
	 */
	EAttribute getLED_Color();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Composite <em>Composite</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Composite</em>'.
	 * @see org.fritzing.fritzing.Composite
	 * @generated
	 */
	EClass getComposite();

	/**
	 * Returns the meta object for the containment reference list '{@link org.fritzing.fritzing.Composite#getParts <em>Parts</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the containment reference list '<em>Parts</em>'.
	 * @see org.fritzing.fritzing.Composite#getParts()
	 * @see #getComposite()
	 * @generated
	 */
	EReference getComposite_Parts();

	/**
	 * Returns the meta object for the containment reference list '{@link org.fritzing.fritzing.Composite#getWires <em>Wires</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the containment reference list '<em>Wires</em>'.
	 * @see org.fritzing.fritzing.Composite#getWires()
	 * @see #getComposite()
	 * @generated
	 */
	EReference getComposite_Wires();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Sketch <em>Sketch</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Sketch</em>'.
	 * @see org.fritzing.fritzing.Sketch
	 * @generated
	 */
	EClass getSketch();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Breadboard <em>Breadboard</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Breadboard</em>'.
	 * @see org.fritzing.fritzing.Breadboard
	 * @generated
	 */
	EClass getBreadboard();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Breadboard#getLines <em>Lines</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Lines</em>'.
	 * @see org.fritzing.fritzing.Breadboard#getLines()
	 * @see #getBreadboard()
	 * @generated
	 */
	EAttribute getBreadboard_Lines();

	/**
	 * Returns the meta object for the attribute '{@link org.fritzing.fritzing.Breadboard#getColumns <em>Columns</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for the attribute '<em>Columns</em>'.
	 * @see org.fritzing.fritzing.Breadboard#getColumns()
	 * @see #getBreadboard()
	 * @generated
	 */
	EAttribute getBreadboard_Columns();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.Module <em>Module</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Module</em>'.
	 * @see org.fritzing.fritzing.Module
	 * @generated
	 */
	EClass getModule();

	/**
	 * Returns the meta object for class '{@link org.fritzing.fritzing.GenericPart <em>Generic Part</em>}'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the meta object for class '<em>Generic Part</em>'.
	 * @see org.fritzing.fritzing.GenericPart
	 * @generated
	 */
	EClass getGenericPart();

	/**
	 * Returns the factory that creates the instances of the model.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the factory that creates the instances of the model.
	 * @generated
	 */
	FritzingFactory getFritzingFactory();

	/**
	 * <!-- begin-user-doc -->
	 * Defines literals for the meta objects that represent
	 * <ul>
	 *   <li>each class,</li>
	 *   <li>each feature of each class,</li>
	 *   <li>each enum,</li>
	 *   <li>and each data type</li>
	 * </ul>
	 * <!-- end-user-doc -->
	 * @generated
	 */
	interface Literals {
		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.DocumentRootImpl <em>Document Root</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.DocumentRootImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getDocumentRoot()
		 * @generated
		 */
		EClass DOCUMENT_ROOT = eINSTANCE.getDocumentRoot();

		/**
		 * The meta object literal for the '<em><b>Mixed</b></em>' attribute list feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute DOCUMENT_ROOT__MIXED = eINSTANCE.getDocumentRoot_Mixed();

		/**
		 * The meta object literal for the '<em><b>XMLNS Prefix Map</b></em>' map feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference DOCUMENT_ROOT__XMLNS_PREFIX_MAP = eINSTANCE.getDocumentRoot_XMLNSPrefixMap();

		/**
		 * The meta object literal for the '<em><b>XSI Schema Location</b></em>' map feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference DOCUMENT_ROOT__XSI_SCHEMA_LOCATION = eINSTANCE.getDocumentRoot_XSISchemaLocation();

		/**
		 * The meta object literal for the '<em><b>Sketch</b></em>' containment reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference DOCUMENT_ROOT__SKETCH = eINSTANCE.getDocumentRoot_Sketch();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.TerminalImpl <em>Terminal</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.TerminalImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getTerminal()
		 * @generated
		 */
		EClass TERMINAL = eINSTANCE.getTerminal();

		/**
		 * The meta object literal for the '<em><b>Id</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute TERMINAL__ID = eINSTANCE.getTerminal_Id();

		/**
		 * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute TERMINAL__NAME = eINSTANCE.getTerminal_Name();

		/**
		 * The meta object literal for the '<em><b>Parent</b></em>' container reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference TERMINAL__PARENT = eINSTANCE.getTerminal_Parent();

		/**
		 * The meta object literal for the '<em><b>Leg</b></em>' containment reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference TERMINAL__LEG = eINSTANCE.getTerminal_Leg();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.ElementImpl <em>Element</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.ElementImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getElement()
		 * @generated
		 */
		EClass ELEMENT = eINSTANCE.getElement();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.ConnectionImpl <em>Connection</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.ConnectionImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getConnection()
		 * @generated
		 */
		EClass CONNECTION = eINSTANCE.getConnection();

		/**
		 * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute CONNECTION__NAME = eINSTANCE.getConnection_Name();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.WireImpl <em>Wire</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.WireImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getWire()
		 * @generated
		 */
		EClass WIRE = eINSTANCE.getWire();

		/**
		 * The meta object literal for the '<em><b>Source</b></em>' reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference WIRE__SOURCE = eINSTANCE.getWire_Source();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.TrackImpl <em>Track</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.TrackImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getTrack()
		 * @generated
		 */
		EClass TRACK = eINSTANCE.getTrack();

		/**
		 * The meta object literal for the '<em><b>Parent</b></em>' container reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference TRACK__PARENT = eINSTANCE.getTrack_Parent();

		/**
		 * The meta object literal for the '<em><b>Target</b></em>' reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference TRACK__TARGET = eINSTANCE.getTrack_Target();

		/**
		 * The meta object literal for the '<em><b>Source</b></em>' reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference TRACK__SOURCE = eINSTANCE.getTrack_Source();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.ITrackConnection <em>ITrack Connection</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.ITrackConnection
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getITrackConnection()
		 * @generated
		 */
		EClass ITRACK_CONNECTION = eINSTANCE.getITrackConnection();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.LegImpl <em>Leg</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.LegImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getLeg()
		 * @generated
		 */
		EClass LEG = eINSTANCE.getLeg();

		/**
		 * The meta object literal for the '<em><b>Parent</b></em>' container reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference LEG__PARENT = eINSTANCE.getLeg_Parent();

		/**
		 * The meta object literal for the '<em><b>Target</b></em>' reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference LEG__TARGET = eINSTANCE.getLeg_Target();

		/**
		 * The meta object literal for the '<em><b>Source</b></em>' reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference LEG__SOURCE = eINSTANCE.getLeg_Source();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.ILegConnection <em>ILeg Connection</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.ILegConnection
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getILegConnection()
		 * @generated
		 */
		EClass ILEG_CONNECTION = eINSTANCE.getILegConnection();

		/**
		 * The meta object literal for the '<em><b>Target</b></em>' reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference WIRE__TARGET = eINSTANCE.getWire_Target();

		/**
		 * The meta object literal for the '<em><b>Parent</b></em>' container reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference WIRE__PARENT = eINSTANCE.getWire_Parent();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.PartImpl <em>Part</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.PartImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getPart()
		 * @generated
		 */
		EClass PART = eINSTANCE.getPart();

		/**
		 * The meta object literal for the '<em><b>Id</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute PART__ID = eINSTANCE.getPart_Id();

		/**
		 * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute PART__NAME = eINSTANCE.getPart_Name();

		/**
		 * The meta object literal for the '<em><b>Species</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute PART__SPECIES = eINSTANCE.getPart_Species();

		/**
		 * The meta object literal for the '<em><b>Genus</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute PART__GENUS = eINSTANCE.getPart_Genus();

		/**
		 * The meta object literal for the '<em><b>Version</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute PART__VERSION = eINSTANCE.getPart_Version();

		/**
		 * The meta object literal for the '<em><b>Footprint</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute PART__FOOTPRINT = eINSTANCE.getPart_Footprint();

		/**
		 * The meta object literal for the '<em><b>Terminals</b></em>' containment reference list feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference PART__TERMINALS = eINSTANCE.getPart_Terminals();

		/**
		 * The meta object literal for the '<em><b>Parent</b></em>' container reference feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference PART__PARENT = eINSTANCE.getPart_Parent();

		/**
		 * The meta object literal for the '<em><b>Tracks</b></em>' containment reference list feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference PART__TRACKS = eINSTANCE.getPart_Tracks();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.ResistorImpl <em>Resistor</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.ResistorImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getResistor()
		 * @generated
		 */
		EClass RESISTOR = eINSTANCE.getResistor();

		/**
		 * The meta object literal for the '<em><b>Value</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute RESISTOR__VALUE = eINSTANCE.getResistor_Value();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.LEDImpl <em>LED</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.LEDImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getLED()
		 * @generated
		 */
		EClass LED = eINSTANCE.getLED();

		/**
		 * The meta object literal for the '<em><b>Color</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute LED__COLOR = eINSTANCE.getLED_Color();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.CompositeImpl <em>Composite</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.CompositeImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getComposite()
		 * @generated
		 */
		EClass COMPOSITE = eINSTANCE.getComposite();

		/**
		 * The meta object literal for the '<em><b>Parts</b></em>' containment reference list feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference COMPOSITE__PARTS = eINSTANCE.getComposite_Parts();

		/**
		 * The meta object literal for the '<em><b>Wires</b></em>' containment reference list feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EReference COMPOSITE__WIRES = eINSTANCE.getComposite_Wires();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.SketchImpl <em>Sketch</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.SketchImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getSketch()
		 * @generated
		 */
		EClass SKETCH = eINSTANCE.getSketch();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.BreadboardImpl <em>Breadboard</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.BreadboardImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getBreadboard()
		 * @generated
		 */
		EClass BREADBOARD = eINSTANCE.getBreadboard();

		/**
		 * The meta object literal for the '<em><b>Lines</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute BREADBOARD__LINES = eINSTANCE.getBreadboard_Lines();

		/**
		 * The meta object literal for the '<em><b>Columns</b></em>' attribute feature.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		EAttribute BREADBOARD__COLUMNS = eINSTANCE.getBreadboard_Columns();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.ModuleImpl <em>Module</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.ModuleImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getModule()
		 * @generated
		 */
		EClass MODULE = eINSTANCE.getModule();

		/**
		 * The meta object literal for the '{@link org.fritzing.fritzing.impl.GenericPartImpl <em>Generic Part</em>}' class.
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @see org.fritzing.fritzing.impl.GenericPartImpl
		 * @see org.fritzing.fritzing.impl.FritzingPackageImpl#getGenericPart()
		 * @generated
		 */
		EClass GENERIC_PART = eINSTANCE.getGenericPart();

	}

} //FritzingPackage
