CC = gcc
CFLAGS = -Wall -g

all : ext2_cp ext2_ln ext2_ls ext2_mkdir ext2_rm

ext2_cp: ext2_cp.c ext2.h ext2_functions.h ext2_functions.c
	$(CC) $(CFLAGS) -o $@ $^

ext2_ln: ext2_ln.c ext2.h ext2_functions.h ext2_functions.c 
	$(CC) $(CFLAGS) -o $@ $^

ext2_ls: ext2_ls.c ext2.h ext2_functions.h ext2_functions.c
	$(CC) $(CFLAGS) -o $@ $^

ext2_mkdir: ext2_mkdir.c ext2.h ext2_functions.h ext2_functions.c 
	$(CC) $(CFLAGS) -o $@ $^

ext2_rm: ext2_rm.c ext2.h ext2_functions.h ext2_functions.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f ext2_rm ext2_mkdir ext2_cp ext2_ls ext2_ln