function onLogin(cid)
	registerCreatureEvent(cid, "PlayerDeath")
	dofile("./config.lua")
	if sqlType == "mysql" then
		env = assert(luasql.mysql())
		con = assert(env:connect(mysqlDatabase, mysqlUser, mysqlPass, mysqlHost, mysqlPort))
	else -- sqlite
		env = assert(luasql.sqlite3())
		con = assert(env:connect(sqliteDatabase))
	end
	if serverStarted == FALSE then
		cur = assert(con:execute("DELETE FROM `players_online`;"))
		serverStarted = TRUE
	end
	assert(con:execute("INSERT INTO `players_online` VALUES ("..getPlayerGUID(cid)..");"))
	con:close()
	env:close()
	return TRUE
end
