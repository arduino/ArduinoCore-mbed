/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename : 

Project  : ANX7625 

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil
 
Description:

Revision History:

******************************************************************************/

#ifndef __SI_TOOLCHAIN_H__
#define __SI_TOOLCHAIN_H__

#include "stdint.h"
#include "stdbool.h"

/**************************************************************************//**
 *
 * @addtogroup toolchain_group Toolchain Abstraction
 *
 * @brief Macros for toolchain abstraction.
 *
 * # Introduction #
 *
 * This header file contains macros that are used to provide an abstraction
 * for toolchain use in source code.  The 8051 compiler requires C-language
 * extensions in order to fully use features of the 8051 architecture.  All
 * compilers for 8051 implement a set of extensions but use different names
 * and ways of implementing those extensions.  This header file provides
 * macros that are defined for each supported toolchain and can be used in
 * the source code.  This allows the source code to use 8051 extensions and
 * remain independent of which toolchain is used for compilation.
 *
 * ## Variable and Pointer Declarations ##
 *
 * It is often useful to specify the memory area (or segment) of a variable,
 * pointer, or pointer target.  For example, you may wish to place all
 * variables in XDATA by default, but for variables used in time-sensitive
 * code you use DATA for efficient access.  In this case you declare the
 * XDATA variable in the normal C way, but declare the variables to be located
 * in the DATA segment using @ref SI_SEGMENT_VARIABLE.
 *
 * Pointers are more complicated because there are two memory spaces
 * associated with a pointer, the pointer target, and the pointer variable
 * itself.  When using default memory segment for the pointer location and
 * target, then no special macro is needed.  But if you wish to specify the
 * pointer variable location, or target memory segment, then you can use one
 * of the following macros to do this in a toolchain-independent way.
 *
 * |Pointer segment|Target segment|Macro                                   |
 * |---------------|--------------|----------------------------------------|
 * |default        |generic       |None                                    |
 * |default        |specific      |@ref SI_VARIABLE_SEGMENT_POINTER        |
 * |specific       |generic       |@ref SI_SEGMENT_POINTER                 |
 * |specific       |specific      |@ref SI_SEGMENT_VARIABLE_SEGMENT_POINTER|
 *
 * ## Prior Toolchain Abstraction Header File ##
 *
 * This file supercedes an earlier header file named `compiler_defs.h`.  We
 * are deprecating the use of compiler_defs.h, however it will remain for
 * backwards compatibility.  This file was created to normalize macro names,
 * remove unused macros, and to provide documentation.
 *
 * ## Supported Toolchains ##
 *
 * - Keil/ARM C51
 *
 * @{
 *
 *****************************************************************************/

// Make sure there is a NULL defined if the toolchain does not provide it.
#ifndef NULL
#define NULL ((void *)0)
#endif

// -------------------------------
// Keil/ARM C51
//
#if defined(__C51__)

/// Used with pointers, declares a generic pointer.  Generic pointers
/// work with any memory space but are inefficient.
#define SI_SEG_GENERIC

/// Declares a variable to be located in 8051 DATA space.
#define SI_SEG_DATA data

/// Declares a variable to be located in 8051 IDATA space.
#define SI_SEG_IDATA idata

/// Declares a variable to be located in 8051 XDATA space.
#define SI_SEG_XDATA xdata

/// Declares a variable to be located in 8051 PDATA space.
#define SI_SEG_PDATA pdata

/// Declares a variable to be located in 8051 BDATA (bit-addressable) space.
#define SI_SEG_BDATA bdata

/// Declares a variable to be located in 8051 CODE space.
#define SI_SEG_CODE code

/**************************************************************************//**
 * Declares a bit variable in a bit-addressable SFR or memory space.
 *
 * @param name The name of the bit variable.
 * @param address The address of the byte containing the bit.
 * @param bitnum The bit number (0-7) within the byte.
 *
 * This cannot be used to make any arbitrary SFR or variable into
 * a bit variable.  The underlying memory must support bit-addressability.
 *****************************************************************************/
#define SI_SBIT(name, address, bitnum) sbit name = address^bitnum

