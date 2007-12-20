/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.providers;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;

/**
 * @generated
 */
public class ElementInitializers {

	/**
	 * @generated
	 */
	public static class Initializers {

		/**
		 * @generated NOT
		 */
		public static final IObjectInitializer Arduino_2001 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getArduino()) {

			protected void init() {
				//				add(createNewElementFeatureInitializer(
				//						FritzingPackage.eINSTANCE.getPart_Terminals(),
				//						new ObjectInitializer[] { terminals_1(), terminals_2(),
				//								terminals_3(), terminals_4(), terminals_5(),
				//								terminals_6(), terminals_7(), terminals_8(),
				//								terminals_9(), terminals_10(), terminals_11(),
				//								terminals_12(), terminals_13(), terminals_14(),
				//								terminals_15(), terminals_16(), terminals_17(),
				//								terminals_18(), terminals_19(), terminals_20(),
				//								terminals_21(), terminals_22(), terminals_23(),
				//								terminals_24(), terminals_25(), terminals_26(),
				//								terminals_27(), terminals_28(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'Arduino\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getArduino())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'RESET\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'3V3\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_3() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'5V\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_4() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'Gnd\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_5() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'Gnd1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_6() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'Vin\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_7() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'A0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_8() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'A1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_9() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'A2\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_10() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'A3\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_11() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'A4\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_12() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'A5\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_13() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_14() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_15() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D2\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_16() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D3\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_17() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D4\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_18() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D5\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_19() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D6\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_20() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D7\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_21() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D8\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_22() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D9\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_23() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D10\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_24() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D11\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_25() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D12\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_26() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'D13\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_27() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'GND\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}

			ObjectInitializer terminals_28() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {

					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'AREF\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				};
			}
		};

		/**
		 * @generated
		 */
		public static final IObjectInitializer LED_2002 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getLED()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'L\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getLED())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'+\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'-\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}
		};

