package com.g2one.fab1

class User {

	String uuid
	String username
	String email
	String key

	static mapping = {
		table "externals_usernameuuidmodel"
		version false
		 id generator: 'assigned', name: "username", type: 'string'
	}
}