#pragma once

#include <stdio.h>

#define SAGEPATCH_LOG(fmt, ...) \
    fprintf(stderr, "[sagepatch] " fmt "\n", ##__VA_ARGS__)
