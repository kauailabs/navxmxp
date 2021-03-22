#ifndef VMX_PI_TEST_JIG_REGISTERS_H_
#define VMX_PI_TEST_JIG_REGISTERS_H_

#include "VMXPiTestJigProtocol.h"

/*******************************************************************/
/*******************************************************************/
/*                      Register Definitions                       */
/*******************************************************************/
/* NOTE:  All multi-byte registers are in little-endian format.    */
/*        All registers with 'signed' data are twos-complement.    */
/*        Data Type Summary:                                       */
/*        unsigned byte:           0   - 255    (8 bits)           */
/*        unsigned short:          0   - 65535  (16 bits)          */
/*        signed short:        -32768  - 32767  (16 bits)          */
/*        signed hundredeths:  -327.68 - 327.67 (16 bits)		   */
/*        unsigned hundredths:    0.0  - 655.35 (16 bits)          */
/*        signed thousandths:  -32.768 - 32.767 (16 bits)          */
/*        signed short ratio: -1/16384 - 1/16384 (16 bits)         */
/*        16:16:           -32768.9999 - 32767.9999 (32 bits)      */
/*        unsigned long:             0 - 4294967295 (32 bits)      */
/*******************************************************************/

typedef int16_t		s_short_hundred_float;
typedef uint16_t	u_short_hundred_float;
typedef int16_t     s_short_thousand_float;
typedef int16_t     s_short_ratio_float;
typedef int32_t     s_1616_float;

/**********************************************/
/* Device Identification Registers            */
/**********************************************/

#define NAVX_REG_WHOAMI             0x00 /* IMU_MODEL_XXX */
#define NAVX_REG_HW_REV             0x01
#define NAVX_REG_FW_VER_MAJOR       0x02
#define NAVX_REG_FW_VER_MINOR       0x03

/**********************************************/
/* Status and Control Registers               */
/**********************************************/

/* Read-write */
#define	VMX_PI_TEST_JIG_COMMDIO_OPMODE	0x04
#define VMX_PI_TEST_JIG_COMMDIO_OUTPUT	0x05
/* Read-only */
#define VMX_PI_TEST_JIG_COMMDIO_INPUT	0x06

#define NAVX_REG_LAST VMX_PI_TEST_JIG_COMMDIO_INPUT

/* NAVX_MODEL */

#define NAVX_MODEL_NAVX_MXP                         0x32

/* NAVX_CAL_STATUS */

#define VMX_PI_TEST_JIG_COMMDIO_OPMODE_MASK         0x03
#define VMX_PI_TEST_JIG_COMMDIO_OPMODE_INPUT		0x00
#define VMX_PI_TEST_JIG_COMMDIO_OPMODE_OUTPUT		0x01
#define VMX_PI_TEST_JIG_COMMDIO_OPMODE_ECHO			0x02

class VMXPiTestJigRegisters
{
public:
    /************************************************************/
    /* NOTE:                                                    */
    /* The following functions assume a little-endian processor */
    /************************************************************/

    static inline uint16_t decodeProtocolUint16( char *uint16_bytes ) {
        return *((uint16_t *)uint16_bytes);
    }
    static inline void encodeProtocolUint16( uint16_t val, char *uint16_bytes) {
        *((uint16_t *)uint16_bytes) = val;
    }

    static inline int16_t decodeProtocolInt16( char *int16_bytes ) {
        return *((int16_t *)int16_bytes);
    }
    static inline void encodeProtocolInt16( int16_t val, char *int16_bytes) {
        *((int16_t *)int16_bytes) = val;
    }
	
    static inline uint32_t decodeProtocolUint32( char *uint32_bytes ) {
        return *((uint32_t *)uint32_bytes);
    }	

    static inline int32_t decodeProtocolInt32( char *int32_bytes ) {
        return *((int32_t *)int32_bytes);
    }
    static inline void encodeProtocolInt32( int32_t val, char *int32_bytes) {
        *((int32_t *)int32_bytes) = val;
    }

    /* -327.68 to +327.68 */
    static inline float decodeProtocolSignedHundredthsFloat( char *uint8_signed_angle_bytes ) {
        float signed_angle = (float)decodeProtocolInt16(uint8_signed_angle_bytes);
        signed_angle /= 100;
        return signed_angle;
    }
    static inline void encodeProtocolSignedHundredthsFloat( float input, char *uint8_signed_hundredths_float) {
        int16_t input_as_int = (int16_t)(input * 100.0f);
        encodeProtocolInt16(input_as_int,uint8_signed_hundredths_float);
    }

