

	  
$(document).ready(function(){
	toggleBillingAddress()
	$("#billing_enabled").change(toggleBillingAddress)
	syncShippingAndBillingValue()
});

var toggleBillingAddress = function() {
	var checkbox = $("#billing_enabled");
	var checked = checkbox.is(':checked');
	enableBillingAddress(checked,true)
}

var enableBillingAddress = function(enabled,clean) {
	$(".billing-customer-address input").each(function() {
		if($(this).attr("type") != "checkbox") {
			if (!enabled) {
				var billing = $(this)
				billing.attr('disabled', true);
				var value = "";
				var name = billing.attr("name")
				name = name.substring(name.indexOf('-'))
				$(".shipping-customer-address  input[name$="+name+"]")
					.each(function() {
						billing.val($(this).val());
					})
			} else {
				$(this).removeAttr('disabled');
				if(clean) {
					$(this).val("")
				}
			}   
		}
	})
	
	$(".billing-customer-address span[class=required]").each(function() {
		if(enabled) {
			$(this).show()
		} else {
			$(this).hide()
		}
	})
	
	$(".billing-customer-address   label[class=error]").each(function() {
		$(this).remove()
	})
}

var syncShippingAndBillingValue = function() {
	$(".shipping-customer-address  input").each(function() {
		var mirror = $(this) 
		$(this).change(function(){
			var name = mirror.attr("name")
			name = name.substring(name.indexOf('-'))
			$(".billing-customer-address input[name$="+name+"]")
				.each(function() {
					if($(this).attr('disabled')) {
						$(this).val(mirror.val())
					}
				})
		})
	})
}

