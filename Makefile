CC=gcc
CFLAGS=-Wall -Wextra -fPIC
LIBS=-lpam -lpam_misc
PAM_CONFIG_FILE=/etc/pam.d/cli-auth

all: cli-auth libcli-auth.so

cli-auth: cli-auth.c
	$(CC) $(CFLAGS) -o cli-auth cli-auth.c $(LIBS)

libcli-auth.so: cli-auth.c
	$(CC) $(CFLAGS) -shared -o libcli-auth.so cli-auth.c $(LIBS)

install: cli-auth libcli-auth.so install-pam
	install -m 4755 cli-auth /usr/local/bin/cli-auth
	install -m 0644 cli-auth.h /usr/local/include/
	install -m 0755 libcli-auth.so /usr/local/lib/
	ldconfig

install-pam:
	@echo "Installing PAM configuration..."
	@echo "#%PAM-1.0" | sudo tee $(PAM_CONFIG_FILE)
	@echo "auth    required    pam_unix.so" | sudo tee -a $(PAM_CONFIG_FILE)
	@echo "account required    pam_unix.so" | sudo tee -a $(PAM_CONFIG_FILE)
	@sudo chmod 644 $(PAM_CONFIG_FILE)

uninstall:
	rm -f /usr/local/bin/cli-auth
	rm -f /usr/local/include/cli-auth.h
	rm -f /usr/local/lib/libcli-auth.so
	rm -f $(PAM_CONFIG_FILE)
	ldconfig

clean:
	rm -f cli-auth libcli-auth.so
