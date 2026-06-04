# A029483 final validation notes

Date: 2026-05-30

Final public artifacts:

- `src/a029483.c`
- `src/Makefile`
- `src/a029483`

Promoted after the fifth internal code-review round.

Local smoke tests executed before final public promotion:

- Release build: passed, zero warnings.
- `./a029483 --self-test`: passed.
- `make -f Makefile test`: passed.
- `make -f Makefile sanitize`: passed with `ASAN_OPTIONS=detect_leaks=0`.
- `make -f Makefile tsan`: passed.
- Cross-thread determinism on `[1,250000]` for threads `1,2,4,8,16`: passed.
- Large known term replay: emitted `15081597011`.
- `scan-build make -f Makefile clean all`: no bugs found.
- `cppcheck --enable=warning,style,performance,portability --std=c17 a029483.c`: only the known defensive-wrap false positive.
- `clang-tidy-18 a029483.c -- -std=c17 -pthread`: only standard C/POSIX insecure-API portability warnings, triaged as non-actionable.
- Valgrind self-test on non-native debug build: zero errors, zero leaks.
- Valgrind smoke run on non-native debug build: zero errors, zero leaks.
- `hyperfine` 1,000,000-candidate probe near `3e10`: `267.3 ms +/- 4.3 ms`.
- `/usr/bin/time` 1,000,000-candidate probe near `3e10`: `wall=0.27 rss_kb=1760`.

Promotion checks after removing version suffixes:

```text
make -f Makefile clean
make -f Makefile
./a029483 --self-test
make -f Makefile test
```

Results:

- Final build: passed, zero warnings.
- Final self-test: passed.
- Final Makefile regression suite: passed.

Final status:

**READY FOR PRODUCTION**
