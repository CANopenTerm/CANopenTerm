--[[ Oscilloscope demo

Author:  Michael Fitzmayer
License: Public domain

--]]

-- Initialize display.
widget_theme(1)
window_show()
window_clear()

-- Create oscilloscope buffer.
local osc_id = oscilloscope_buffer_create(0, 100)

-- Simulate data collection and display
local current_value = 50
local time_counter = 0
local demo_running = true

-- Calculate layout based on window dimensions.
function calculate_layout(width, height)
  -- Use default dimensions if window hasn't initialized yet
  if width == 0 or height == 0 then
    width = 800
    height = 600
  end

  local layout = {}
  local margin = 10
  layout.pos_x = margin
  layout.pos_y = margin
  layout.width = width - (margin * 2)
  layout.height = height - (margin * 2)
  return layout
end

while demo_running and not key_is_hit() do
  -- Simulate sensor readings with combined waveforms for variety
  local sine_component = 30 * math.sin(time_counter / 10)
  local triangle_component = 20 * (math.abs((time_counter % 40) - 20) / 20 - 1)
  local noise = math.random(-8, 8)

  current_value = 50 + sine_component + triangle_component + noise
  current_value = math.max(0, math.min(100, math.floor(current_value)))

  -- Add to buffer.
  oscilloscope_buffer_push(osc_id, current_value)

  -- Clear and render.
  window_clear()

  -- Get dynamic window dimensions and calculate layout.
  local width, height = window_get_resolution()
  local layout = calculate_layout(width, height)

  -- Draw oscilloscope using dynamic layout.
  widget_oscilloscope(layout.pos_x, layout.pos_y, layout.width, layout.height, osc_id, current_value, "Temperature")

  -- Update display.
  window_update(true)

  time_counter = time_counter + 1
  delay_ms(50)

  -- Limit demo duration if needed (optional safety exit).
  if time_counter > 2000 then
    demo_running = false
  end
end

-- Cleanup.
oscilloscope_buffer_destroy(osc_id)
window_hide()
