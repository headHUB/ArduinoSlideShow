#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

#define BACKLIGHT 7

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
#define SD_CS    4  // Chip select line for SD card
#define TFT_CS  10  // Chip select line for TFT display
#define TFT_DC   9  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

File slidesFolder;
File dataFile;
uint16_t fileCount = 0;

const uint8_t RECORD_LEN = 12;
const uint16_t SLIDE_TIME_MILLIS = 10000;

char* const DATA_FILE_NAME = "/slides/slides.dat";

void setup() {
  tft.initR(INITR_REDTAB);   // initialize a ST7735R chip, red tab

  tft.setRotation(1);
  pinMode(BACKLIGHT, OUTPUT); // backlight
  digitalWrite(BACKLIGHT, HIGH);

  tft.fillScreen(ST7735_BLACK);

  tft.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    tft.println("failed!");
    return;
  }
  tft.println("OK!");
  
  slidesFolder = SD.open("/slides");
  
  if (SD.exists(DATA_FILE_NAME)) {
    dataFile = SD.open(DATA_FILE_NAME);
    dataFile.readBytes((char*)&fileCount, 2);
    tft.print(fileCount);
    tft.println(" slides");
  } else {
    tft.print("Scanning /slides ");
    const uint8_t *fileCountBuf = (uint8_t*)&fileCount;
    dataFile = SD.open(DATA_FILE_NAME, FILE_WRITE);
    dataFile.seek(0);
    dataFile.write(fileCountBuf, 2);
    
    while (File file = slidesFolder.openNextFile()) {
      // make sure it's a .raw file
      if (!strstr(file.name(), ".RAW")) {
        continue;
      }
      ++fileCount;
      // add to data file
      dataFile.write((uint8_t*)file.name(), 12);
      if (fileCount % 32 == 0) {
        tft.print('.');
      }
      file.close();
    }
    dataFile.seek(0);
    dataFile.write(fileCountBuf, 2);
    dataFile.flush();
    
    tft.print("Found ");
    tft.print(fileCount);
    tft.println(" slides");
    delay(3000);
  }
  
  randomSeed(analogRead(0));
}

void loop() {
//  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  
  long start = millis();
  long end = start + SLIDE_TIME_MILLIS;  
  char *filename = "/slides/abcdefgh.raw";
  nextFilename(filename);

//  int fileIndex = nextFileIndex();
//  tft.print("Index: ");
//  tft.println(fileIndex);
//  dataFile.seek(2+fileIndex*RECORD_LEN);
//  dataFile.readBytes(filename+8, 12);
//  tft.println(filename);  
  
//  delay(2000);

  if (SD.exists(filename)) {
    drawRaw(filename, 160, 128);
    while (millis() < end);
  }  else {
    tft.fillScreen(ST7735_BLACK);
    tft.println("File does not exist!");
    delay(2000);
  }
}

void nextFilename(char* filename) {
  int fileIndex = nextFileIndex();
  dataFile.seek(2+fileIndex*RECORD_LEN);
  dataFile.readBytes(filename+8, 12);
}

int nextFileIndex() {
  return random(fileCount);
}

void drawRaw(char *filename, uint8_t w, uint8_t h) {

  File     file;
  uint8_t  row, col;
  uint8_t  buf[30];
  uint8_t  r, g, b;

//  tft.println();
//  tft.print("Loading image '");
//  tft.print(filename);
//  tft.println('\'');

  // Open requested file on SD card
  if ((file = SD.open(filename)) == NULL) {
    tft.println("File not found:");
    tft.println(filename);
    return;
  }

  tft.setAddrWindow(0, 0, w-1, h-1);
  
  for (uint8_t x = 0; x < w; ++x) {
    for (uint8_t y = 0; y < h; ++y) {
      file.read(buf, 3);
      r = buf[0];
      g = buf[1];
      b = buf[2];
      tft.pushColor(tft.Color565(r,g,b));
    }
  }

  file.close();
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       tft.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       tft.print('\t');
     }
     tft.print(entry.name());
     if (entry.isDirectory()) {
       tft.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       tft.print("\t\t");
       tft.println(entry.size(), DEC);
     }
   }
}

