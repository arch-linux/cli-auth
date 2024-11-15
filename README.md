
# CLI-Auth

A secure command-line and programmatic authentication tool for Linux systems that verifies user credentials against the system's PAM (Pluggable Authentication Modules) infrastructure.

## Features

- Command-line authentication interface
- Shared library for programmatic integration
- PAM-based system authentication
- Support for special characters and quoted passwords
- Detailed error reporting
- Thread-safe implementation
- Multiple integration options (CLI, C library, PHP/FFI)

## Prerequisites

The following packages are required to build the project:

```bash
sudo apt-get update
sudo apt-get install build-essential libpam0g-dev
```

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/cli-auth.git
   cd cli-auth
   ```

2. Build and install:

   ```bash
   make
   sudo make install
   ```

   This will:
   - Compile the command-line tool and shared library
   - Install the binary to `/usr/local/bin/cli-auth`
   - Install the shared library to `/usr/local/lib/libcli-auth.so`
   - Install the header file to `/usr/local/include/cli-auth.h`
   - Create and configure the PAM service file

## Usage

### Command Line Interface

Basic usage:

```bash
cli-auth username password
```

With special characters or spaces in the password:

```bash
cli-auth username 'password with spaces!!'
cli-auth username "complex@password#123"
```

The program returns:
- Exit code `0` and `OK` for successful authentication
- Exit code `1` and `FAIL:[error message]` for failed authentication

### C Library Integration

```c
#include <cli-auth.h>

int main() {
    char error_msg[1024];
    int result = authenticate_user("username", "password", error_msg, sizeof(error_msg));
    
    if (result) {
        printf("Authentication successful\n");
    } else {
        printf("Authentication failed: %s\n", error_msg);
    }
    
    return !result;
}
```

### PHP Integration

#### Using Shell Execution:

```php
<?php

class AuthenticationService
{
    public function authenticate(string $username, string $password): bool
    {
        $escapedUsername = escapeshellarg($username);
        $escapedPassword = escapeshellarg($password);
        
        exec("cli-auth {$escapedUsername} {$escapedPassword}", $output, $returnVar);
        
        return $returnVar === 0;
    }
}
```
#### Using FFI:

```php
<?php

class AuthenticationService
{
    private $ffi;

    public function __construct()
    {
        $this->ffi = FFI::cdef("
            int authenticate_user(const char *username, const char *password, 
                                char *error_msg, size_t error_msg_size);
        ", "libcli-auth.so");
    }

    public function authenticate(string $username, string $password): bool
    {
        $error_msg = FFI::new("char[1024]");
        $result = $this->ffi->authenticate_user(
            $username,
            $password,
            FFI::cast("char *", FFI::addr($error_msg)),
            1024
        );
        return $result === 1;
    }
}
```

## Security Considerations

- The command-line tool is installed with SUID permissions to allow PAM authentication
- Passwords containing special characters should be properly quoted
- When using programmatically, ensure proper escaping of username and password inputs
- The shared library maintains system-level security through PAM

## Uninstallation

To remove the application and all its components:

```bash
sudo make uninstall
```

## Development

To modify the PAM authentication rules, edit `/etc/pam.d/cli-auth` after installation. The default configuration uses standard Unix authentication:

```plaintext
#%PAM-1.0
auth    required    pam_unix.so
account required    pam_unix.so
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Authors

- Christopher Allen, archlinuxusa@gmail.com

## Acknowledgments

- PAM development team
- Linux authentication system maintainers
