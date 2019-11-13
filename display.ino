uint8_t display_begin() {
    pinMode(MISO, INPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SCK, OUTPUT);
    pinMode(CS, OUTPUT);
    pinMode(RESET, OUTPUT);
    pinMode(HRDY, INPUT);

    uint8_t err = IT8951_Init();

    return err == 0;
}

void display_buffer(uint8_t* addr, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    gpFrameBuf = addr;
    Serial.println("Sending image");
    IT8951_BMP_Example(x, y, w, h);
    Serial.println("Displaying image");
    IT8951DisplayArea(x, y, w, h, 2);
    Serial.println("Waiting for display ...");
    LCDWaitForReady();
    Serial.println("done");
}
