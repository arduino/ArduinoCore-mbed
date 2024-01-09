# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class` [`Arduino_H7_Video`](#class-arduino_h7_video) | The main class for managing the video controller and the display.

# class [`Arduino_H7_Video`](#class-arduino_h7_video)
The main class for managing the video controller and the display.

## Summary

| Members                                                     | Descriptions                                |
|-------------------------------------------------------------|---------------------------------------------|
| `public ` [`Arduino_H7_Video`](#public-arduino_h7_videoint-width-int-height-h7displayshield-shield) | Construct a new Arduino_H7_Video object with the specified width, height, and display shield. |
| `public int` [`begin`](#public-int-begin) | Initialize the video controller and display. |
| `public void` [`end`](#public-void-end) | De-initialize the video controller and display. |
| `public int` [`width`](#public-int-width) | Get the width of the display. |
| `public int` [`height`](#public-int-height) | Get the height of the display. |
| `public bool` [`isRotated`](#public-bool-isrotated) | Check if the display is rotated. |
| `public void` [`clear`](#public-void-clear) | Clear the display. |
| `public void` [`beginDraw`](#public-void-begindraw) | Begin drawing operations on the display. |
| `public void` [`endDraw`](#public-void-enddraw) | End drawing operations on the display. |
| `public void` [`set`](#public-void-setint-x-int-y-uint8_t-r-uint8_t-g-uint8_t-b) | Set the color of the pixel at the specified coordinates. |

> *Note: For all drawing functions, refer to the documentation of the [`ArduinoGraphics`](https://reference.arduino.cc/reference/en/libraries/arduinographics/) library.*

## Members

### `public ` [`Arduino_H7_Video`](#)`(int width, int height, H7DisplayShield &shield)`

Construct a new Arduino_H7_Video object with the specified width, height, and display shield.

#### Parameters
- `width`: The width of the display.
- `height`: The height of the display.
- `shield`: The display shield used.
    - *GigaDisplayShield*: Giga Display Shield
    - *USBCVideo*: Display attach to the USB-C port

---

### `public int` [`begin`](#)`()`

Initialize the video controller and display.

#### Returns
`int`: 0 if initialization is successful, otherwise an error code.

---

### `public void` [`end`](#)`()`

De-initialize the video controller and display.

---

### `public int` [`width`](#)`()`

Get the width of the display.

#### Returns
`int`: The width of the display.

---

### `public int` [`height`](#)`()`

Get the height of the display.

#### Returns
`int`: The height of the display.

---

### `public bool` [`isRotated`](#)`()`

Check if the display is rotated.

#### Returns
`bool`: True if the display is rotated, false otherwise.

---

### `public void` [`clear`](#)`()`

Clear the display.

---

### `public void` [`beginDraw`](#)`()`

Begin drawing operations on the display.

---

### `public void` [`endDraw`](#)`()`

End drawing operations on the display.

---

### `public void` [`set`](#)`(int x, int y, uint8_t r, uint8_t g, uint8_t b)`

Set the color of the pixel at the specified coordinates.

#### Parameters
- `x`: The x-coordinate of the pixel.
- `y`: The y-coordinate of the pixel.
- `r`: The red component of the color.
- `g`: The green component of the color.
- `b`: The blue component of the color.

---

> *Note: For detailed information on drawing functions, please refer to the documentation of the [`ArduinoGraphics`](https://reference.arduino.cc/reference/en/libraries/arduinographics/) library.*


