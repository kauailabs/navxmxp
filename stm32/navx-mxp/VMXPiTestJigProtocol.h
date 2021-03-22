#ifndef _VMX_PI_TEST_JIG_PROTOCOL_H_
#define _VMX_PI_TEST_JIG_PROTOCOL_H_

#define PACKET_START_CHAR       '!'
#define CHECKSUM_LENGTH         2   /* 8-bit checksump, all bytes before checksum */
#define TERMINATOR_LENGTH       2   /* Carriage Return, Line Feed */

#define BINARY_PACKET_INDICATOR_CHAR '#'  // NOTE:  Not currently used

#define PROTOCOL_FLOAT_LENGTH   7

#define TEST_JIG_OPMODE_INPUT						'I'
#define TEST_JIG_OPMODE_OUTPUT						'O'
#define TEST_JIG_OPMODE_ECHO						'E'

#define MSGID_TEST_JIG_UPDATE						'u'
#define TEST_JIG_UPDATE_OPMODE_VALUE_INDEX			2 // TEST_JIG_OPMODE_XXX
#define TEST_JIG_UPDATE_COMMDIO_INPUT_VALUE_INDEX	3
#define TEST_JIG_UPDATE_CHECKSUM_INDEX				5
#define TEST_JIG_UPDATE_TERMINATOR_INDEX			7
#define TEST_JIG_UPDATE_MESSAGE_LENGTH				9

#define MSGID_TEST_JIG_CONFIG						'c'
#define TEST_JIG_CONFIG_OPMODE_VALUE_INDEX			2 // TEST_JIG_OPMODE_XXX
#define TEST_JIG_CONFIG_COMMDIO_OUTPUT_VALUE_INDEX	3
#define TEST_JIG_CONFIG_CHECKSUM_INDEX				5
#define TEST_JIG_CONFIG_TERMINATOR_INDEX			7
#define TEST_JIG_CONFIG_MESSAGE_LENGTH				9

#include <stdio.h>
#include <stdlib.h>

#define TEST_JIG_PROTOCOL_MAX_MESSAGE_LENGTH TEST_JIG_CONFIG_MESSAGE_LENGTH

#define TEST_JIG_COMMDIO_PIN_POSITION_I2C_SDA		(1 << 0)
#define TEST_JIG_COMMDIO_PIN_POSITION_I2C_SCL		(1 << 1)
#define TEST_JIG_COMMDIO_PIN_POSITION_UART_TX		(1 << 2)
#define TEST_JIG_COMMDIO_PIN_POSITION_UART_RX		(1 << 3)
#define TEST_JIG_COMMDIO_PIN_POSITION_SPI_SCK		(1 << 4)
#define TEST_JIG_COMMDIO_PIN_POSITION_SPI_MOSI		(1 << 5)
#define TEST_JIG_COMMDIO_PIN_POSITION_SPI_MISO		(1 << 6)
#define TEST_JIG_COMMDIO_PIN_POSITION_SPI_CS		(1 << 7)

class VMXPiTestJigProtocol
{
public:

	struct TestJigUpdate {
		char opmode;
		uint8_t commdio_input;
	};

	struct TestJigConfig {
		char opmode;
		uint8_t commdio_output; // Only valid if Opmode == TEST_JIG_OPMODE_INPUT
	};


public:

    static int encodeTestJigUpdate( char *protocol_buffer, char opmode, uint8_t commdio_input )
    {
        // Header
        protocol_buffer[0] = PACKET_START_CHAR;
        protocol_buffer[1] = MSGID_TEST_JIG_UPDATE;

        // Data
        protocol_buffer[TEST_JIG_UPDATE_OPMODE_VALUE_INDEX] = opmode;
        encodeProtocolUint8( commdio_input, &protocol_buffer[TEST_JIG_UPDATE_COMMDIO_INPUT_VALUE_INDEX]);

        // Footer
        encodeTermination( protocol_buffer, TEST_JIG_UPDATE_MESSAGE_LENGTH, TEST_JIG_UPDATE_MESSAGE_LENGTH - 4 );

        return TEST_JIG_UPDATE_MESSAGE_LENGTH;
    }

    static int decodeTestJigUpdate( char *buffer, int length, char& opmode, uint8_t& commdio_input )
    {
        if ( length < TEST_JIG_UPDATE_MESSAGE_LENGTH ) return 0;
        if ( ( buffer[0] == '!' ) && ( buffer[1] == MSGID_TEST_JIG_UPDATE ) )
        {
            if ( !verifyChecksum( buffer, TEST_JIG_UPDATE_CHECKSUM_INDEX ) ) return 0;

            opmode = buffer[TEST_JIG_UPDATE_OPMODE_VALUE_INDEX];
            commdio_input = decodeUint8( &buffer[TEST_JIG_UPDATE_COMMDIO_INPUT_VALUE_INDEX] );

            return TEST_JIG_UPDATE_MESSAGE_LENGTH;
        }
        return 0;
    }

