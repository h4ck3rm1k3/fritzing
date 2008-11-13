#include "svgdomdocument.h"
//
SVGDomDocument::SVGDomDocument(  ) 
	: QDomDocument()
{
	
	QDomElement svgroot = createElement("svg");
	svgroot.setAttribute("xmlns","http://www.w3.org/2000/svg");
	svgroot.setAttribute("version","1.2");
	svgroot.setAttribute("baseProfile","tiny");
	svgroot.setAttribute("viewBox","0 0 400 400");  //TODO: set dynamically
	appendChild(svgroot);
	
	QDomElement desc = createElement("desc");
	QDomText descText = createTextNode("Fritzing footprint SVG");
	desc.appendChild(descText);
	
	svgroot.appendChild(desc);
}

void SVGDomDocument::save(QString fileName){
	QFile file(fileName);
	
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
    	QMessageBox::warning(NULL, QObject::tr("FritzBau"),
                     QObject::tr("Cannot write to file %1:\n%2.")
                     .arg(fileName)
                     .arg(file.errorString()));
    }
    
    QTextStream out(&file);
    // This is kinda naughty but QDom seem to have no other way to do it!
    QString xmlDeclaration = "<?xml version='1.0' encoding='UTF-8'?>\n";
    out << xmlDeclaration << toString();
    
    file.close();
}
