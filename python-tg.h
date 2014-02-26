#ifndef __PYTHON_TG_H__
#define __PYTHON_TG_H__

#include <string.h>
#include "structures.h"

void python_init (const char *file);
void python_finalize (void);
void python_new_msg (struct message *M);

#endif
