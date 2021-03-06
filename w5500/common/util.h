    /**
    * @file     util.h
    * @brief        Common Utility Function Set Source File
    * @version  1.0
    * @date     2013/02/22
    * @par Revision
    *           2013/02/22 - 1.0 Release
    * @author   Mike Jeong
    * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
    */

    #ifndef _UTIL_H
    #define _UTIL_H


    /**
    * @defgroup macro_util Macro Util
    * Simple Macro Utilities.
    * @ingroup common_util
    * @{
    * @def MEM_FREE
    * If pointer is not NULL, free it and set to NULL.
    * @def BITSET
    * Set bit in variable.
    * @def BITCLR
    * Clear bit in variable.
    * @typedef long2char
    * General purpose type change union (32bit <-> 8bit).
    * @typedef short2char
    * General purpose type change union (16bit <-> 8bit).
    * @}
    * @typedef void_func
    * General purpose void function form.
    * @typedef alarm_cbfunc
    * Alarm call back function form.
    * @}
    */
    #define MEM_FREE(mem_p) do{ if(mem_p) { free(mem_p); mem_p = NULL; } }while(0)
    #define BITSET(var_v, bit_v) (var_v |= bit_v)
    #define BITCLR(var_v, bit_v) (var_v &= ~(bit_v))

    typedef void (*void_func)(void);
    typedef void (*alarm_cbfunc)(int8_t arg);
    typedef union long2char_t {
        uint32_t    byte4;
        uint8_t byte1[4];
    } long2char;

    typedef union short2char_t {
        uint16_t    s;
        uint8_t c[2];
    } short2char;


    int8_t alarm_set(uint32_t time, alarm_cbfunc cb, int8_t arg);
    int8_t alarm_del(alarm_cbfunc cb, int8_t arg);
    int8_t alarm_chk(alarm_cbfunc cb, int8_t arg);
    void alarm_run(void);
    int8_t   digit_length(int32_t dgt, int8_t base);
    int32_t  str_check(int (*method)(int), int8_t *str);
    void   print_dump(void *buf, uint16_t len);
    uint16_t checksum(uint8_t * src, uint32_t len);
    int32_t  base64_decode(int8_t *text, uint8_t *dst, int32_t numBytes );
    int32_t  base64_encode(int8_t *text, int32_t numBytes, int8_t *encodedText);

    #endif //_UTIL_H



