hocker: hocker.c
	$(CC) $^ -o $@

debug: hocker.c
	$(CC) -g $^ -o hocker