    static inline s_short_hundred_float encodeSignedHundredthsFloat( float input ) {
        return (s_short_hundred_float)(input * 100.0f);
    }
    static inline u_short_hundred_float encodeUnsignedHundredthsFloat(float input ) {
        return (u_short_hundred_float)(input * 100.0f);
    }

    static inline s_short_ratio_float encodeRatioFloat(float input_ratio) {
        return (s_short_hundred_float)(input_ratio *= 32768.0f);
    }
    static inline s_short_thousand_float encodeSignedThousandthsFloat(float input) {
        return (s_short_thousand_float)(input * 1000.0f);
    }

    /* 0 to 655.35 */
    static inline float decodeProtocolUnsignedHundredthsFloat( char *uint8_unsigned_hundredths_float ) {
        float unsigned_float = (float)decodeProtocolUint16(uint8_unsigned_hundredths_float);
        unsigned_float /= 100;
        return unsigned_float;
    }
    static inline void encodeProtocolUnsignedHundredthsFloat( float input, char *uint8_unsigned_hundredths_float) {
        uint16_t input_as_uint = (uint16_t)(input * 100.0f);
        encodeProtocolUint16(input_as_uint,uint8_unsigned_hundredths_float);
    }

    /* -32.768 to +32.768 */
    static inline float decodeProtocolSignedThousandthsFloat( char *uint8_signed_angle_bytes ) {
        float signed_angle = (float)decodeProtocolInt16(uint8_signed_angle_bytes);
        signed_angle /= 1000;
        return signed_angle;
    }
    static inline void encodeProtocolSignedThousandthsFloat( float input, char *uint8_signed_thousandths_float) {
        int16_t input_as_int = (int16_t)(input * 1000.0f);
        encodeProtocolInt16(input_as_int,uint8_signed_thousandths_float);
    }

    /* In units of -1 to 1, multiplied by 16384 */
    static inline float decodeProtocolRatio( char *uint8_ratio ) {
        float ratio = (float)decodeProtocolInt16(uint8_ratio);
        ratio /= 32768.0f;
        return ratio;
    }
    static inline void encodeProtocolRatio( float ratio, char *uint8_ratio ) {
        ratio *= 32768.0f;
        encodeProtocolInt16(ratio,uint8_ratio);
    }

    /* <int16>.<uint16> (-32768.9999 to 32767.9999) */
    static float decodeProtocol1616Float( char *uint8_16_16_bytes ) {
        float result = (float)decodeProtocolInt32( uint8_16_16_bytes );
        result /= 65536.0f;
        return result;
    }
    static void encodeProtocol1616Float( float val, char *uint8_16_16_bytes ) {
        val *= 65536.0f;
        int32_t packed_float = (int32_t)val;
        encodeProtocolInt32(packed_float, uint8_16_16_bytes);
    }

#define CRC7_POLY 0x91

    static void buildCRCLookupTable( uint8_t* table, size_t length )
    {
        size_t crc;
        size_t i, j;
        if ( length == 256 ) {
            for ( i = 0; i < length; i++ ) {
                crc = (uint8_t)i;
                for (j = 0; j < 8; j++) {
                    if (crc & 1) {
                        crc ^= CRC7_POLY;
                    }
                    crc >>= 1;
                }
                table[i] = crc;
            }
        }
    }

    static inline uint8_t getCRCWithTable( uint8_t* table, uint8_t message[], uint8_t length )
    {
        uint8_t i, crc = 0;

        for (i = 0; i < length; i++)
        {
            crc ^= message[i];
            crc = table[crc];
        }
        return crc;
    }

    static uint8_t getCRC(uint8_t message[], uint8_t length)
    {
        uint8_t i, j, crc = 0;

        for (i = 0; i < length; i++)
        {
            crc ^= message[i];
            for (j = 0; j < 8; j++)
            {
                if (crc & 1) {
                    crc ^= CRC7_POLY;
                }
                crc >>= 1;
            }
        }
        return crc;
    }
};


#endif /* VMX_PI_TEST_JIG_REGISTERS_H_ */
