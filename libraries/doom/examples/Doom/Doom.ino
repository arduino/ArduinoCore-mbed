/*
    Arduino wrapper for DoomGeneric
    Mouse and keyboard controls are not implemented at the moment.

    To use the internal QSPI flash as storage, run QSPIFormat
    sketch once to create the partitions, AccessFlashAsUSBDisk to expose the QSPI flash
    as a USB disk, copy DOOM1.WAD in the biggest partition, flash this sketch and you are ready to go :)
*/

#include "QSPIFBlockDevice.h"
#include "FATFileSystem.h"
#include "MBRBlockDevice.h"
#include "doomgeneric.h"

QSPIFBlockDevice block_device;
// Comment previous line and uncomment these two if you want to store DOOM.WAD in an external SD card (FAT32 formatted)
// #include "SDMMCBlockDevice.h"
// SDMMCBlockDevice block_device;

mbed::MBRBlockDevice fs_data(&block_device, 2);
static mbed::FATFileSystem fs("fs");

extern "C" int main_wrapper(int argc, char **argv);
char*argv[] = {"/fs/doom", "-iwad", "/fs/DOOM1.WAD"};

void setup() {
  int err =  fs.mount(&fs_data);
  if (err) {
    printf("No filesystem found, please run AccessFlashAsUSBDisk sketch and copy DOOM1.WAD in the big partition");
    pinMode(LEDB, OUTPUT);
    while (1) {
      digitalWrite(LEDB, LOW);
      delay(100);
      digitalWrite(LEDB, HIGH);
      delay(100);
    }
  }
  DIR *dir;
  struct dirent *ent;
  printf("try to open dir\n");
  if ((dir = opendir("/fs")) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      printf ("%s\n", ent->d_name);
    }
    closedir (dir);
  } else {
    /* could not open directory */
    printf ("error\n");
  }
  main_wrapper(3, argv);
}

void loop() {
  // put your main code here, to run repeatedly:

}
