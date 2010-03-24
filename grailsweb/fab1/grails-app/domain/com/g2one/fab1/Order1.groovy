package com.g2one.fab1

class Order1 implements Serializable {
	Double width
	Double height
	Double pricePerBoard
	Double totalPrice
	Double taxes
	Double shipping
	Double qualityCheckPrice
	String gid
	String filename
	String destination
	Integer quantity
	Boolean qualityCheck
	Person person
	String email
	Boolean paid
	Date date
		
	static constraints = {
		person()
		email(nullable:false, email:true, blank:true)
		filename(nullable:true)
		width(min:0D)
		height(min:0D)
		pricePerBoard(min:0D)
		totalPrice(min:0D, scale:2)
		qualityCheckPrice(min:0D, scale:2)
		taxes(min:0D, scale:2)
		shipping(min:0D, scale:2)
		quantity(min:1, max:100)
		qualityCheck()
		date()
	}
    
}
