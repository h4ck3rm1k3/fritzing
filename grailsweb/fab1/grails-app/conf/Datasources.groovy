datasources = {

	datasource(name: 'user') {
		domainClasses([com.g2one.fab1.User])
		readOnly(true)
		
		//driverClassName('com.mysql.jdbc.Driver')
		//url('jdbc:mysql://localhost/django')
		//username('root')
		//password('\$r00t\$')
		//dbCreate('create-drop')
		//logSql(true)
		
		//dbCreate("update") // one of 'create', 'create-drop','update'
		url ("jdbc:sqlite:/C:/Users/jonathan/django_stuff/fritzing/development.db")
		dialect("dialect.SQLiteDialect")    // MySQLDialect  SQLiteDialect
		driverClassName("org.sqlite.JDBC")	
		username('admin')
		password('admin')
		logSql(true)
		
		hibernate {
			cache {
				use_second_level_cache(false)
				use_query_cache(false)
			}
		}
	}

}
