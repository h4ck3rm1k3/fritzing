package com.g2one.fab1

import java.util.zip.*
import grails.converters.JSON
import aesencrypt.AESEncrypt

class FormController {
	
	def beforeInterceptor = {
		log.trace("Executing action $actionName with params $params")
	}
	
	// note: event names and state names mustn't match or the state machine mechanism gets confused
	def buyBoardFlow = {
	
		start {
			action {
				log.trace("entering flow with params " + params)
				if (!params.x) {
					log.trace("no params x")
					response.status = 404;
					return no()
				}
				
				log.trace("user " + params.x )
				
				def theUser = null
				for (user in User.list()) {
					log.trace("user list " + user.username)
					def result = AESEncrypt.decrypt(user.ekey, params.x)
					def decode = AESEncrypt.decode(user.uuid)
					log.trace("result " + result + " " + decode)
					if (Arrays.equals(result, decode)) {
						theUser = user
						break;
					}
				}
				
				log.trace("before session " + session.getId())
				
				//def test = User.findByUsername("jones")
				//log.trace("jones result " + test + " " +  test?.uuid)
				//test.delete()
				
				//def user = User.findByUuidAndUsername(params.UUID, params.username)
				//log.trace("user result " + user + " " + user?.username)
				
				if (!theUser) {
					response.status = 404;
					return no()
				}
				
				flow.person = Person.findByUsername(theUser.username)
				if (flow.person == null) {
					flow.person = new Person(username:theUser.username);
					flow.person.orders = []
					flow.person.save(flush:true);
				}
				
				flow.originatingEmail = theUser.email
				theUser.delete()
					
				return yes()
			}
			
			on("no"){
				log.trace("should be to error exit")
			}.to "errorExit"
			on("yes").to "uploadReady"
		}
		
		uploadReady {
			log.trace("upload ready")
			render(view:"start")
			on("upload").to("uploader")
		}
				
		uploader {
			action {
				if (!flow.order1) {
					flow.order1 = new Order1()
					flow.order1.person = flow.person
					flow.order1.quantity = 1
					flow.order1.qualityCheck = false
					flow.order1.gid = UUID.randomUUID().toString() 
					flow.order1.paid = false
					flow.order1.date = new Date()
					flow.order1.email = flow.originatingEmail
					
					// if the person is already known, find the most recent order from that person, and use that email
					if (flow.person.orders) {
						def sz = flow.person.orders.size()
						if (sz > 0) {
							def mostRecentOrder = flow.person.orders[sz - 1]
							flow.order1.email = mostRecentOrder.email
						}
					}	
				}
				
				flow.order1.totalPrice = 0	
				flow.order1.pricePerBoard = 0
				flow.order1.width = 0
				flow.order1.height = 0
				flow.order1.taxes = 0
				flow.order1.shipping = 0
											
				if (this.uploadOk(flow.order1)) {				// use "this." or grails thinks "uploadOk" is an event
					log.trace("orders " + flow.order1.width + " " + flow.order1.filename)
					return yes()
				}
				return no()
			}
			
			on("no").to("uploadReady")
			on("yes"){
				log.trace("destinations " + servletContext.destinations)
			}.to("quantity")
		}
		
		quantity { 
			on("upload").to("uploader")
			
			on("orderBoard") {
				log.trace("order params " + params)
								
				bindData(flow.order1, params, [include:["quantity", "qualityCheck", "qualityCheckPrice", "destination", "totalPrice", "taxes", "shipping"]])
				flow.order1.validate()
				if (flow.order1.hasErrors()) {
					flow.order1.errors.allErrors.each {
						log.trace("order error " + it)
					}
					
					return error()
				}

			}.to("buy")
			
		}
		
		buy {
			on("paypal") {
				if (params["confirm email"] != params["email"]) {
					flow.order1.errors.rejectValue('email', 
						"",														// Error code within the grails-app/i18n/message.properties
						['email', 'class com.g2one.fapp3.Order1'] as Object[],                          		// Groovy list cast to Object[]
						'[Property [{0}] of class [{1}] does not match confirmation]')					
					flow.order1.errors.allErrors.each {
						log.trace("email error " + it)
					}
					return error()
				}
				
				bindData(flow.order1, params, [include:["email"]])
				flow.order1.validate()
				if (flow.order1.hasErrors()) {
					flow.order1.errors.allErrors.each {
						log.trace("order error " + it)
					}
					
					return error()
				}
				
			}.to("doIt")
		}
		
		doIt { 

			action { 
			
				flow.order1.date = new Date()
				flow.person.addToOrders(flow.order1)
				flow.person.save(flush:true)

				session.orderID = flow.order1.id

				def successURL = g.createLink(absolute: true, controller: 'form', action: 'orderSuccess').encodeAsURL() 
				def notifyURL = g.createLink(absolute: true, controller: 'form', action: 'orderNotify').encodeAsURL() 
				def cancelURL = g.createLink(absolute: true, controller: 'form', action: 'orderCancel').encodeAsURL() 
				
				//<form id="order-form" action="https://www.paypal.com/cgi-bin/webscr" method="post"> 
				//<input name="cmd" type="hidden" value="_xclick" /> 
				//<input name="currency_code" type="hidden" value="EUR" /> 
				//<input name="lc" type="hidden" value="de" /> 
				//<input name="business" type="hidden" value="order@ixds.de" /> 
				//<input name="item_name" type="hidden" value="Fritzing Starter Kit" /> 
				//<input name="amount" type="hidden" value="55.00" /> 
				// <input name="quantity" type="hidden" value="1" /> 
				// <input name="tax" type="hidden" value="10.45" /> 
				// <input name="shipping" type="hidden" value="4.50" /> 
				// <input name="return" type="hidden" value="http://fritzing.org/shop/order-successful" /> 
				// <input name="cancel_return" type="hidden" value="http://fritzing.org/shop/order-failed" /> 

				def url = new StringBuffer("https://www.paypal.com/cgi-bin/webscr?") 
				url << "cmd=_xclick&" 
				url << "business=order@ixds.de&" 
				url << "lc=de&"
				url << "item_name=FritzingFab&" 
				url << "item_number=1&" 
				url << "quantity=${flow.order1.quantity}&" 
				url << "tax=${ g.formatNumber(number: flow.order1.taxes, type: 'number', minFractionDigits: 2, maxFractionDigits: 2)}&" 
				url << "shipping=${g.formatNumber(number: flow.order1.shipping, type: 'number', minFractionDigits: 2, maxFractionDigits: 2)}&" 
				url << "amount=${g.formatNumber(number: flow.order1.totalPrice - flow.order1.taxes - flow.order1.shipping, type: 'number', minFractionDigits: 2, maxFractionDigits: 2)}&" 
				url << "currency_code=EUR&" 
				url << "notify_url=${notifyURL}&" 
				url << "return=${successURL}&" 
				url << "cancel_return=${cancelURL}" 

				log.trace "Redirection to PayPal with URL: $url" 
				redirect(url: url) 

				return yes()
			} 
			
			on("yes").to "done"
			// never gets here
			on("orderSuccess").to "paid" 
			on("orderCancel").to "cancelled" 
			on("orderNotify").to "notified" 
		}
		
		paid {
		}
		
		cancelled {
		}
		
		notified {
		}
		
		done {
		}
		
		errorExit {
			log.trace("error exit")
			//render text:"not allowed"
		}
	}

