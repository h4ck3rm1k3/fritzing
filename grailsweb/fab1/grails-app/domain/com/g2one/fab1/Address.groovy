package com.g2one.fab1

class Address implements Serializable {
	String firstName
	String lastName
	String company
	String street
	String city
	String state
	String country
	String zip
	
	static constraints = {
		firstName(blank:false)
		lastName(blank:false)
		company(nullable:true,blank:false)
		street(blank:false)
		city(blank:false)
		state(nullable:true,blank:false)
		country(blank:false)
		zip(blank:false)
	}
}
