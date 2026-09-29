--[[ Dual Oscilloscope Demo

Author:  Michael Fitzmayer
License: Public domain

--]]

-- Initialize display.
widget_theme(2)
window_show()
window_clear()
window_resize(1000, 600)

-- Create two oscilloscope buffers for comparison.
local osc_id1 = oscilloscope_buffer_create(0, 100, 2048)
local osc_id2 = oscilloscope_buffer_create(0, 100, 2048)

-- Simulate data collection and display
local current_value1 = 50
local current_value2 = 50
local time_counter = 0
local demo_running = true

-- Calculate layout based on window dimensions.
function calculate_layout(width, height)
  -- Use default dimensions if window hasn't initialized yet
  if width == 0 or height == 0 then
    width = 1000
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
  -- Generate first signal (oscillating pattern with 10ms period)
  local sine_component = 30 * math.sin(time_counter / 10)
  local noise1 = math.random(-5, 5)
  current_value1 = 50 + sine_component + noise1
  current_value1 = math.max(0, math.min(100, math.floor(current_value1)))

  -- Generate second signal (slower oscillating pattern with 50ms period)
  local triangle_component = 25 * (math.abs((time_counter % 50) - 25) / 25 - 1)
  local noise2 = math.random(-5, 5)
  current_value2 = 50 + triangle_component + noise2
  current_value2 = math.max(0, math.min(100, math.floor(current_value2)))

  -- Add to buffers.
  oscilloscope_buffer_push(osc_id1, current_value1)
  oscilloscope_buffer_push(osc_id2, current_value2)

  -- Clear and render.
  window_clear()

  -- Get dynamic window dimensions and calculate layout.
  local width, height = window_get_resolution()
  local layout = calculate_layout(width, height)

  -- Draw dual oscilloscope using dynamic layout for direct comparison.
  widget_oscilloscope_2ch(layout.pos_x, layout.pos_y, layout.width, layout.height,
                           osc_id1, current_value1, "Signal 1 (10ms period)",
                           osc_id2, current_value2, "Signal 2 (50ms period)", 500)

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
oscilloscope_buffer_destroy(osc_id1)
oscilloscope_buffer_destroy(osc_id2)
window_hide()
