#pragma once
#include "session.h"

void map_Load();
void mapinit(type_session *c);
void map_pData_snd(type_session *c, char *mesg, int pLen);
void map_get_sectorinfo_All(type_session *c);