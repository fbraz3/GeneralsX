#pragma once

/* Minimal shim for <malloc.h> on platforms without it (macOS)
   Redirects to <stdlib.h> which provides malloc/free declarations. */

#include <stdlib.h>

/* If extra malloc-specific declarations are required they can be added here. */
