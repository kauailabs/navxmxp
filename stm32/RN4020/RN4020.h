/* ============================================
navX-blue source code is placed under the MIT license
Copyright (c) 2015 Kauai Labs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef RN4020_H_
#define RN4020_H_

#include "navx-mxp_hal.h"

class RN4020
{
public:
    RN4020();
    bool isPresent() {
#ifdef ENABLE_RN4020
        return true;
#else
        return false;
#endif
    }

    void Sleep() { HAL_RN4020_Sleep(); }
    void Wake() { HAL_RN4020_Wake(); }
    bool isRTS() { return HAL_RN4020_Get_RTS() != 0; }
    void setCTS() { HAL_RN4020_Set_CTS(1); }
    void clearCTS() { HAL_RN4020_Set_CTS(0); }
    bool isMLDP_EV() { return HAL_RN4020_Get_MLDP_EV() != 0; }
    void setCMD_MLDP() { HAL_RN4020_Set_CMD_MLDP(1); }
    void clearCMD_MLDP() { HAL_RN4020_Set_CMD_MLDP(0); }
    virtual ~RN4020();
};

#endif /* RN4020_H_ */
