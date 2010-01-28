datasources = {

	datasource(name: 'user') {
		domainClasses([com.g2one.fab1.User])
		readOnly(true)
		driverClassName('com.mysql.jdbc.Driver')
		url('jdbc:mysql://localhost/django')
		username('')
		password('')
		dbCreate('create-drop')
		logSql(true)
		hibernate {
			cache {
				use_second_level_cache(false)
				use_query_cache(false)
			}
		}
	}

}
