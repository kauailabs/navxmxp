[![Gitter](https://img.shields.io/gitter/room/nwjs/nw.js.svg?style=flat-square)](https://gitter.im/FRC900/navX-MXP-LabVIEW?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Latest Release](https://img.shields.io/github/release/FRC900/navX-MXP-LabVIEW.svg?style=flat-square)](https://github.com/FRC900/navX-MXP-LabVIEW/releases/latest)
[![Issues](https://img.shields.io/github/issues/FRC900/navX-MXP-LabVIEW.svg?style=flat-square)](https://github.com/FRC900/navX-MXP-LabVIEW/issues)

# navX LabVIEW Library
LabVIEW library for communicating with <a href="http://www.kauailabs.com/store/index.php?route=product/product&product_id=56">Kauailabs' navX MXP Robotics Navigation Sensor</a>.

## Wiki: http://navx-mxp.kauailabs.com
* Please go here for official updates and questions not about this library.

### Supports Serial, I<sup>2</sup>C, SPI.
* Easy to use navX Library for FIRST FRC Teams.
* See "Examples/Funtions/navX Example v2.vi" for complete use of library.
* See "Docs/navX Library v2" for what VIs do.

#### To use:
1. Download the [![Latest Release](https://img.shields.io/github/release/FRC900/navX-MXP-LabVIEW.svg?style=flat-square)](https://github.com/FRC900/navX-MXP-LabVIEW/releases/latest). Either the ZIP or the lvlibp.
 * Extract the ZIP file if downloaded.
2. Include the lvlib from the extracted ZIP file or the lvlibp in your robot project.
3. In "Begin.vi" add the "IO/Z900_navX_Open.vi" and a "RefNum/Z900_navX_RefNum_Set.vi"
4. In "Finish.vi" add a "RefNum/Z900_navX_RefNum_Get.vi" and the "IO/Z900_navX_Close.vi"
 * THIS IS RECOMMENDED! DO IT! JUST, DO IT!
5. Use any of the "Get/Z900_navX_Get_*.vi" VIs elsewhere in your code.
6. Profit?
