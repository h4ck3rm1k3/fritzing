// based on: http://www.yvoschaap.com/instantedit/instantedit.js

var changing = false;
var lastGoodWidth = 0;
var lastGoodHeight = 0;
var lastGoodResistance = 0;

function fieldEnter(field,evt,idfld) {
	evt = (evt) ? evt : window.event;
	if (evt.keyCode == 13 && field.value!="") {
		elem = document.getElementById( idfld );
		//remove glow
		noLight(elem);
		elem.innerHTML = field.value
		sketch.setInstanceTitle(currentItem.id(), field.value, true, true, false);
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
		sketch.setInstanceTitle(currentItem.id(), field.value, true, true, false);
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
	mainWindow.swapSelected(currProps, family, name);
}

function setWireColor(wireTitle, wireId, newColor) {
	sketch.changeWireColor(newColor);
}

function setWireWidthMils(wireTitle, wireId, newWidth) {
    sketch.changeWireWidthMils(newWidth);
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

function loadBoardImage() {
    alert("load board image");
}

function resizeBoard() {    
	var reg = /^(\d{1,3}$)|(\d{1,3}\.\d$)/;
	
	var w = document.getElementById("boardwidth").value;			
   	if (!reg.test(w)) {
	    alert("board width is not a number");
	    setLastGoodSize();
	    return;
	}
	if (w < 3) {
	    alert("board width must be at least 3 mm");
	    return;
	}
	
   	var h = document.getElementById("boardheight").value;		
   	if (!reg.test(h)) {
	    alert("board height is not a number");
	    setLastGoodSize();
	    return;
	}	
	if (h < 3) {
	    alert("board width must be at least 3 mm");
	    setLastGoodSize();
	    return;
	}
	
	lastGoodWidth = w;
	lastGoodHeight = h;
	
    sketch.resizeBoard(w, h);
}

function updateBoardSize(w, h) {
    lastGoodWidth = w;
    lastGoodHeight = h;
    var bw = document.getElementById("boardwidth");
    if (bw) {
        bw.value = w;
    }
    var bh = document.getElementById("boardheight");
    if (bh) {
        bh.value = h;
    }
}

function setResistance() {
	var reg = /^((\d{1,3})|(\d{1,3}\.\d))[kMG]{0,1}[\u03A9]{0,1}$/;
	
	var r = document.getElementById("sResistance").value;
	var s = r;			
   	if (!reg.test(r) || (s < 0) || (s > 9900000000)) {
	    alert("The value '" + r + "' doesn't fit within the range of 1.0 to 9.9G");
	    setLastResistance();
	    return;
	}
		
	lastGoodResistance = r;
	
    sketch.setResistance(r, "");
}

function setResistanceEnter(evt) {
	evt = (evt) ? evt : window.event;	
	if (evt.keyCode == 13) {
	    setResistance();
		return false;
	} 
		
	return true;
}

function resizeBoardWidth() {
    resizeBoard();
}

function setChipLabel() {
	var r = document.getElementById("sChipLabel").value;
    sketch.setChipLabel(r);
}

function setChipLabelEnter(evt) {
	evt = (evt) ? evt : window.event;	
	if (evt.keyCode == 13) {
	    setChipLabel();
		return false;
	} 
		
	return true;
}

function resizeBoardWidth() {
    resizeBoard();
}

function resizeBoardHeight() {
    resizeBoard();
}

function resizeBoardWidthEnter(evt) {
	evt = (evt) ? evt : window.event;	
	if (evt.keyCode == 13) {
	    resizeBoard();
		return false;
	} 
		
	return true;
}

function resizeBoardHeightEnter(evt) {
	evt = (evt) ? evt : window.event;	
	if (evt.keyCode == 13) {
	    resizeBoard();
		return false;
	} 
		
	return true;
}

function setLastGoodSize() {
    document.getElementById("boardwidth").value = lastGoodWidth;
    document.getElementById("boardheight").value = lastGoodHeight;
}

function setLastResistance() {
    document.getElementById("sResistance").value = lastGoodResistance;
}

