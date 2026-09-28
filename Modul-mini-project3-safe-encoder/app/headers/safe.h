#ifndef SAFE_H
#define SAFE_H

#include "safe_types.h"

void safe_init(void);

void safe_handle_encoder(EncoderDirection direction);

void safe_handle_button(void);

const SafeContext *safe_get_context(void);

#endif