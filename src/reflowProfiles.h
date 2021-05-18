#ifndef _REFLOWPROFILES_H
#define _REFLOWPROFILES_H

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define NAMESIZE 64


struct reflowProfile_s
{
  char name[NAMESIZE];
  uint8_t tempProfile[6];
  uint8_t timeProfile[6];
};

struct reflowProfile_s myReflowProfile[] = {
  {
    "ChipQuik SMD4300AX10 & SMD291AX10",
    {100, 150, 183, 235, 183, 55},
    {30, 120, 150, 210, 240, 600}
  },
  {
    "ChipQuik SMD4300SNL10 Sn96.5/Sn3/Ag0.5 Water Wash",
    {150, 175, 217, 249, 217, 55},
    {90, 180, 210, 240, 270, 600}
  },
  {
    "Test profile",
    {5, 10, 15, 20, 25, 30},
    {5, 10, 15, 20, 25, 600}
  }
};

#endif /* _REFLOWPROFILES_H */