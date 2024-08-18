function onLogout(cid)
	dofile("./config.lua")
	if sqlType == "mysql" then
		env = assert(luasql.mysql())
		con = assert(env:connect(mysqlDatabase, mysqlUser, mysqlPass, mysqlHost, mysqlPort))
		assert(con:execute("DELETE FROM `players_online` WHERE id = "..getPlayerGUID(cid).." LIMIT 1;"))
	else -- sqlite
		env = assert(luasql.sqlite3())
		con = assert(env:connect(sqliteDatabase))
		assert(con:execute("DELETE FROM `players_online` WHERE `rowid` = (SELECT `rowid` FROM `players_online` WHERE id = "..getPlayerGUID(cid).." LIMIT 1);"))
	end
	con:close()
	env:close()
	return TRUE
end
