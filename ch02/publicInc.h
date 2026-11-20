#pragma once

/*map size*/
#define S_WIDTH  		32

extern pthread_mutex_t 		synclock;
extern pthread_cond_t		synccond;
extern pthread_mutex_t      sendQuelock;
extern pthread_cond_t       sendQuecond;