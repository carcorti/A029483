CC ?= gcc
MANDATORY_CFLAGS := -std=c17 -Wall -Wextra -Wshadow -Wconversion -Wdouble-promotion -Wformat=2 -pthread
OPT_CFLAGS ?= -O3 -march=native -mtune=native -flto -funroll-loops -fomit-frame-pointer
CFLAGS := $(MANDATORY_CFLAGS) $(OPT_CFLAGS)
LDFLAGS ?= -flto -pthread -lm
SAN_CFLAGS := -std=c17 -O1 -g3 -Wall -Wextra -pthread -fsanitize=address,undefined
SAN_LDFLAGS := -pthread -fsanitize=address,undefined -lm
TSAN_CFLAGS := -std=c17 -O1 -g -Wall -Wextra -pthread -fsanitize=thread -no-pie
TSAN_LDFLAGS := -pthread -fsanitize=thread -no-pie -lm

TARGET := a029483
SRC := a029483.c
SAN_TARGET := $(TARGET)_san

.PHONY: all clean self-test test sanitize tsan

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

self-test: $(TARGET)
	./$(TARGET) --self-test

test: self-test
	rm -f /tmp/a029483_test_hits.tsv /tmp/a029483_test_ckpt.txt
	./$(TARGET) --start 1 --end 250000 --block-size 50000 --threads 4 --out /tmp/a029483_test_hits.tsv --checkpoint /tmp/a029483_test_ckpt.txt --quiet
	printf '1\n13\n143\n169\n221\n403\n587\n11219\n178357\n222157\n' > /tmp/a029483_ref.tsv
	sort -n /tmp/a029483_test_hits.tsv > /tmp/a029483_test_hits_sorted.tsv
	diff -u /tmp/a029483_ref.tsv /tmp/a029483_test_hits_sorted.tsv
	printf 'next_start=0\n' > /tmp/a029483_bad_ckpt.txt
	! ./$(TARGET) --resume /tmp/a029483_bad_ckpt.txt --end 10 --out /tmp/a029483_bad_hits.tsv --checkpoint /tmp/a029483_bad_out_ckpt.txt --quiet
	! ./$(TARGET) --start 1 --end 10 --out /tmp/a029483_alias.tsv --checkpoint /tmp/a029483_alias.tsv --quiet
	! ./$(TARGET) --start 0 --end 10 --out /tmp/a029483_zero.tsv --checkpoint /tmp/a029483_zero_ckpt.txt --quiet
	rm -rf /tmp/a029483_outdir
	mkdir -p /tmp/a029483_outdir
	./$(TARGET) --start 1 --end 10 --out /tmp/a029483_outdir/hits.tsv --checkpoint /tmp/a029483_outdir/ckpt.txt --quiet

$(SAN_TARGET): $(SRC)
	$(CC) $(SAN_CFLAGS) -o $@ $< $(SAN_LDFLAGS)

sanitize: $(SAN_TARGET)
	ASAN_OPTIONS=detect_leaks=0 ./$(SAN_TARGET) --self-test
	rm -f /tmp/a029483_san_hits.tsv /tmp/a029483_san_ckpt.txt
	ASAN_OPTIONS=detect_leaks=0 ./$(SAN_TARGET) --start 1 --end 20000 --block-size 5000 --threads 4 --out /tmp/a029483_san_hits.tsv --checkpoint /tmp/a029483_san_ckpt.txt --quiet

$(TARGET)_tsan: $(SRC)
	$(CC) $(TSAN_CFLAGS) -o $@ $< $(TSAN_LDFLAGS)

tsan: $(TARGET)_tsan
	setarch $$(uname -m) -R ./$(TARGET)_tsan --start 1 --end 10000 --block-size 5000 --threads 4 --out /tmp/a029483_tsan_hits.tsv --checkpoint /tmp/a029483_tsan_ckpt.txt --quiet

clean:
	rm -f $(TARGET) $(SAN_TARGET) $(TARGET)_tsan