/**************************************************************************//**
 * Declares an 8-bit special function register (SFR) variable.
 *
 * @param name The name of the SFR variable.
 * @param address The address of the SFR.
 *
 * This creates a C variable (8-bit) that maps to a physical special function
 * register of the 8051.  This cannot be used to make any arbitrary memory
 * location into an SFR.  The _address_ must map to a real SFR in the memory
 * map.
 *****************************************************************************/
#define SI_SFR(name, address) sfr name = address

/**************************************************************************//**
 * Declares a 16-bit special function register (SFR) variable.
 *
 * @param name The name of the SFR variable.
 * @param address The address of the 16-bit SFR.
 *
 * This creates a C variable (16-bit) that maps to a physical special function
 * register of the 8051.  This cannot be used to make any arbitrary memory
 * location into an SFR.  The _address_ must map to a real 16-bit SFR in the
 * memory map.
 *****************************************************************************/
#define SI_SFR16(name, address) sfr16 name = address

#ifndef __SLS_IDE__
/**************************************************************************//**
 * Define an interrupt handler function for an interrupt vector.
 *
 * @param name The name of the interrupt handler function.
 * @param vector The interrupt vector number.
 *
 * This macro defines a function to be an interrupt handler.  The _vector_
 * parameter is the 8051 interrupt vector number, not the address.  This
 * will cause the compiler to treat the function as the interrupt handler
 * and generate the appropriate prolog/epilog code.
 *
 * @note This macro is used to define the function implementation.  To declare
 * the interrupt function prototype, use @ref SI_INTERRUPT_PROTO.
 *****************************************************************************/
#define SI_INTERRUPT(name, vector) void name (void) interrupt vector

/**************************************************************************//**
 * Define an interrupt handler function using a specific register bank.
 *
 * @param name The name of the interrupt handler function.
 * @param vector The interrupt vector number.
 * @param regnum The register bank number (0-3).
 *
 * This macro defines a function to be an interrupt handler, using a specific
 * register bank for the interrupt code.  The _vector_ parameter is the 8051
 * interrupt vector number, not the address.  The _regnum_ parameter is the
 * register bank number (0-3) that will be used as general purpose registers
 * for the instructions in the compiled code.  Using dedicated register banks
 * for interrupt handlers allows the prolog code to just switch banks instead
 * of saving and restoring all the general purpose registers.  This can make
 * interrupt entry/exit faster but requires dedicating a register bank for
 * the interrupt handler.
 *
 * @note This macro is used to define the function implementation.  To declare
 * the interrupt function prototype, use @ref SI_INTERRUPT_PROTO_USING.
 *****************************************************************************/
#define SI_INTERRUPT_USING(name, vector, regnum)                             \
             void name (void) interrupt vector using regnum

/**************************************************************************//**
 * Declare an interrupt handler prototype for an interrupt vector.
 *
 * @param name The name of the interrupt handler function.
 * @param vector The interrupt vector number.
 *
 * This macro declares a function prototype for an interrupt handler.  The
 * _vector_ parameter is the 8051 interrupt vector number, not the address.
 * Declaring the function prototype this way will cause the compiler to
 * recognize that the function is an interrupt handler and not a normal C
 * function.
 *
 * @note This macro is used to declare a prototype for the interrupt function.
 * To define the interrupt function implementation, use @ref SI_INTERRUPT.
 *****************************************************************************/
#define SI_INTERRUPT_PROTO(name, vector) void name (void)

/**************************************************************************//**
 * Declare an interrupt handler prototype using a specific register bank.
 *
 * @param name The name of the interrupt handler function.
 * @param vector The interrupt vector number.
 * @param regnum The register bank number (0-3).
 *
 * This macro declares a function prototype for an interrupt handler, for a
 * function that uses a specific register bank for the interrupt code.  The
 * _vector_ parameter is the 8051 interrupt vector number, not the address.
 * The _regnum_ parameter is the register bank number (0-3) that will be used
 * as general purpose registers in the function.  Declaring the function
 * prototype this way will cause the compiler to recognize that the function
 * is an interrupt handler and is not a normal C function.
 *
 * @note This macro is used to declare a prototype for the interrupt function.
 * To define the interrupt function implementation,
 * use @ref SI_INTERRUPT_USING.
 *****************************************************************************/
