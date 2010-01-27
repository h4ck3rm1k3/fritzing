package com.g2one.fab1

class ShippingRate implements Serializable {

	Integer minQuantity
	Integer maxQuantity
	Double rate
	
	static belongsTo = [destination:Destination]
	
	static constraints = {
		minQuantity(min:1, nullable:false)
		maxQuantity(min:1, nullable:true, validator:{val, obj ->
			if (val == null) return true;
			return (val.compareTo(obj.minQuantity) >= 0) 
		})
		rate(min:0D, nullable:false)
	}
		
}
