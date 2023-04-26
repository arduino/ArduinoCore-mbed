#ifndef MCUboot_h_
#define MCUboot_h_

class MCUboot
{

public:
  static void confirmSketch(void);
  static void applyUpdate(bool permanent);
  static void bootDebug(bool enable);

};

#endif // MCUboot_h_