#define SI_INTERRUPT_PROTO_USING(name, vector, regnum) void name (void)

/**************************************************************************//**
 * Define a function to use a specific register bank.
 *
 * @param name The name of the function.
 * @param return_value The data type of the function return value
 * (void, int, etc).
 * @param parameter One C function parameter (or "void") (type and name).
 * @param regnum The register bank number (0-3).
 *
 * This macro defines a function that uses a specific register bank.  The
 * _regnum_ parameter is the register bank number (0-3) that will be used as
 * general purpose registers for the instructions in the compiled function
 * code.  Using dedicated register banks for a function can reduce the amount
 * of registers saving and restoring needed on entry and exit to the
 * function.  However, this is an advanced feature and you should not use it
 * unless you fully understand how and when to use register banking.
 *
 * You must specify the _return_value_ which is the type of the function.  It
 * can be `void` or any other C type or typedef.  The _parameters_ argument
 * is the list of function parameters.  It can be `void` or else it must be
 * a parameter data type and name.  It can also be multiple parameters but
 * they must be enclosed in parentheses and separated by commas.
 *
 * __Example__
 *
 * ~~~~~~~~.c
 * // The following is used to implement a function with the following
 * // signature, and that uses register bank 3 ...
 * uint16_t myFunction(uint8_t parm1, uint8_t parm2);
 *
 * SI_FUNCTION_USING(myFunction, uint16_t, (uint8_t parm1, uint8_t parm2), 3)
 * {
 *   // Function implementation body
 * }
 * ~~~~~~~~
 *
 * @note This macro is used to define the function implementation.  To declare
 * the function prototype, use @ref SI_FUNCTION_PROTO_USING.
 *****************************************************************************/
#define SI_FUNCTION_USING(name, return_value, parameter, regnum)              \
             return_value name (parameter) using regnum

/**************************************************************************//**
 * Declare a function that uses a specific register bank.
 *
 * @param name The name of the function.
 * @param return_value The data type of the function return value
 * (void, int, etc).
 * @param parameter One C function parameter (or "void") (type and name).
 * @param regnum The register bank number (0-3).
 *
 * This macro declares a function prototype for a C function that uses a
 * specific register as its working registers.  See the documentation for
 * @ref SI_FUNCTION_USING for an explanation of the macro arguments.  This is
 * an advanced feature.
 *
 * @note This macro is used to declare a prototype for the function.  To
 * define the function implementation, use @ref SI_FUNCTION_USING.
 *****************************************************************************/
#define SI_FUNCTION_PROTO_USING(name, return_value, parameter, regnum)        \
             return_value name (parameter)

/**************************************************************************//**
 * Declare a variable to be located in a specific memory segment.
 *
 * @param name The variable name.
 * @param vartype The variable data type.
 * @param memseg The memory segment to use for the variable.
 *
 * This macro declares a variable to be located in a specific memory area
 * (or segment) of the 8051 memory space.  It is only necessary to use this
 * macro if you want to force the variable into a specific memory space instead
 * of the default memory space used by the compiler.  The segment can be
 * one of the following:
 *
 * - @ref SI_SEG_DATA
 * - @ref SI_SEG_IDATA
 * - @ref SI_SEG_BDATA
 * - @ref SI_SEG_PDATA
 * - @ref SI_SEG_XDATA
 * - @ref SI_SEG_CODE
 *
 * __Example__
 *
 * ~~~~~~~~.c
 * // The following macro can be used to create a variable located in
 * // XDATA with the following signature:
 * uint8_t myVar;
 *
 * SI_SEGMENT_VARIABLE(myVar, uint8_t, SEG_XDATA);
 * ~~~~~~~~
 *****************************************************************************/
#define SI_SEGMENT_VARIABLE(name, vartype, memseg) vartype memseg name

