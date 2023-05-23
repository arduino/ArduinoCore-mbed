function convertRGB565ToRGB888(pixelValue) {
    // RGB565
    let r = (pixelValue >> (6 + 5)) & 0x1F;
    let g = (pixelValue >> 5) & 0x3F;
    let b = pixelValue & 0x1F;
    // RGB888 - amplify
    r <<= 3;
    g <<= 2;
    b <<= 3;
    return [r, g, b];
}