	def orderCancel = {
		
		log.trace("order cancel params " + params +  " " + session.getId())
		render(view:"orderCancel", model:[order: Order1.get(session.orderID)])

	}
	
	def orderSuccess = {
		log.trace("order success params " + params)
	}
	
	def orderNotify = {
		log.trace("order notify params " + params)
	}

	def uploadOk(order1) {
		log.trace("checking upload")	
		
		if (!params.fzzFile) {
			return closeErr("Upload request missing fzzFile")
		}	
					
		def maxSize = 1024 * 1024
		def file = request.getFile('fzzFile')
		if (!file) {
			return closeErr("File not found")
		}
		
		if (file.empty) {
			return closeErr("File '${file.originalFilename}' is empty")
		}
		
		if (!file.originalFilename.endsWith(".fzz")) {
			return closeErr("File '${file.originalFilename}' must be an '.fzz' file.")
		}
		
		if (file.size > maxSize) {
			return closeErr("File '${file.originalFilename}' must be < ${maxSize / 1024}k.")
		}
		
		def prefix = "./uploads/${order1.gid}"
		def fn =  prefix + "_" + file.originalFilename
		file.transferTo( new java.io.File(fn))
		def fz = unzipFz(fn, prefix)
		if (!fz) {
			return closeErr("Sketch not found in '${file.originalFilename}'")
		}
		
		def doc = new XmlParser().parse(new File(fz))
		if (!doc) {
			return closeErr("File '${file.originalFilename}': sketch can't be parsed")
		}
		
		if (doc.name() != "module") {
			return closeErr("File '${file.originalFilename}': sketch has improper format (1)")
		}
		
		if (!doc.instances) {
			return closeErr("File '${file.originalFilename}': sketch is empty")
		}
		
		if (doc.instances.size() != 1) {
			return closeErr("File '${file.originalFilename}': sketch has improper format (2)")
		}
				
		order1.filename = file.originalFilename
		log.trace("order 1 " + order1.filename)
		
		//log.trace("file " + file.getName())
		
		def customIDs = findCustomBoard(fn, order1)
		if (customIDs.size() > 1) {
			return closeOrderMultiple(file)
		}

		def boards = findBoard(file, order1, doc, customIDs) 
		if (boards < 0) {
			// error
			return null		
		}			
			
		if (boards > 1) {
			return closeOrderMultiple(file)
		}
		
		if (boards == 1) {
			log.trace("order 1 width" + order1.width)
			return true
		}
		
		return closeErr("File '${file.originalFilename}': no board found")
	}
	
