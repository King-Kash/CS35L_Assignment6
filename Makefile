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


check: randall
	@echo "Running test cases..."

	@echo "=========== TEST 1 ==========="
	@./randall 20 | wc -c | (grep -q "^20$$" && echo "PASSED") || (echo "FAILED" && false)

	@echo "=========== TEST 2 ==========="
	@./randall 1000000 | python3 -c 'import sys; data = sys.stdin.buffer.read(); counts = [0] * 256; [counts.__setitem__(b, counts[b] + 1) for b in data]; total = len(data); expected = total / 256; chi2 = sum(((c - expected) ** 2) / expected for c in counts); sys.exit(0) if 206 <= chi2 <= 304 else sys.exit(1)' && echo "PASSED" || (echo "FAILED" && false)

	@echo "=========== TEST 3: default backend ==========="
	@./randall -i rdrand 50 | wc -c | grep -q "^50$$" && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 4: mrand48_r backend ==========="
	@./randall -i mrand48_r 60 | wc -c | grep -q "^60$$" && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 5: file backend (/dev/zero) ==========="
	@./randall -i /dev/zero 100 | od -An -t u1 | tr -s ' ' '\n' | head -100 | grep -q "^0$$" && \
	  wc -c <(./randall -i /dev/zero 100) | grep -q "^100$$" && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 6: stdio output ==========="
	@./randall -o stdio 70 | wc -c | grep -q "^70$$" && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 7: blocked-write output ==========="
	@CHUNK=10; TOTAL=25; \
	 [ "$$(./randall -o $$CHUNK $$TOTAL | wc -c)" -eq "$$TOTAL" ] && \
	 [ "$$(strace -e trace=write ./randall -o $$CHUNK $$TOTAL 2>&1 | grep -c '^write(')" -eq $$(( (TOTAL+CHUNK-1)/CHUNK )) ] && \
	 echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 8: missing NBYTES ==========="
	-@! ./randall -i rdrand -o stdio   && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 9: non-numeric NBYTES ==========="
	-@! ./randall abc                && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 10: invalid input option ==========="
	-@! ./randall -i foo 10          && echo "PASSED" || echo "FAILED"

	@echo "=========== TEST 11: invalid output option ==========="
	-@! ./randall -o bar 10          && echo "PASSED" || echo "FAILED"

	@echo "All tests complete."
# Automatically formats all of your C code.
format:
	clang-format -i *.c *.h
# Helps find memory leaks. For an explanation on each flag, see this link.
valgrind: randall
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./randall 100



