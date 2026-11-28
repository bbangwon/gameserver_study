#pragma once

void skill_defset(int type, int skillidx, char *lvpwr, char *lvupcost, int coin);
void skill_get(void *chr, char *dat);
void skill_lvup(void *chr, char *dat);
void skill_use(void *chr, char *dat, void *conn);