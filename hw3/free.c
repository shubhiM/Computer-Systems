#include <unistd.h>
#include "utils.h"

void free(void* ptr) {
        freeWrapper(ptr);
}
