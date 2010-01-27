	
$(document).ready(function(){
	$("#qualityCheck").change(updateTotal);
	$("#quantity").change(updateTotal);
	$("#destination").change(updateTotal);
	updateTotal();
});

function updateTotal()
{
	var qual = $("#qualityCheck").attr('checked') ? getPricePerQualityCheck() : 0;
	var quantStr = $("#quantity").attr("value");
	var rexp = /^\d+$/;
	if (rexp.test(quantStr)) {
		var quant = parseInt(quantStr);
		var total = qual + (quant * getPricePerBoard());	
		var shipping = 0;
		var taxes = 0;
		// get the value of destination and use it to calculate shipping and taxes
		var dests = getDestinations()
		var sel = $("#destination option:selected");
		//alert("dests " + dests + " " + dests.length + " " + sel + " " + sel.text());
		for (var i = 0; i < dests.length; i++) {
			if (dests[i].name == sel.text()) {
				taxes = dests[i].taxRate * total
				total += taxes;
				for (var j = 0; j < dests[i].shippingRates.length; j++) {
					if (quant >= dests[i].shippingRates[j].minQuantity) {
						if ((dests[i].shippingRates[j].maxQuantity == null) || (quant <= dests[i].shippingRates[j].maxQuantity)) {
							shipping = dests[i].shippingRates[j].rate;
							break;
						}
					}
				}
				total += shipping;
				break;
			}
		}
		
		$("#taxes").val(taxes);
		$("#shipping").val(shipping);
		$("#qualityCheckPrice").val(qual);
		$("#totalPrice").val(total);
		$("#taxesLabel").text(taxes.toFixed(2));
		$("#shippingLabel").text(shipping.toFixed(2));
		$("#qualityCheckPriceLabel").text(qual.toFixed(2));
		$("#totalPriceLabel").text("EUR" + total.toFixed(2));
	}
	else {
		$("#upload_progress").text("'" + quantStr + "' is not a number");
	}
}

