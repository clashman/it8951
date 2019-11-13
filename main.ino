void setup(void)
{
    Serial.begin(115200);
    Serial.println("\n\n\n");

    Serial.printf("MISO %i\n", MISO);
    Serial.printf("MOSI %i\n", MOSI);
    Serial.printf("SCK %i\n", SCK);
    Serial.printf("CS %i\n", CS);
    Serial.printf("RESET %i\n", RESET);
    Serial.printf("HRDY %i\n", HRDY);

    Serial.println("Display init ...");
    if(!display_begin()) {
        Serial.println("Display init failed");
        exit(1);
    };
    Serial.println("Display init ok");
}

extern unsigned char pic[];
extern unsigned int pic_width;
extern unsigned int pic_height;

void loop() {
    int x = random(0, gstI80DevInfo.usPanelW - pic_width);
    int y = random(0, gstI80DevInfo.usPanelH - pic_height);
    x -= x%4; // somehow the image is distorted when not aligned like this

    display_buffer(pic, x, y, pic_width, pic_height);
}
