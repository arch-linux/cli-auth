#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <ctype.h>

// Function to trim quotes and whitespace
char* trim_quotes(char* str) {
    char* end;
    
    // Trim leading whitespace and quotes
    while(isspace((unsigned char)*str) || *str == '\'' || *str == '"') str++;
    
    if(*str == 0) return str;  // All spaces
    
    // Trim trailing whitespace and quotes
    end = str + strlen(str) - 1;
    while(end > str && (isspace((unsigned char)*end) || *end == '\'' || *end == '"')) end--;
    
    end[1] = '\0';
    
    return str;
}

// Custom conversation function that uses provided credentials
static int custom_conv(int num_msg, const struct pam_message **msg,
                      struct pam_response **resp, void *appdata_ptr) {
    struct pam_response *reply = calloc(num_msg, sizeof(struct pam_response));
    if (reply == NULL) {
        return PAM_CONV_ERR;
    }

    *resp = reply;
    const char *password = (const char *)appdata_ptr;

    for (int i = 0; i < num_msg; i++) {
        switch (msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_OFF:
            case PAM_PROMPT_ECHO_ON:
                reply[i].resp = strdup(password);
                if (reply[i].resp == NULL) {
                    return PAM_CONV_ERR;
                }
                break;

            case PAM_ERROR_MSG:
            case PAM_TEXT_INFO:
                break;

            default:
                for (int j = 0; j < i; j++) {
                    if (reply[j].resp) {
                        free(reply[j].resp);
                    }
                }
                free(reply);
                *resp = NULL;
                return PAM_CONV_ERR;
        }
    }

    return PAM_SUCCESS;
}

// Main authentication function
int authenticate_user(const char *username, const char *password, char *error_msg, size_t error_msg_size) {
    pam_handle_t *pamh = NULL;
    struct pam_conv conv;
    int retval;

    // Initialize error message
    if (error_msg && error_msg_size > 0) {
        error_msg[0] = '\0';
    }

    // Set up the custom conversation function
    conv.conv = custom_conv;
    conv.appdata_ptr = (void *)password;

    // Initialize PAM
    retval = pam_start("cli-auth", username, &conv, &pamh);
    if (retval != PAM_SUCCESS) {
        if (error_msg && error_msg_size > 0) {
            snprintf(error_msg, error_msg_size, "Failed to initialize PAM: %s", pam_strerror(pamh, retval));
        }
        return 0;
    }

    // Authenticate
    retval = pam_authenticate(pamh, 0);
    
    // Check account validity
    if (retval == PAM_SUCCESS) {
        retval = pam_acct_mgmt(pamh, 0);
    }

    if (retval != PAM_SUCCESS && error_msg && error_msg_size > 0) {
        snprintf(error_msg, error_msg_size, "%s", pam_strerror(pamh, retval));
    }

    pam_end(pamh, retval);

    return (retval == PAM_SUCCESS) ? 1 : 0;
}

// Command line interface
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s username password\n", argv[0]);
        return 1;
    }

    // Combine all arguments after the username into the password
    // This handles cases where the password contains spaces
    char password[1024] = "";
    for (int i = 2; i < argc; i++) {
        if (i > 2) strcat(password, " ");
        strcat(password, argv[i]);
    }

    // Trim quotes from username and password
    char *cleaned_username = strdup(argv[1]);
    cleaned_username = trim_quotes(cleaned_username);
    char *cleaned_password = trim_quotes(password);

    char error_msg[1024];
    int result = authenticate_user(cleaned_username, cleaned_password, error_msg, sizeof(error_msg));

    free(cleaned_username);

    if (result) {
        printf("OK\n");
        return 0;
    } else {
        printf("FAIL:%s\n", error_msg);
        return 1;
    }
}
