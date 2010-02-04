package com.g2one.fab1

class Person implements Serializable {
	
	String username
	List orders
	
	static hasMany = [orders:Order1]
	
	static constraints = {
		username(unique:true)
		orders()
	}
}
