#ifndef _HC_SR04_h_
#define _HC_SR04_h_

#define TRIGGER PB0
#define ECHO    PB1
#ifdef __cplusplus
extern "C" {
#endif

class HC_SR04
{

    public:

    HC_SR04(TRIGGER, ECHO);
    float dist_cm();
};

#ifdef __cplusplus
}
#endif

#endif
