#Yep, this is my Makefile

CC     = gcc
CFLAGS = -O1 -fsanitize=address
LFLAGS = -lc -fsanitize=address

EXEC = run


all:
	$(CC) $(CFLAGS) $(LFLAGS) -o $(EXEC) main.c service/daemonize.c ipc/sem.c daemon/backup.c daemon/cwd.c common/error.c daemon/event.c io/cmdline.c daemon/inotify.c daemon/vfs.c daemon/watch.c io/io.c ipc/signal.c daemon/daemon.c ipc/shmem.c utils/hash.c utils/apply.c utils/copy.c utils/diff.c

clean:
	rm -f $(EXEC)
