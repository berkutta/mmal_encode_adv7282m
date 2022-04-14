#ifndef __MMALENCODER_H__
#define __MMALENCODER_H__

#include <stdio.h>

void mmal_new_data(char *buffer, size_t length);
int mmal_encoder(void);

#endif
