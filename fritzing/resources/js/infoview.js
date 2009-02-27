// based on: http://www.yvoschaap.com/instantedit/instantedit.js

var changing = false;

function fieldEnter(field,evt,idfld) {
	evt = (evt) ? evt : window.event;
	if (evt.keyCode == 13 && field.value!="") {
		elem = document.getElementById( idfld );
		//remove glow
		noLight(elem);
		elem.innerHTML = field.value
		sketch.setInstanceTitle(currentItem.id(), field.value, true);
		changing = false;
		return false;
	} else {
		return true;
	}
}

function fieldBlur(field,idfld) {
	if (field.value!="") {
		elem = document.getElementById( idfld );
		elem.innerHTML = field.value
		sketch.setInstanceTitle(currentItem.id(), field.value, true);
		changing = false;
		return false;
	}
}

function createInput(current, width, height) {
	var inputElem = document.createElement("input");
	with (inputElem) {
		setAttribute("id", current.id + "_field");
		style.width = width + "px";
		style.height = height + "px";
		setAttribute("maxlength", 254);
		setAttribute("type", "text");
		value = current.innerHTML;
		onkeypress = function(event){
			return fieldEnter(this, event, current.id);
		};
		onfocus = function(){
			highLight(this);
		};
		onblur = function(){
			noLight(this);
			return fieldBlur(this, current.id);
		};
	}
	return inputElem;
}

function createTextarea(current, width, height) {
	var textareaElem = document.createElement("textarea");
	with (textareaElem) {
		setAttribute("id", current.id + "_field");
		style.width = width + "px";
		style.height = height + "px";
		setAttribute("maxlength", 254);
		setAttribute("type", "text");
		value = current.innerHTML;
		onfocus = function(){
			highLight(this);
		};
		onblur = function(){
			noLight(this);
		};
	}
	return textareaElem;
}

//edit field created
function editBox(current) {
	//alert(current.nodeName+' '+changing);
	if(!changing){
		var width = widthEl(current.id) + 20;
		var height =heightEl(current.id) + 2;

		var input;
		if(height < 40){
			if(width < 70)	width = 70;
			input = createInput(current, width, height);
		} else {
			if(width < 70) width = 70;
			if(height < 50) height = 50;
			input = createTextarea(current, width, height);
		}
		
		current.removeChild(current.firstChild);
		current.appendChild(input);
		input.select();
		
		changing = true;
	}
	current.firstChild.focus();
}

function showPartLabel(current, showIt) {
    sketch.showPartLabel(currentItem.id(), showIt);
}

//get width of text element
function widthEl(span){
	if (document.layers){
		w=document.layers[span].clip.width-30;
	} else if (document.all && !document.getElementById){
		w=document.all[span].offsetWidth-30;
	} else if(document.getElementById){
		w=document.getElementById(span).offsetWidth-30;
	}
	return w;
}

//get height of text element
function heightEl(span){
	if (document.layers){
		h=document.layers[span].clip.height;
	} else if (document.all && !document.getElementById){
		h=document.all[span].offsetHeight;
	} else if(document.getElementById){
		h=document.getElementById(span).offsetHeight;
	}
	return h;
}

function highLight(span){
	span.style.border = "1px solid black";          
}

function noLight(span){
	span.style.border = "0px";
}

var currProps = {};

function doSwap(family,name,currValue) {
	var value = document.getElementById(name).value;
	currProps[name] = value;
	//alert("refmodel " + refModel);
	for(propName in currProps) {
		refModel.recordProperty(propName, currProps[propName]);
	}
	var moduleID = refModel.retrieveModuleIdWith(family);
	//swapper.swapSelected(moduleID);
	sketch.swapSelected(moduleID);
}

function setWireColor(wireTitle, wireId, newColor) {
	sketch.changeWireColor(newColor);
}

function setWireWidth(wireTitle, wireId, newWidth) {
    sketch.changeWireWidth(newWidth);
}

function toggleVisibility(emitter,idToAffect) {
	var isBeingShown = emitter.innerHTML == "[-]";
	var elemToAffect = document.getElementById(idToAffect);
	if(isBeingShown) {
		elemToAffect.style.display = "none";
		emitter.innerHTML = "[+]";
	} else {
		elemToAffect.style.display = "block";
		emitter.innerHTML = "[-]";
	}
	infoView.setBlockVisibility(idToAffect,!isBeingShown);
}
