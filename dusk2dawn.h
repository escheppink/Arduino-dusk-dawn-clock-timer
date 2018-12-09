/*  Dusk2Dawn.h
 *  Get time of sunrise and sunset.
 *  Created by DM Kishi <dm.kishi@gmail.com> on 2017-02-01.
 *  <https://github.com/dmkishi/Dusk2Dawn>
 *  
 *  Interface modified to have better fit in this program.
 *  Most class variables removed to decrease memory usage.
 */

#ifndef Dusk2Dawn_h
#define Dusk2Dawn_h

#include "Arduino.h"
#include <math.h>

namespace dusk_dawn_timer {

class Dusk2Dawn {
  public:
    void update(uint16_t year, uint8_t month, uint8_t day, bool isDST);
    uint16_t mSunrise;
    uint16_t mSunset;
  private:
    uint8_t mDateHash;
    static int sunrise(int y, int m, int d, bool isDST);
    static int sunset(int y, int m, int d, bool isDST);
    static int   sunriseSet(bool, int, int, int, bool);
    static float sunriseSetUTC(bool, float, float, float);
    static float equationOfTime(float);
    static float meanObliquityOfEcliptic(float);
    static float eccentricityEarthOrbit(float);
    static float sunDeclination(float);
    static float sunApparentLong(float);
    static float sunTrueLong(float);
    static float sunEqOfCenter(float);
    static float hourAngleSunrise(float, float);
    static float obliquityCorrection(float);
    static float geomMeanLongSun(float);
    static float geomMeanAnomalySun(float);
    static float jDay(int, int, int);
    static float fractionOfCentury(float);
    static float radToDeg(float);
    static float degToRad(float);
};

} // namespace

#endif
