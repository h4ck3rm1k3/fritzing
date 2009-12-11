function filter(group,item) {
	checkbox1 = $('input[name='+group+'][id='+item+']');

	if(checkbox1.attr("checked")) {
		checkbox1.removeAttr("checked")
	} else {
		checkbox1.attr("checked","checked")
	}
	$('#filterForm').submit();
}
