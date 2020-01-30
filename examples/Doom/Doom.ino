#include "SDMMCBlockDevice.h"
#include "FATFileSystem.h"
#include "doomgeneric.h"

SDMMCBlockDevice block_device;

mbed::FATFileSystem fs("fs");

extern "C" int main_wrapper(int argc, char **argv);
char*argv[] = {"/fs/doom", "-iwad", "/fs/DOOM1.WAD"};

void setup() {
  // put your setup code here, to run once:
  delay(2000);
  int err =  fs.mount(&block_device);
  if (err) {
    // Reformat if we can't mount the filesystem
    // this should only happen on the first boot
    printf("No filesystem found, formatting... ");
    fflush(stdout);
    err = fs.reformat(&block_device);
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
