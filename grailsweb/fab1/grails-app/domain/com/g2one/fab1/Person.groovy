package com.g2one.fab1

class Person implements Serializable {
	
	String ticket
	static hasMany = [order1:Order1]

	static constraints = {
		ticket(nullable:false)
	}
}
