function onDeath(cid, corpse, killer)
	doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "You are dead.")
	dofile("./config.lua")
	if deathListEnabled == "yes" then
		if sqlType == "mysql" then
			env = assert(luasql.mysql())
			con = assert(env:connect(mysqlDatabase, mysqlUser, mysqlPass, mysqlHost, mysqlPort))
		else -- sqlite
			env = assert(luasql.sqlite3())
			con = assert(env:connect(sqliteDatabase))
		end
		local byPlayer = FALSE
		local killerName = escapeString(getCreatureName(killer))
		if isPlayer(killer) == TRUE then
			byPlayer = TRUE
		elseif isPlayer(killer) ~= FALSE then
			killerName = "field item"
		end
		query = assert(con:execute("INSERT INTO `player_deaths` (`player_id`, `time`, `level`, `killed_by`, `is_player`) VALUES (" .. getPlayerGUIDByName(getCreatureName(cid)) .. ", " .. os.time() .. ", " .. getPlayerLevel(cid) .. ", '" .. killerName .. "', " .. byPlayer .. ");"))
		local cursor = assert(con:execute("SELECT `player_id` FROM `player_deaths` WHERE `player_id` = " .. getPlayerGUIDByName(getCreatureName(cid)) .. ";"))
		local deathRecords = numRows(cursor)
		while deathRecords > maxDeathRecords do
			delete = assert(con:execute("DELETE FROM `player_deaths` WHERE `player_id` = " .. getPlayerGUIDByName(getCreatureName(cid)) .. " ORDER BY `time` LIMIT 1;"))
			deathRecords = deathRecords - 1
		end
		con:close()
		env:close()
	end
end