package com.g2one.fab1

class Destination implements Serializable {
	String name
	Double taxRate
	List shippingRates
	
	static hasMany = [shippingRates:ShippingRate]
	
	static constraints = {
		taxRate(min:0D, nullable:false)
		name(nullable:false, blank:false)
		shippingRates(nullable:true)
	}
}
