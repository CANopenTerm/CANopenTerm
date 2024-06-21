--[[

Title:   Utility functions
Author:  Michael Fitzmayer
License: Public domain

--]]

local function read_word(file)
    local bytes = file:read(2)
    if not bytes or #bytes < 2 then
        error("Failed to read 2 bytes for WORD")
    end
    local b1, b2 = string.byte(bytes, 1, 2)
    return b1 + b2 * 256
end

local function read_long(file)
    local bytes = file:read(4)
    if not bytes or #bytes < 4 then
        error("Failed to read 4 bytes for LONG")
    end
    local b1, b2, b3, b4 = string.byte(bytes, 1, 4)
    return b1 + b2 * 256 + b3 * 65536 + b4 * 16777216
end

local function read_ulong(file)
    local bytes = file:read(4)
    if not bytes or #bytes < 4 then
        error("Failed to read 4 bytes for ULONG")
    end
    local b1, b2, b3, b4 = string.byte(bytes, 1, 4)
    return b1 + b2 * 256 + b3 * 65536 + b4 * 16777216
end

local function read_byte(file)
    local byte = file:read(1)
    if not byte or #byte < 1 then
        error("Failed to read 1 byte")
    end
    return string.byte(byte)
end

local function left_shift(a, shift)
    return a * 2^shift
end

local function is_bit_set(number, position)
    return bitwise_and(number, left_shift(1, position)) ~= 0
end

local function bitwise_and(a, b)
    local result = 0
    local bit_val = 1
    while a > 0 and b > 0 do
        local ra = a % 2
        local rb = b % 2
        if ra == 1 and rb == 1 then
            result = result + bit_val
        end
        bit_val = bit_val * 2
        a = (a - ra) / 2
        b = (b - rb) / 2
    end
    return result
end

return {
  read_word   = read_word,
  read_long   = read_long,
  read_ulong  = read_ulong,
  read_byte   = read_byte,
  left_shift  = left_shift,
  is_bit_set  = is_bit_set,
  bitwise_and = bitwise_and
}
