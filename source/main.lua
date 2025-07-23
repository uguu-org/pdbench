-- Stub to be used when running inside simulator.
--
-- For an extra ~6k in package size, we get a friendlier error message.

import "CoreLibs/graphics"

local text <const> = "\nThis app only runs on the device.\n\nPlease see sideload instructions:\nhttps://help.play.date/games/sideloading/\n"

-- Log help text to console.  This makes it easier to copy&paste.
print(text)

function playdate.update()
	playdate.graphics.clear()
	playdate.graphics.drawText(text, 20, 20)

	-- Stop updating after the first frame.
	playdate.update = function() end
end