	def closeOrderMultiple(file) {
		return closeErr("File '${file.originalFilename}': found multiple boards, can only deal with one")
	}
	
	def closeErr(msg) {
		flash.uploadMessage = msg
		return false
	}
	
	def saveOrder(order1) {
		if (!order1.save(flush:true)) {
			order1.errors.each {
				log.trace("save error " + it)
			}
		}
	}

	def findCustomBoard(fn, order1) {
		try 
		{
			def moduleIds = []
			def zipfile = new ZipFile(fn)
			for (entry in zipfile.entries())  {
				if (!entry.getName().endsWith(".fzp")) continue
					
				log.trace("Extracting: " + entry.getName())
				def reader = new InputStreamReader(zipfile.getInputStream(entry))
				def doc = new XmlParser().parse(reader)
				if (!doc) continue
				
				if (doc.name() != "module") continue
				if (!doc.title.text().contains("Custom PCB")) continue
				
				def descr = doc.description.text()
				//log.trace("descr " + descr)
				if (!descr) continue
				
				def floatMatch = /([0-9]*\.?[0-9]+)/
				def matcher = descr =~ ".+- " + floatMatch + "x" + floatMatch + "mm" + ".+"
				//log.trace(matcher[0])
				
				if (matcher[0].size() != 3) continue
				
				order1.width = matcher[0][1].toDouble()
				order1.height = matcher[0][2].toDouble()
				//log.trace("module id " + doc['@moduleId'])
				moduleIds.add(doc['@moduleId'])
			}
			return moduleIds
		} 
		catch(Exception e) 
		{
			 e.printStackTrace()
		}
		return []
	}
	
	def findBoard(file, order1, doc, customIDs) {
		def count = 0
		for (instance in doc.instances[0].children()) {
			if (instance.name() == "instance") {
				def moduleID = instance['@moduleIdRef']
				switch (moduleID) {
					case 'RectanglePCBModuleID':
						count++
						for (property in instance.children()) {
							if (property.name() == "property") {
								if (property['@name'] == "width") {
									order1.width = property['@value'].toDouble()
								}
								if (property['@name'] == "height") {
									order1.height = property['@value'].toDouble()
								}
							}
						}
						break
					case '423120090505':
						count++
						order1.width = 69.215				//width="2.725in" 
						order1.height = 53.37556			//height="2.1014in" 
						break
					default:
						if (customIDs.contains(moduleID)) {
							// order width and height already set
							count++
						}
						break
				}
			}
		}

		if (count == 1) {
			if (order1.width == 0 || order1.height == 0) {
				closeErr("File '${file.originalFilename}': sketch has improper format (3)")
				return -1
			}
			
			order1.totalPrice = order1.pricePerBoard = order1.width * order1.height * servletContext.getAttribute("pricePerSqMm")
		}
		
		return count
	}

	def unzipFz(filename, prefix) {
		try 
		{
			def BUFFERSIZE = 2048
			def buf = new byte[BUFFERSIZE]
			def zipfile = new ZipFile(filename)
			for (entry in zipfile.entries())  {
				if (entry.getName().endsWith(".fz")) {
					log.trace("Extracting: " + entry.getName())
					def is = new BufferedInputStream(zipfile.getInputStream(entry))
					def fz = "${prefix}_${entry.getName()}"
					def fos = new FileOutputStream(fz)
					def dest = new BufferedOutputStream(fos, BUFFERSIZE)
					def count
					while ((count = is.read(buf, 0, BUFFERSIZE))  != -1) {
						dest.write(buf, 0, count)
					}
					dest.flush()
					dest.close()
					is.close()
					return fz
				}
			}
		} 
		catch(Exception e) 
		{
			 e.printStackTrace()
		}
		return ""
	}

}

class QuantityCommand implements Serializable {
	String destination
	Integer quantity
	Boolean qualityCheck
	String email
	String confirmEmail
	
	static constraints = {
		quantity(min:1, max:100)
		
	}
}




