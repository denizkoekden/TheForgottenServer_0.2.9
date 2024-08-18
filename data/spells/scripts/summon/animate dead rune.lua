function doTargetCorpse(cid, pos)
	local position = pos
	position.stackpos = 255
	corpse = getThingfromPos(position)
	if corpse.uid > 0 and isCreature(corpse.uid) == FALSE and isInArray(CORPSES, corpse.itemid) == TRUE then
		doRemoveItem(corpse.uid, 1)
		local creature = doSummonCreature("Skeleton", pos)
		doConvinceCreature(cid, creature)
		doSendMagicEffect(pos, CONST_ME_MAGIC_BLUE)
		return LUA_NO_ERROR
	end
	doSendMagicEffect(getPlayerPosition(cid), CONST_ME_POFF)
	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	return LUA_ERROR
end

function onCastSpell(cid, var)
	local pos = variantToPosition(var)
	if pos.x ~= 0 and pos.y ~= 0 and pos.z ~= 0 and pos.stackpos ~= 0 then
		return doTargetCorpse(cid, pos)
	end
	doSendMagicEffect(getPlayerPosition(cid), CONST_ME_POFF)
	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	return LUA_ERROR
end