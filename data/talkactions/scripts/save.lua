local savingEvent = 0
local savingDelay = 0

function onSay(cid, words, param)
	if getPlayerAccess(cid) ~= 0 then
		if param == "" then
			savePlayers()
		elseif isNumber(param) == TRUE then
			stopEvent(savingEvent)
			savingDelay = param * 1000 * 60
			save()
		end
	end
end

function save()
	savingEvent = addEvent(save, savingDelay, {})
	savePlayers()
end