/**************************************************************************//**
 * Declare a memory segment specific pointer variable.
 *
 * @param name The pointer variable name.
 * @param vartype The pointer data type.
 * @param targseg The target memory segment for the pointer.
 *
 * This macro declares a pointer that points at a specific memory area
 * (or segment).  The memory segment of the pointer variable itself is not
 * specified and the default is used.  The segment can be one of the following:
 *
 * - @ref SI_SEG_DATA
 * - @ref SI_SEG_IDATA
 * - @ref SI_SEG_BDATA
 * - @ref SI_SEG_PDATA
 * - @ref SI_SEG_XDATA
 * - @ref SI_SEG_CODE
 *
 * __Example__
 *
 * ~~~~~~~~.c
 * // The following macro can be used to create a pointer that points to
 * // a location in XDATA with the following signature:
 * uint8_t *pVar; // where pVar is pointing at XDATA
 *
 * SI_VARIABLE_SEGMENT_POINTER(pVar, uint8_t, SEG_XDATA);
 * ~~~~~~~~
 *****************************************************************************/
#define SI_VARIABLE_SEGMENT_POINTER(name, vartype, targseg)                  \
             vartype targseg * name

/**************************************************************************//**
 * Declare a memory segment specific pointer variable, in a specific segment.
 *
 * @param name The pointer variable name.
 * @param vartype The pointer data type.
 * @param targseg The target memory segment for the pointer.
 * @param memseg The memory segment to use for the pointer variable.
 *
 * This macro declares a pointer that points at a specific memory area
 * (or segment).  The pointer variable itself is also located in a specified
 * memory segment by _memseg_.  The arguments _targseg_ and _memseg_ can be
 * one of the following:
 *
 * - @ref SI_SEG_DATA
 * - @ref SI_SEG_IDATA
 * - @ref SI_SEG_BDATA
 * - @ref SI_SEG_PDATA
 * - @ref SI_SEG_XDATA
 * - @ref SI_SEG_CODE
 *
 * __Example__
 *
 * ~~~~~~~~.c
 * // The following macro can be used to create a pointer that points to
 * // a location in XDATA while the pointer itself is located in DATA, with
 * // the following signature:
 * uint8_t *pVar; // where pVar is located in DATA and is pointing at XDATA
 *
 * SI_SEGMENT_VARIABLE_SEGMENT_POINTER(pVar, uint8_t, SEG_XDATA, SEG_DATA);
 * ~~~~~~~~
 *****************************************************************************/
#define SI_SEGMENT_VARIABLE_SEGMENT_POINTER(name, vartype, targseg, memseg)  \
             vartype targseg * memseg name

/**************************************************************************//**
 * Declare a generic pointer variable that is located in a specific segment.
 *
 * @param name The pointer variable name.
 * @param vartype The pointer data type.
 * @param memseg The memory segment to use for the pointer variable.
 *
 * This macro declares a pointer that is a generic pointer.  This means it can
 * point at any kind of memory location.  However the pointer variable itself
 * is located in a specific memory segment by _memseg_, which can be one of
 * the following:
 *
 * - @ref SI_SEG_DATA
 * - @ref SI_SEG_IDATA
 * - @ref SI_SEG_BDATA
 * - @ref SI_SEG_PDATA
 * - @ref SI_SEG_XDATA
 * - @ref SI_SEG_CODE
 *
 * __Example__
 *
 * ~~~~~~~~.c
 * // The following macro can be used to create a generic pointer that
 * // is located in DATA and points at any memory type, with the
 * // following signature:
 * uint8_t *pVar; // where pVar is located in DATA and is a generic pointer
 *
 * SI_SEGMENT_POINTER(pVar, uint8_t, SEG_DATA);
 * ~~~~~~~~
 *****************************************************************************/
#define SI_SEGMENT_POINTER(name, vartype, memseg) vartype * memseg name

/**************************************************************************//**
 * Declare an uninitialized variable that is located at a specific address.
 *
 * @param name The variable name.
 * @param vartype The variable data type.
 * @param memseg The memory segment to use for the variable.
 * @param address The memory address of the variable.
 *
 * This macro allows declaring a variable that can be placed at a specific
 * location in memory.  This can only be used for variables that do not need
 * initializers.  The _address_ is the memory address within the specified
 * segment.  The memory segment, _memseg_, can be one of the following:
 *
 * - @ref SI_SEG_DATA
 * - @ref SI_SEG_IDATA
 * - @ref SI_SEG_BDATA
 * - @ref SI_SEG_PDATA
 * - @ref SI_SEG_XDATA
 * - @ref SI_SEG_CODE
 *
 * __Example__
 *
 * ~~~~~~~~.c
 * // The following declares a variable located at 0x4000 in XDATA with
 * // the following signature:
 * uint8_t myMemVar;
 *
 * SI_LOCATED_VARIABLE_NO_INIT(myMemVar, uint8_t, SEG_DATA, 0x4000);
 * ~~~~~~~~
 *****************************************************************************/
