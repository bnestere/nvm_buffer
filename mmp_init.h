#ifndef _MMP_INIT_
#define _MMP_INIT_

#include "mmp_user.h"

typedef struct mmp_initializer_t mmp_initializer_t;

struct mmp_initializer_t {
  rt_mem_t *(*initialize)(void);
};

//Functions
mmp_initializer_t *get_mmp_initializer(void);

#endif
