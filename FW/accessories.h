#include <stdbool.h>

#ifndef _ACCESSORIES_H
#define _ACCESSORIES_H

#ifdef __cplusplus
extern "C" {
#endif

#define NO_ITERATIONS 10    // iteration for accessories identification

// define callBack function for accessories list
typedef void (*callBack)(void);

// define a structure of accessories list
typedef  struct{
  char        ID;
  char        type;
  char*       name;
  callBack    function; 
  callBack    startup;
} accessories_t; 


#define ACCS(i, t, n, f, s)  {.ID = i, .type = t, .name = n, .function = f, .startup = s}

enum ACCESSORIES_TYPE{
  ACS_NONE,
  ACS_3D_DRAW       = 1,
  ACS_BURNING       = 2,
  ACS_SOLDERING     = 3,
  ACS_FOAM_CUTTING  = 4,
};

enum ACCESSORIES_ID{
  ACS_ID_NONE          = 0x00,
  
  ACS_ID_BURNING       = 0x64,
  ACS_ID_FOAM_CUTTING  = 0x24,
  ACS_ID_3D_DRAW       = 0x35,
  ACS_ID_SOLDERING     = 0x52,
};


// return identified accessories
void acsIdentify(void);

accessories_t getAccessories(void);

void loadMaterial(int id);
void startup3D(void);
void startupBurning(void);
void startupSoldering(void);
void startupFoamCutting(void);
void startupNone(void);

#ifdef __cplusplus
}
#endif

#endif /* _NANODEUNIO_LIB_H */
