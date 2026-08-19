local STATE_PATH = "C:\\Users\\mto1\\Documents\\NES_DEV\\state_separate_channels.bin"
local RAM_BASE   = 0x0700
local STATE_SIZE = 58

local MEM_TYPE = emu.memType.nesDebug
local lastSeq = -1
local frameCounter = 0

local function pushStateToRam()
    frameCounter = (frameCounter + 1) % 2
    if frameCounter ~= 0 then
        return
    end

    local file = io.open(STATE_PATH, "rb")
    if not file then
        return
    end

    local data = file:read(STATE_SIZE)
    file:close()

    if not data or #data < STATE_SIZE then
        return
    end

    local bytes = { string.byte(data, 1, #data) }
    local seq = bytes[1]

    if seq == lastSeq then
        return
    end

    lastSeq = seq

    for i = 1, STATE_SIZE do
        emu.write(RAM_BASE + (i - 1), bytes[i], MEM_TYPE)
    end
end

emu.addEventCallback(pushStateToRam, emu.eventType.endFrame)
