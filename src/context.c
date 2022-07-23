#include "context.h"

#include <stdbool.h>

static Context context = {
    .help = false,
    .host = "localhost",
    .port = "80",
    .root = ".",
};

Context *GetContext() { return &context; }