#define SI_LOCATED_VARIABLE_NO_INIT(name, vartype, memseg, address)          \
             vartype memseg name _at_ address


#else  // __SLS_IDE__ : Macros defined to remove syntax errors within Simplicity Studio
#define SI_INTERRUPT(name, vector) void name (void)
#define SI_INTERRUPT_USING(name, vector, regnum) void name (void)
#define SI_INTERRUPT_PROTO(name, vector) void name (void)
#define SI_INTERRUPT_PROTO_USING(name, vector, regnum) void name (void)

#define SI_FUNCTION_USING(name, return_value, parameter, regnum) return_value name (parameter)
#define SI_FUNCTION_PROTO_USING(name, return_value, parameter, regnum) return_value name (parameter)
// Note: 'parameter' must be either 'void' or include a variable type and name. (Example: char temp_variable)

#define SI_SEGMENT_VARIABLE(name, vartype, locsegment) vartype name
#define SI_VARIABLE_SEGMENT_POINTER(name, vartype, targsegment) vartype * name
#define SI_SEGMENT_VARIABLE_SEGMENT_POINTER(name, vartype, targsegment, locsegment) vartype * name
#define SI_SEGMENT_POINTER(name, vartype, locsegment) vartype * name
#define SI_LOCATED_VARIABLE_NO_INIT(name, vartype, locsegment, addr) vartype name
#endif  /* __SLS_IDE__ */

// The following are used for byte ordering when referring to individual
// bytes within a SI_UU32_t.  B0 is the least significant byte.
#define B0 3 ///< Least significant byte of a 4 byte word
#define B1 2 ///< Byte 1 of a 4-byte word, where byte 0 is LSB
#define B2 1 ///< Byte 2 of a 4-byte word, where byte 0 is LSB
#define B3 0 ///< Most significant byte of a 4-byte word

#define LSB 1 ///< Index to least significant bit of a 2 byte word
#define MSB 0 ///< Index to most significant bit of a 2 byte word

/// A union type to make it easier to access individual bytes of a 16-bit
/// word, and to use as signed or unsigned type.
typedef union SI_UU16
{
  uint16_t u16;   ///< The two byte value as a 16-bit unsigned integer.
  int16_t s16;    ///< The two byte value as a 16-bit signed integer.
  uint8_t u8[2];  ///< The two byte value as two unsigned 8-bit integers.
  int8_t s8[2];   ///< The two byte value as two signed 8-bit integers.
} SI_UU16_t;

/// A union type to make it easier to access individual bytes within a
/// 32-bit word, or to access it as variations of 16-bit words, or to
/// use as signed or unsigned type.
typedef union SI_UU32
{
  uint32_t u32;       ///< The 4-byte value as a 32-bit unsigned integer.
  int32_t s32;        ///< The 4-byte value as a 32-bit signed integer.
  SI_UU16_t uu16[2];  ///< The 4-byte value as two SI_UU16_t.
  uint16_t u16[2];    ///< The 4-byte value as two unsigned 16-bit integers.
  int16_t s16[2];     ///< The 4-byte value as two signed 16-bit integers.
  uint8_t u8[4];      ///< The 4-byte value as 4 unsigned 8-bit integers.
  int8_t s8[4];       ///< The 4-byte value as 4 signed 8-bit integers.
} SI_UU32_t;

// Generic pointer memory segment constants.
#define SI_GPTR                   ///< Generic pointer indeterminate type.
#define SI_GPTR_MTYPE_DATA  0x00  ///< Generic pointer for DATA segment.
#define SI_GPTR_MTYPE_IDATA 0x00  ///< Generic pointer for IDATA segment.
#define SI_GPTR_MTYPE_BDATA 0x00  ///< Generic pointer for BDATA segment.
#define SI_GPTR_MTYPE_PDATA 0xFE  ///< Generic pointer for PDATA segment.
#define SI_GPTR_MTYPE_XDATA 0x01  ///< Generic pointer for XDATA segment.
#define SI_GPTR_MTYPE_CODE  0xFF  ///< Generic pointer for CODE segment.