    static int encodeTestJigConfig( char *protocol_buffer, char opmode, uint8_t commdio_output )
    {
        // Header
        protocol_buffer[0] = PACKET_START_CHAR;
        protocol_buffer[1] = MSGID_TEST_JIG_UPDATE;

        // Data
        protocol_buffer[TEST_JIG_CONFIG_OPMODE_VALUE_INDEX] = opmode;
        encodeProtocolUint8( commdio_output, &protocol_buffer[TEST_JIG_CONFIG_COMMDIO_OUTPUT_VALUE_INDEX]);

        // Footer
        encodeTermination( protocol_buffer, TEST_JIG_CONFIG_MESSAGE_LENGTH, TEST_JIG_CONFIG_MESSAGE_LENGTH - 4 );

        return TEST_JIG_CONFIG_MESSAGE_LENGTH;
    }

    static int decodeTestJigConfig( char *buffer, int length, char& opmode, uint8_t& commdio_output )
    {
        if ( length < TEST_JIG_CONFIG_MESSAGE_LENGTH ) return 0;
        if ( ( buffer[0] == '!' ) && ( buffer[1] == MSGID_TEST_JIG_CONFIG ) )
        {
            if ( !verifyChecksum( buffer, TEST_JIG_CONFIG_CHECKSUM_INDEX ) ) return 0;

            opmode = buffer[TEST_JIG_CONFIG_OPMODE_VALUE_INDEX];
            commdio_output = decodeUint8( &buffer[TEST_JIG_CONFIG_COMMDIO_OUTPUT_VALUE_INDEX] );

            return TEST_JIG_CONFIG_MESSAGE_LENGTH;
        }
        return 0;
    }

protected:

    static void encodeTermination( char *buffer, int total_length, int content_length )
    {
        if ( ( total_length >= (CHECKSUM_LENGTH + TERMINATOR_LENGTH) ) && ( total_length >= content_length + (CHECKSUM_LENGTH + TERMINATOR_LENGTH) ) )
        {
            // Checksum
            unsigned char checksum = 0;
            for ( int i = 0; i < content_length; i++ )
            {
                checksum += buffer[i];
            }
            // convert checksum to two ascii bytes
            sprintf(&buffer[content_length], "%02X", checksum);
            // Message Terminator
            sprintf(&buffer[content_length + CHECKSUM_LENGTH], "%s","\r\n");
        }
    }

    // Formats a float as follows
    //
    // e.g., -129.235
    //
    // "-129.24"
    //
    // e.g., 23.4
    //
    // "+023.40"

    static void encodeProtocolFloat( float f, char* buff )
    {
        char work_buffer[PROTOCOL_FLOAT_LENGTH + 1];
        int i;
        int temp1 = abs((int)((f - (int)f) * 100));
        if ( f < 0 ) buff[0] = '-'; else buff[0] = ' ';
        sprintf(work_buffer,"%03d.%02d", abs((int)f), temp1);
        for ( i = 0; i < (PROTOCOL_FLOAT_LENGTH-1); i++ ) {
            buff[1 + i] = work_buffer[i];
        }
    }

    static void encodeProtocolUint16( uint16_t value, char* buff )
    {
        sprintf(&buff[0],"%04X", value );
    }

    static uint16_t decodeProtocolUint16( char *uint16_string )
    {
        uint16_t decoded_uint16 = 0;
        unsigned int shift_left = 12;
        for ( int i = 0; i < 4; i++ )
        {
            unsigned char digit = uint16_string[i] <= '9' ? uint16_string[i] - '0' : ((uint16_string[i] - 'A') + 10);
            decoded_uint16 += (((uint16_t)digit) << shift_left);
            shift_left -= 4;
        }
        return decoded_uint16;
    }

    static void encodeProtocolUint8( uint8_t value, char* buff )
    {
        sprintf(&buff[0],"%02X", value );
    }

    static uint8_t decodeProtocolUint8( char *uint8_string )
    {
        return decodeUint8(uint8_string);
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

    static bool verifyChecksum( char *buffer, int content_length )
    {
        // Calculate Checksum
        unsigned char checksum = 0;
        for ( int i = 0; i < content_length; i++ )
        {
            checksum += buffer[i];
        }

        // Decode Checksum
        unsigned char decoded_checksum = decodeUint8( &buffer[content_length] );

        return ( checksum == decoded_checksum );
    }

    static unsigned char decodeUint8( char *checksum )
    {
        unsigned char first_digit = checksum[0] <= '9' ? checksum[0] - '0' : ((checksum[0] - 'A') + 10);
        unsigned char second_digit = checksum[1] <= '9' ? checksum[1] - '0' : ((checksum[1] - 'A') + 10);
        unsigned char decoded_checksum = (first_digit * 16) + second_digit;
        return decoded_checksum;
    }

    static float decodeProtocolFloat( char *buffer )
    {
        char temp[PROTOCOL_FLOAT_LENGTH+1];
        for ( int i = 0; i < PROTOCOL_FLOAT_LENGTH; i++ )
        {
            temp[i] = buffer[i];
        }
        temp[PROTOCOL_FLOAT_LENGTH] = 0;
        return atof(temp);
    }

};

#endif // _IMU_PROTOCOL_H_
