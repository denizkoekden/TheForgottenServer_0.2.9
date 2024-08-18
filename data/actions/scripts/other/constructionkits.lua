local constructionKits = {[3909] = 1614, [3901] = 1615, [3902] = 1616, [3903] = 1619, [3904] = 1652, [3905] = 1658, [3906] = 1666, [3907] = 1670, [3908] = 1674, [3909] = 1716, [3910] = 1718, [3916] = 1724, [3911] = 1728, [3912] = 1732, [6373] = 1736, [3917] = 2117, [3913] = 2080, [3914] = 2084, [3922] = 2093, [3923] = 2094, [3924] = 2098, [5088] = 2099, [3925] = 2101, [3928] = 2104, [3929] = 2105, [3918] = 2582, [3927] = 2586, [3919] = 3806, [3920] = 3808, [3921] = 3810, [3930] = 3813, [3935] = 3817,	[3926] = 3826, [3932] = 3828, [3933] = 3830, [3934] = 3832, [3936] = 6371}
function onUse(cid, item, fromPosition, itemEx, toPosition)
	if fromPosition.x == CONTAINER_POSITION then
		doPlayerSendCancel(cid, "Put the construction kit on the ground first.")
	elseif constructionKits[item.itemid] ~= nil then
		doTransformItem(item.uid, constructionKits[item.itemid])
		doSendMagicEffect(fromPosition, CONST_ME_POFF)
	else
		return FALSE
	end
	return TRUE
end