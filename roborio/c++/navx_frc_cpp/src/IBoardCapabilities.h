/*
 * IBoardCapabilities.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_IBOARDCAPABILITIES_H_
#define SRC_IBOARDCAPABILITIES_H_

class IBoardCapabilities {
public:
    IBoardCapabilities() {}
    virtual bool IsOmniMountSupported() = 0;
    virtual bool IsBoardYawResetSupported() = 0;
    virtual bool IsDisplacementSupported() = 0;
    virtual bool IsAHRSPosTimestampSupported() = 0;
};

#endif /* SRC_IBOARDCAPABILITIES_H_ */
