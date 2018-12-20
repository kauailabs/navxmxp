/*
 * RegisterIOI2C.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_REGISTERIOI2C_H_
#define SRC_REGISTERIOI2C_H_

#include "RegisterIO.h"
#include "frc/I2C.h"

using namespace frc;

class RegisterIO_I2C : public IRegisterIO {
public:
    RegisterIO_I2C(I2C *port);
    virtual ~RegisterIO_I2C() {}
    bool Init();
    bool Write(uint8_t address, uint8_t value );
    bool Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len);
    bool Shutdown();
    void EnableLogging(bool enable);
private:
    I2C *port;
    bool trace;
};

#endif /* SRC_REGISTERIOI2C_H_ */
