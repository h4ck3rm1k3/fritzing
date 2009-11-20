
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
