/* file: REG_DRV1.h
 * description: Mississippi-2 (ANX7625) register read/write function prototypes (software-emulated I2C)
*/

#ifndef __REG_DRV1_H__
#define __REG_DRV1_H__

////////////////////////////////////////////////////////////////////////////////
// register operations
// read operations
char ReadReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char data *pData);
char ReadWordReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned int data *pData);
char ReadBlockReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char idata *pBuf);
// write operations
char WriteReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal);
char WriteBlockReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char idata *pBuf);

#endif  /* __REG_DRV1_H__ */

