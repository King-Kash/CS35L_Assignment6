# Make x86-64 random byte generators.

# Copyright 2015, 2020, 2021 Paul Eggert

# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

# Optimization level.  Change this -O2 to -Og or -O0 or whatever.
OPTIMIZE =

# The C compiler and its options.
CC = gcc
CFLAGS = $(OPTIMIZE) -g3 -Wall -Wextra -fanalyzer \
  -march=native -mtune=native -mrdrnd

SRCS   = randall.c rand64-hw.c rand64-sw.c output.c options.c
OBJS   = $(SRCS:.c=.o)

# The archiver command, its options and filename extension.
TAR = tar
TARFLAGS = --gzip --transform 's,^,randall/,'
TAREXT = tgz

default: randall

randall: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o randall

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

assignment: randall-assignment.$(TAREXT)
assignment-files = COPYING Makefile randall.c
randall-assignment.$(TAREXT): $(assignment-files)
	$(TAR) $(TARFLAGS) -cf $@ $(assignment-files)

submission-tarball: randall-submission.$(TAREXT)
submission-files = $(assignment-files) \
  notes.txt rand64-hw.c rand64-hw.h options.c options.h rand64-sw.c rand64-sw.h output.c output.h
randall-submission.$(TAREXT): $(submission-files)
	$(TAR) $(TARFLAGS) -cf $@ $(submission-files)

repository-tarball:
	$(TAR) -czf randall-git.tgz .git

.PHONY: default clean assignment submission-tarball repository-tarball check format valgrind sanitizers

clean:
	rm -f *.o *.$(TAREXT) randall

# Unit test that checks NBYTES.
# check:
# 	@echo "Running test cases..."
# 	@echo "=========== TEST 1 ==========="
# 	@./randall 20 | wc -c | (grep -q "^20$$" && echo "PASSED") || (echo "FAILED" && false)
# 	@echo "=========== TEST 2 ==========="
# 	@./randall 1000000 | python3 -c 'import sys; data = sys.stdin.buffer.read(); counts = [0] * 256; [counts.__setitem__(b, counts[b] + 1) for b in data]; total = len(data); expected = total / 256; chi2 = sum(((c - expected) ** 2) / expected for c in counts); sys.exit(0) if 206 <= chi2 <= 304 else sys.exit(1)' && echo "PASSED" || (echo "FAILED" && false)

check:
	@echo "Running test cases..."
	@echo "=========== TEST 1 ==========="
	@./randall 20 | wc -c | (grep -q "^20$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 2 ==========="
	@./randall 1000000 | python3 -c 'import sys; data = sys.stdin.buffer.read(); counts = [0] * 256; \
[counts.__setitem__(b, counts[b] + 1) for b in data]; total = len(data); expected = total / 256; \
chi2 = sum(((c - expected) ** 2) / expected for c in counts); sys.exit(0) if 206 <= chi2 <= 304 else sys.exit(1)' \
	&& echo "PASSED" || (echo "FAILED" && false)

	@echo "=========== TEST 3 ==========="
	@./randall -i rdrand 123 | wc -c | (grep -q "^123$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 4 ==========="
	@./randall -i mrand48_r 256 | wc -c | (grep -q "^256$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 5 ==========="
	@./randall -i /dev/urandom 512 | wc -c | (grep -q "^512$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 6 ==========="
	@./randall -o stdio 1005 | wc -c | (grep -q "^1005$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 7 ==========="
	@./randall -o 7 50 | wc -c | (grep -q "^50$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 8 ==========="
	@./randall -i mrand48_r -o 13 99 | wc -c | (grep -q "^99$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 9 ==========="
	@./randall 0 | wc -c | (grep -q "^0$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 10 ==========="
	@./randall -i rdrand -o stdio 2>/dev/null && (echo "FAILED" && false) || echo "PASSED"

	@echo "=========== TEST 11 ==========="
	@./randall -i notareal 10 >/dev/null 2>&1 && (echo "FAILED" && false) || echo "PASSED"

	@echo "=========== TEST 12 ==========="
	@./randall -o foo 10 >/dev/null 2>&1 && (echo "FAILED" && false) || echo "PASSED"


# Automatically formats all of your C code.
format:
	clang-format -i *.c *.h
# Helps find memory leaks. For an explanation on each flag, see this link.
valgrind: randall
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./randall 100