/// Generic pointer structure containing the type and address.
typedef struct
{
  uint8_t memtype;    ///< The type of memory of the generic pointer.
  SI_UU16_t address;  ///< The address of the generic pointer.
} GPTR_t;

/// A union type to allow access to the fields of a generic pointer.
/// A generic pointer has a field indicating the type of memory and an
/// address within the memory.
typedef union SI_GEN_PTR
{
  uint8_t u8[3];    ///< 3-byte generic pointer as 3 unsigned 8-bit integers.
  GPTR_t gptr;      ///< 3-byte generic pointer as pointer structure
} SI_GEN_PTR_t;

// Declaration of Keil intrinisc
extern void _nop_(void);
/// Macro to insert a no-operation (NOP) instruction.
#define NOP() _nop_()

// -------------------------------
// GCC for ARM Cortex-M
// Provides support for code that can be compiled for 8 or 32-bit
//
#elif defined (__GNUC__)
#if defined(__ARMEL__) && ((__ARMEL__ == 1) && ((__ARM_ARCH == 6) || (__ARM_ARCH == 7)))

// these ignore any memory segment directives
#define SI_SEG_GENERIC
#define SI_SEG_DATA
#define SI_SEG_IDATA
#define SI_SEG_XDATA
#define SI_SEG_PDATA
#define SI_SEG_BDATA
#define SI_SEG_CODE

// the following create a variable of the specified name but ignore the
// address and bit number.  If the using-code cares about the actual
// address or bit number, this probably will break it
#define SI_SBIT(name, address, bitnum) uint8_t name
#define SI_SFR(name, address) uint8_t name
#define SI_SFR16(name, address) uint16_t name

// the following create function and variable names of the specified types
// but the 8051-specific aspects (like memory segment) are ignored
// The format is the same as what is used in Simplicity Studio.
#define SI_INTERRUPT(name, vector) void name (void)
#define SI_INTERRUPT_USING(name, vector, regnum) void name (void)
#define SI_INTERRUPT_PROTO(name, vector) void name (void)
#define SI_INTERRUPT_PROTO_USING(name, vector, regnum) void name (void)
#define SI_FUNCTION_USING(name, return_value, parameter, regnum)              \
             return_value name (parameter)
#define SI_FUNCTION_PROTO_USING(name, return_value, parameter, regnum)        \
             return_value name (parameter)
#define SI_SEGMENT_VARIABLE(name, vartype, memseg) vartype name
#define SI_VARIABLE_SEGMENT_POINTER(name, vartype, targseg)                  \
             vartype * name
#define SI_SEGMENT_VARIABLE_SEGMENT_POINTER(name, vartype, targseg, memseg)  \
             vartype * name
#define SI_SEGMENT_POINTER(name, vartype, memseg) vartype * name
#define SI_LOCATED_VARIABLE_NO_INIT(name, vartype, memseg, address)          \
             vartype name

#define B0 0
#define B1 1
#define B2 2
#define B3 3
#define LSB 0
#define MSB 1
typedef union SI_UU16
{
  uint16_t u16;
  int16_t s16;
  uint8_t u8[2];
  int8_t s8[2];
} SI_UU16_t;

typedef union SI_UU32
{
  uint32_t u32;
  int32_t s32;
  SI_UU16_t uu16[2];
  uint16_t u16[2];
  int16_t s16[2];
  uint8_t u8[4];
  int8_t s8[4];
} SI_UU32_t;

// Generic pointer stuff is left out because if you are accessing
// generic pointer fields then it will need to be rewritten for 32-bit

// __NOP should be declared in cmsis header core_cmInstr.h
extern void __NOP(void);
/// Macro to insert a no-operation (NOP) instruction.
#define NOP() __NOP()

#else // ARM_ARCH 6 | 7
#error unsupported ARM arch
#endif

#else // unknown toolchain
#error Unrecognized toolchain in si_toolchain.h
#endif  /* defined(__C51__), defined (__GNUC__) */

/** @} */

#endif  /* __SI_TOOLCHAIN_H__ */

