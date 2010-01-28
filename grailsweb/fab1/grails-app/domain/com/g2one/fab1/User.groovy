package com.g2one.fab1

class User {

	String uuid
	String username

	static mapping = {
		version false
		 id generator: 'assigned', name: "uuid", type: 'string'
	}
}