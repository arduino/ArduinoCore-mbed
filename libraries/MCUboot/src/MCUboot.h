#ifndef MCUboot_h_
#define MCUboot_h_

class MCUboot
{

public:
  static void confirmSketch(void);
  static void applyUpdate(int permanent);
  static void bootDebug(int enable);

};

#endif // MCUboot_h_
