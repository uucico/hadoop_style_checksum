src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lcrypto -static -lpthread
CFLAGS = -g -Wall -O2 -std=c99

hdfs_style_checksum: $(obj)
	cc -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) hdfs_style_checksum