		/**
		 * @generated
		 */
		public static final IObjectInitializer Resistor_2003 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getResistor()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'R\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getResistor())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}
		};

		/**
		 * @generated
		 */
		public static final IObjectInitializer Button_2004 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getButton()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(),
								terminals_3(), terminals_4(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'B\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getButton())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'0p\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'1p\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}

			ObjectInitializer terminals_3() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_3 ObjectInitializer
			}

			ObjectInitializer terminals_4() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_4 ObjectInitializer
			}
		};

		/**
		 * @generated
		 */
		public static final IObjectInitializer Potentiometer_2005 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getPotentiometer()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(),
								terminals_3(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'P\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getPotentiometer())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'T1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'W\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}

			ObjectInitializer terminals_3() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'T2\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_3 ObjectInitializer
			}
		}; // Potentiometer_2005 ObjectInitializer		

		/**
		 * @generated
		 */
		public static final IObjectInitializer FsrSensor_2006 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getFsrSensor()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'FSR\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getFsrSensor())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}
		}; // FsrSensor_2006 ObjectInitializer		

		/**
		 * @generated
		 */
		public static final IObjectInitializer LightSensor_2007 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getLightSensor()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'LS\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getLightSensor())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}
		}; // LightSensor_2007 ObjectInitializer		

		/**
		 * @generated
		 */
		public static final IObjectInitializer Transistor_2009 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getTransistor()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(),
								terminals_3(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'Q\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getTransistor())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'C\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'B\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}

			ObjectInitializer terminals_3() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'E\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_3 ObjectInitializer
			}
		}; // Transistor_2009 ObjectInitializer		

		/**
		 * @generated NOT
		 */
		public static final IObjectInitializer PowerTransistor_2010 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getPowerTransistor()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(),
								terminals_3(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'Q\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getPowerTransistor())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'c\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'b\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}

			ObjectInitializer terminals_3() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'e\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_3 ObjectInitializer
			}
		}; // PowerTransistor_2010 ObjectInitializer		

		/**
		 * @generated
		 */
		public static final IObjectInitializer GenericPart_2011 = new ObjectInitializer(
				FritzingPackage.eINSTANCE.getGenericPart()) {
			protected void init() {
				add(createNewElementFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Terminals(),
						new ObjectInitializer[] { terminals_1(), terminals_2(), }));
				add(createExpressionFeatureInitializer(
						FritzingPackage.eINSTANCE.getPart_Name(),
						FritzingOCLFactory.getExpression("\'G\'", //$NON-NLS-1$
								FritzingPackage.eINSTANCE.getGenericPart())));
			}

			ObjectInitializer terminals_1() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'0\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_1 ObjectInitializer
			}

			ObjectInitializer terminals_2() {
				return new ObjectInitializer(FritzingPackage.eINSTANCE
						.getTerminal()) {
					protected void init() {
						add(createExpressionFeatureInitializer(
								FritzingPackage.eINSTANCE.getTerminal_Name(),
								FritzingOCLFactory
										.getExpression("\'1\'", //$NON-NLS-1$
												FritzingPackage.eINSTANCE
														.getTerminal())));
					}
				}; // terminals_2 ObjectInitializer
			}
		}; // GenericPart_2011 ObjectInitializer

		/**
		 * @generated
		 */
		private Initializers() {
		}

		/**
		 * @generated
		 */
		public static interface IObjectInitializer {

			/**
			 * @generated
			 */
			public void init(EObject instance);
		}

		/**
		 * @generated
		 */
		public static abstract class ObjectInitializer implements
				IObjectInitializer {

			/**
			 * @generated
			 */
			final EClass element;

			/**
			 * @generated
			 */
			private List featureInitializers = new ArrayList();

			/**
			 * @generated
			 */
			ObjectInitializer(EClass element) {
				this.element = element;
				init();
			}

			/**
			 * @generated
			 */
			protected abstract void init();

			/**
			 * @generated
			 */
			protected final IFeatureInitializer add(
					IFeatureInitializer initializer) {
				featureInitializers.add(initializer);
				return initializer;
			}

			/**
			 * @generated
			 */
			public void init(EObject instance) {
				for (Iterator it = featureInitializers.iterator(); it.hasNext();) {
					IFeatureInitializer nextExpr = (IFeatureInitializer) it
							.next();
					try {
						nextExpr.init(instance);
					} catch (RuntimeException e) {
						FritzingDiagramEditorPlugin.getInstance().logError(
								"Feature initialization failed", e); //$NON-NLS-1$						
					}
				}
			}
		}

		/**
		 * @generated
		 */
		interface IFeatureInitializer {

			/**
			 * @generated
			 */
			void init(EObject contextInstance);
		}

		/**
		 * @generated
		 */
		static IFeatureInitializer createNewElementFeatureInitializer(
				EStructuralFeature initFeature,
				ObjectInitializer[] newObjectInitializers) {
			final EStructuralFeature feature = initFeature;
			final ObjectInitializer[] initializers = newObjectInitializers;
			return new IFeatureInitializer() {
				public void init(EObject contextInstance) {
					for (int i = 0; i < initializers.length; i++) {
						EObject newInstance = initializers[i].element
								.getEPackage().getEFactoryInstance().create(
										initializers[i].element);
						if (feature.isMany()) {
							((Collection) contextInstance.eGet(feature))
									.add(newInstance);
						} else {
							contextInstance.eSet(feature, newInstance);
						}
						initializers[i].init(newInstance);
					}
				}
			};
		}

		/**
		 * @generated
		 */
		static IFeatureInitializer createExpressionFeatureInitializer(
				EStructuralFeature initFeature,
				FritzingAbstractExpression valueExpression) {
			final EStructuralFeature feature = initFeature;
			final FritzingAbstractExpression expression = valueExpression;
			return new IFeatureInitializer() {
				public void init(EObject contextInstance) {
					expression.assignTo(feature, contextInstance);
				}
			};
		}
	}
}
