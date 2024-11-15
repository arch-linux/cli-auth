#ifndef CLI_AUTH_H
#define CLI_AUTH_H

#include <stddef.h>

// Returns 1 for success, 0 for failure
// If error_msg is not NULL, it will contain the error message on failure
int authenticate_user(const char *username, const char *password, char *error_msg, size_t error_msg_size);

#endif
