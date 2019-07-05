#include "CDC.h"

#ifdef SERIAL_CDC
USBSerial arduino::internal::_serial(false, BOARD_VENDORID, BOARD_PRODUCTID);
#endif