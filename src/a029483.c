#define _POSIX_C_SOURCE 200809L

/*
 * A029483 search production source
 *
 * Numbers k such that k divides the left concatenation of 1..k in base 14.
 *
 * The implementation evaluates each k independently by digit-length blocks:
 *
 *   C_b(k) = sum_{n=1..k} n * b^{L_b(n-1)} (mod k), b=14.
 *
 * No huge concatenated integer is ever built.  This makes block-based and
 * resumed runs straightforward: every candidate k can be tested in isolation.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BASE 14ULL
#define DEFAULT_BLOCK_SIZE 1000000ULL
#define DEFAULT_PROGRESS_EVERY 1ULL
#define WORK_CHUNK 4096ULL

typedef unsigned __int128 u128;

typedef struct {
    uint64_t pow_n;  /* q^n */
    uint64_t s0;     /* sum_{r=0}^{n-1} q^r */
    uint64_t s1;     /* sum_{r=0}^{n-1} r*q^r */
} series_t;

typedef struct {
    uint64_t start;
    uint64_t end;
    uint64_t block_size;
    uint64_t progress_every;
    int threads;
    const char *out_path;
    const char *checkpoint_path;
    const char *resume_path;
    bool self_test;
    bool quiet;
} options_t;

typedef struct {
    uint64_t raw;
    uint64_t survivors;
    uint64_t hits;
} counters_t;

typedef struct {
    uint64_t *values;
    size_t count;
    size_t capacity;
} hit_set_t;

typedef struct {
    uint64_t lo;
    uint64_t count;
    uint64_t next_index;
    FILE *out;
    hit_set_t *hit_set;
    counters_t total;
    atomic_bool had_error;
    pthread_mutex_t work_mutex;
    pthread_mutex_t hit_mutex;
    pthread_mutex_t counter_mutex;
} scan_context_t;

static uint64_t mod_u128(u128 x, uint64_t mod) {
    if (mod == 1) {
        return 0;
    }
    return (uint64_t)(x % (u128)mod);
}

static uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t mod) {
    if (mod == 1) {
        return 0;
    }
    return (uint64_t)(((u128)a * (u128)b) % (u128)mod);
}

static uint64_t add_mod(uint64_t a, uint64_t b, uint64_t mod) {
    if (mod == 1) {
        return 0;
    }

    u128 sum = (u128)a + (u128)b;
    if (sum >= (u128)mod) {
        sum -= (u128)mod;
    }
    return (uint64_t)sum;
}

static uint64_t pow_mod_u128_exp(uint64_t base, u128 exp, uint64_t mod) {
    if (mod == 1) {
        return 0;
    }

    uint64_t result = 1 % mod;
    uint64_t x = base % mod;

    while (exp != 0) {
        if ((exp & 1) != 0) {
            result = mul_mod(result, x, mod);
        }
        exp >>= 1;
        if (exp != 0) {
            x = mul_mod(x, x, mod);
        }
    }

    return result;
}

static series_t combine_series(series_t left, uint64_t left_len,
                               series_t right, uint64_t mod) {
    series_t out;

    out.pow_n = mul_mod(left.pow_n, right.pow_n, mod);
    out.s0 = add_mod(left.s0, mul_mod(left.pow_n, right.s0, mod), mod);

    uint64_t left_len_mod = (mod == 1) ? 0 : (left_len % mod);
    uint64_t shifted_right_s1 = add_mod(
        mul_mod(left_len_mod, right.s0, mod),
        right.s1,
        mod
    );
    out.s1 = add_mod(left.s1, mul_mod(left.pow_n, shifted_right_s1, mod), mod);

    return out;
}

static series_t geometric_arithmetic_series(uint64_t q, uint64_t n,
                                            uint64_t mod) {
    if (mod == 1) {
        series_t z = {0, 0, 0};
        return z;
    }

    if (n == 0) {
        series_t z = {1 % mod, 0, 0};
        return z;
    }

    if (n == 1) {
        series_t one = {q % mod, 1 % mod, 0};
        return one;
    }

    if ((n & 1ULL) == 0) {
        uint64_t half_len = n / 2;
        series_t half = geometric_arithmetic_series(q, half_len, mod);
        return combine_series(half, half_len, half, mod);
    }

    series_t prev = geometric_arithmetic_series(q, n - 1, mod);
    uint64_t n_minus_1_mod = (n - 1) % mod;

    series_t out;
    out.pow_n = mul_mod(prev.pow_n, q % mod, mod);
    out.s0 = add_mod(prev.s0, prev.pow_n, mod);
    out.s1 = add_mod(prev.s1, mul_mod(n_minus_1_mod, prev.pow_n, mod), mod);
    return out;
}

static bool is_a029483_term(uint64_t k) {
    if (k == 1) {
        return true;
    }

    /*
     * The rightmost base-14 digit of the left concatenation is always the
     * final digit of 1. Thus the concatenated value is 1 mod 2 and 1 mod 7,
     * so no even k and no multiple of 7 can divide it.
     */
    if ((k % 2) == 0 || (k % 7) == 0) {
        return false;
    }

    uint64_t residue = 0;
    u128 prefix_digits = 0;
    u128 block_start = 1;
    u128 next_power = BASE;

    for (uint64_t digit_len = 1; block_start <= (u128)k; digit_len++) {
        u128 block_end = next_power - 1;
        u128 upper = ((u128)k < block_end) ? (u128)k : block_end;
        uint64_t n_terms = (uint64_t)(upper - block_start + 1);

        uint64_t q = pow_mod_u128_exp(BASE, digit_len, k);
        series_t s = geometric_arithmetic_series(q, n_terms, k);

        uint64_t a_mod = mod_u128(block_start, k);
        uint64_t linear_sum = add_mod(mul_mod(a_mod, s.s0, k), s.s1, k);
        uint64_t shift = pow_mod_u128_exp(BASE, prefix_digits, k);
        residue = add_mod(residue, mul_mod(shift, linear_sum, k), k);

        if ((u128)k <= block_end) {
            break;
        }

        prefix_digits += (u128)digit_len * (block_end - block_start + 1);
        block_start = next_power;
        next_power *= BASE;
    }

    return residue == 0;
}

static bool parse_u64(const char *s, uint64_t *out) {
    if (s == NULL) {
        return false;
    }

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0' || *s == '-' || *s == '+') {
        return false;
    }

    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);

    if (errno != 0 || end == s || *end != '\0') {
        return false;
    }

    *out = (uint64_t)v;
    return true;
}

static bool parse_hit_line(const char *line, uint64_t *out) {
    const char *p = line;
    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '\0' || *p == '-' || *p == '+') {
        return false;
    }

    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(p, &end, 10);
    if (errno != 0 || end == p || v == 0) {
        return false;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return false;
    }

    *out = (uint64_t)v;
    return true;
}

static bool hit_set_contains(const hit_set_t *set, uint64_t value) {
    for (size_t i = 0; i < set->count; i++) {
        if (set->values[i] == value) {
            return true;
        }
    }
    return false;
}

static bool hit_set_add(hit_set_t *set, uint64_t value) {
    if (hit_set_contains(set, value)) {
        return true;
    }

    if (set->count == set->capacity) {
        size_t new_capacity = set->capacity == 0 ? 16 : set->capacity * 2;
        uint64_t *new_values = realloc(set->values, new_capacity * sizeof(*new_values));
        if (new_values == NULL) {
            return false;
        }
        set->values = new_values;
        set->capacity = new_capacity;
    }

    set->values[set->count++] = value;
    return true;
}

static void hit_set_free(hit_set_t *set) {
    free(set->values);
    set->values = NULL;
    set->count = 0;
    set->capacity = 0;
}

static bool load_existing_hits(const char *path, hit_set_t *set) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        if (errno == ENOENT) {
            return true;
        }
        fprintf(stderr, "error: cannot read existing hits '%s': %s\n",
                path, strerror(errno));
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        uint64_t value;
        if (parse_hit_line(line, &value)) {
            if (!hit_set_add(set, value)) {
                fclose(f);
                fprintf(stderr, "error: out of memory while loading hits\n");
                return false;
            }
        }
    }

    fclose(f);
    return true;
}

static bool fsync_parent_dir(const char *path) {
    const char *slash = strrchr(path, '/');
    char dir_path[PATH_MAX];

    if (slash == NULL) {
        snprintf(dir_path, sizeof(dir_path), ".");
    } else {
        size_t len = (size_t)(slash - path);
        if (len == 0) {
            len = 1;
        }
        if (len >= sizeof(dir_path)) {
            errno = ENAMETOOLONG;
            return false;
        }
        memcpy(dir_path, path, len);
        dir_path[len] = '\0';
    }

    int fd = open(dir_path, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        return false;
    }

    bool ok = fsync(fd) == 0;
    int saved_errno = errno;
    close(fd);
    errno = saved_errno;
    return ok;
}

static void print_usage(const char *argv0) {
    fprintf(stderr,
        "Usage:\n"
        "  %s --self-test\n"
        "  %s --start N --end M [options]\n\n"
        "Options:\n"
        "  --block-size N       Raw integer interval per segment (default: %" PRIu64 ")\n"
        "  --threads N          Worker threads; 0 means auto (default: online CPU count)\n"
        "  --out PATH           Append hits to PATH (default: results/a029483_hits.tsv)\n"
        "  --checkpoint PATH    Write latest completed segment to PATH\n"
        "  --resume PATH        Read next start from checkpoint PATH\n"
        "  --progress-every N   Print one progress line every N segments (default: 1)\n"
        "  --quiet              Suppress progress lines\n\n"
        "For long production runs, choose --block-size so each segment lasts\n"
        "about 90 minutes or less on the target machine.\n",
        argv0, argv0, (uint64_t)DEFAULT_BLOCK_SIZE);
}

static bool read_checkpoint_next(const char *path, uint64_t *next_start) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "error: cannot open checkpoint '%s': %s\n",
                path, strerror(errno));
        return false;
    }

    char line[256];
    bool ok = false;
    while (fgets(line, sizeof(line), f) != NULL) {
        uint64_t v;
        if (sscanf(line, "next_start=%" SCNu64, &v) == 1) {
            *next_start = v;
            ok = true;
        }
    }

    fclose(f);

    if (!ok) {
        fprintf(stderr, "error: checkpoint '%s' does not contain next_start\n", path);
    }
    return ok;
}

static bool write_checkpoint(const char *path, uint64_t next_start,
                             uint64_t completed_end, counters_t total,
                             double elapsed_seconds) {
    if (path == NULL) {
        return true;
    }

    size_t tmp_len = strlen(path) + 5;
    char *tmp_path = malloc(tmp_len);
    if (tmp_path == NULL) {
        fprintf(stderr, "error: out of memory while preparing checkpoint path\n");
        return false;
    }
    snprintf(tmp_path, tmp_len, "%s.tmp", path);

    FILE *f = fopen(tmp_path, "w");
    if (f == NULL) {
        fprintf(stderr, "error: cannot write checkpoint temp '%s': %s\n",
                tmp_path, strerror(errno));
        free(tmp_path);
        return false;
    }

    time_t now = time(NULL);
    fprintf(f, "sequence=A029483\n");
    fprintf(f, "timestamp=%lld\n", (long long)now);
    fprintf(f, "completed_end=%" PRIu64 "\n", completed_end);
    fprintf(f, "next_start=%" PRIu64 "\n", next_start);
    fprintf(f, "raw_tested=%" PRIu64 "\n", total.raw);
    fprintf(f, "survivors_tested=%" PRIu64 "\n", total.survivors);
    fprintf(f, "hits=%" PRIu64 "\n", total.hits);
    fprintf(f, "elapsed_seconds=%.3f\n", elapsed_seconds);

    if (fflush(f) != 0 || fsync(fileno(f)) != 0) {
        fprintf(stderr, "error: cannot flush checkpoint temp '%s': %s\n",
                tmp_path, strerror(errno));
        fclose(f);
        unlink(tmp_path);
        free(tmp_path);
        return false;
    }

    if (fclose(f) != 0) {
        fprintf(stderr, "error: cannot close checkpoint temp '%s': %s\n",
                tmp_path, strerror(errno));
        unlink(tmp_path);
        free(tmp_path);
        return false;
    }

    if (rename(tmp_path, path) != 0) {
        fprintf(stderr, "error: cannot promote checkpoint '%s' -> '%s': %s\n",
                tmp_path, path, strerror(errno));
        unlink(tmp_path);
        free(tmp_path);
        return false;
    }

    if (!fsync_parent_dir(path)) {
        fprintf(stderr, "warning: cannot fsync checkpoint parent directory for '%s': %s\n",
                path, strerror(errno));
    }

    free(tmp_path);
    return true;
}

static bool parse_options(int argc, char **argv, options_t *opt) {
    memset(opt, 0, sizeof(*opt));
    opt->start = 0;
    opt->end = 0;
    opt->block_size = DEFAULT_BLOCK_SIZE;
    opt->progress_every = DEFAULT_PROGRESS_EVERY;
    opt->threads = 0;
    opt->out_path = "results/a029483_hits.tsv";
    opt->checkpoint_path = "results/a029483_checkpoint.txt";
    bool saw_start = false;
    bool saw_end = false;
    bool self_test_conflict = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--self-test") == 0) {
            opt->self_test = true;
        } else if (strcmp(arg, "--quiet") == 0) {
            opt->quiet = true;
        } else if (strcmp(arg, "--start") == 0 && i + 1 < argc) {
            if (!parse_u64(argv[++i], &opt->start)) {
                fprintf(stderr, "error: invalid --start value\n");
                return false;
            }
            saw_start = true;
            self_test_conflict = true;
        } else if (strcmp(arg, "--end") == 0 && i + 1 < argc) {
            if (!parse_u64(argv[++i], &opt->end)) {
                fprintf(stderr, "error: invalid --end value\n");
                return false;
            }
            saw_end = true;
            self_test_conflict = true;
        } else if (strcmp(arg, "--block-size") == 0 && i + 1 < argc) {
            if (!parse_u64(argv[++i], &opt->block_size) || opt->block_size == 0) {
                fprintf(stderr, "error: invalid --block-size value\n");
                return false;
            }
            self_test_conflict = true;
        } else if (strcmp(arg, "--progress-every") == 0 && i + 1 < argc) {
            if (!parse_u64(argv[++i], &opt->progress_every) || opt->progress_every == 0) {
                fprintf(stderr, "error: invalid --progress-every value\n");
                return false;
            }
            self_test_conflict = true;
        } else if (strcmp(arg, "--threads") == 0 && i + 1 < argc) {
            uint64_t t;
            if (!parse_u64(argv[++i], &t) || t > 1024) {
                fprintf(stderr, "error: invalid --threads value\n");
                return false;
            }
            opt->threads = (int)t;
            self_test_conflict = true;
        } else if (strcmp(arg, "--out") == 0 && i + 1 < argc) {
            opt->out_path = argv[++i];
            self_test_conflict = true;
        } else if (strcmp(arg, "--checkpoint") == 0 && i + 1 < argc) {
            opt->checkpoint_path = argv[++i];
            self_test_conflict = true;
        } else if (strcmp(arg, "--resume") == 0 && i + 1 < argc) {
            opt->resume_path = argv[++i];
            self_test_conflict = true;
        } else {
            fprintf(stderr, "error: unknown or incomplete option '%s'\n", arg);
            return false;
        }
    }

    if (opt->self_test && self_test_conflict) {
        fprintf(stderr, "error: --self-test cannot be combined with run options\n");
        return false;
    }

    if (saw_start && opt->resume_path != NULL) {
        fprintf(stderr, "error: --start cannot be combined with --resume\n");
        return false;
    }

    if (!opt->self_test && opt->end == 0) {
        if (saw_end) {
            fprintf(stderr, "error: --end must be a positive integer\n");
        } else {
            fprintf(stderr, "error: either --self-test or --end is required\n");
        }
        return false;
    }

    if (!opt->self_test && opt->start == 0 && opt->resume_path == NULL) {
        if (saw_start) {
            fprintf(stderr, "error: --start must be a positive integer\n");
        } else {
            fprintf(stderr, "error: --start is required unless --resume is used\n");
        }
        return false;
    }

    if (opt->resume_path != NULL && !read_checkpoint_next(opt->resume_path, &opt->start)) {
        return false;
    }

    if (!opt->self_test && opt->resume_path != NULL && opt->start == 0) {
        fprintf(stderr, "error: checkpoint next_start must be a positive integer\n");
        return false;
    }

    if (!opt->self_test && opt->start > opt->end) {
        fprintf(stderr, "error: --start must be <= --end\n");
        return false;
    }

    if (!opt->self_test && strcmp(opt->out_path, opt->checkpoint_path) == 0) {
        fprintf(stderr, "error: --out and --checkpoint must be different files\n");
        return false;
    }

    return true;
}

static bool write_hit(FILE *out, uint64_t k) {
    if (fprintf(out, "%" PRIu64 "\n", k) < 0) {
        return false;
    }
    if (fflush(out) != 0) {
        return false;
    }
    return fsync(fileno(out)) == 0;
}

static void *scan_worker(void *arg) {
    scan_context_t *ctx = arg;
    counters_t local = {0, 0, 0};

    for (;;) {
        pthread_mutex_lock(&ctx->work_mutex);
        if (atomic_load(&ctx->had_error)) {
            pthread_mutex_unlock(&ctx->work_mutex);
            break;
        }
        uint64_t begin = ctx->next_index;
        if (begin >= ctx->count) {
            pthread_mutex_unlock(&ctx->work_mutex);
            break;
        }
        uint64_t end = begin + WORK_CHUNK;
        if (end > ctx->count || end < begin) {
            end = ctx->count;
        }
        ctx->next_index = end;
        pthread_mutex_unlock(&ctx->work_mutex);

        for (uint64_t i = begin; i < end; i++) {
            uint64_t k = ctx->lo + i;
            local.raw++;

            if (k != 1 && ((k % 2) == 0 || (k % 7) == 0)) {
                continue;
            }

            local.survivors++;

            if (is_a029483_term(k)) {
                bool wrote_hit = false;

                pthread_mutex_lock(&ctx->hit_mutex);
                if (!hit_set_contains(ctx->hit_set, k)) {
                    if (!hit_set_add(ctx->hit_set, k)) {
                        fprintf(stderr, "error: out of memory while recording hit %" PRIu64 "\n", k);
                        atomic_store(&ctx->had_error, true);
                        pthread_mutex_unlock(&ctx->hit_mutex);
                        goto done;
                    }
                    if (!write_hit(ctx->out, k)) {
                        fprintf(stderr, "error: failed while writing hit %" PRIu64 "\n", k);
                        atomic_store(&ctx->had_error, true);
                        pthread_mutex_unlock(&ctx->hit_mutex);
                        goto done;
                    }
                    wrote_hit = true;
                }
                pthread_mutex_unlock(&ctx->hit_mutex);

                if (wrote_hit) {
                    local.hits++;
                }
            }
        }
    }

done:
    pthread_mutex_lock(&ctx->counter_mutex);
    ctx->total.raw += local.raw;
    ctx->total.survivors += local.survivors;
    ctx->total.hits += local.hits;
    pthread_mutex_unlock(&ctx->counter_mutex);

    return NULL;
}

static int default_thread_count(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 1) {
        return 1;
    }
    if (n > 1024) {
        return 1024;
    }
    return (int)n;
}

static void destroy_scan_mutexes(scan_context_t *ctx) {
    pthread_mutex_destroy(&ctx->counter_mutex);
    pthread_mutex_destroy(&ctx->hit_mutex);
    pthread_mutex_destroy(&ctx->work_mutex);
}

static bool scan_segment(uint64_t lo, uint64_t hi, FILE *out,
                         hit_set_t *hit_set, int threads, counters_t *result) {
    scan_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.lo = lo;
    ctx.count = hi - lo + 1;
    ctx.out = out;
    ctx.hit_set = hit_set;
    atomic_init(&ctx.had_error, false);
    *result = ctx.total;

    if (threads < 1) {
        threads = default_thread_count();
    }

    int rc = pthread_mutex_init(&ctx.work_mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "error: pthread_mutex_init failed: %s\n", strerror(rc));
        return false;
    }
    rc = pthread_mutex_init(&ctx.hit_mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "error: pthread_mutex_init failed: %s\n", strerror(rc));
        pthread_mutex_destroy(&ctx.work_mutex);
        return false;
    }
    rc = pthread_mutex_init(&ctx.counter_mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "error: pthread_mutex_init failed: %s\n", strerror(rc));
        pthread_mutex_destroy(&ctx.hit_mutex);
        pthread_mutex_destroy(&ctx.work_mutex);
        return false;
    }

    if (threads == 1) {
        scan_worker(&ctx);
    } else {
        pthread_t *workers = calloc((size_t)threads, sizeof(*workers));
        if (workers == NULL) {
            fprintf(stderr, "error: cannot allocate worker thread handles\n");
            destroy_scan_mutexes(&ctx);
            return false;
        }

        int created = 0;
        for (int i = 0; i < threads; i++) {
            rc = pthread_create(&workers[i], NULL, scan_worker, &ctx);
            if (rc != 0) {
                fprintf(stderr, "error: pthread_create failed: %s\n", strerror(rc));
                atomic_store(&ctx.had_error, true);
                break;
            }
            created++;
        }

        for (int i = 0; i < created; i++) {
            rc = pthread_join(workers[i], NULL);
            if (rc != 0) {
                fprintf(stderr, "error: pthread_join failed: %s\n", strerror(rc));
                atomic_store(&ctx.had_error, true);
            }
        }

        free(workers);
    }

    *result = ctx.total;
    bool ok = !atomic_load(&ctx.had_error);
    destroy_scan_mutexes(&ctx);

    return ok;
}

static bool run_self_test(void) {
    static const uint64_t known[] = {
        1ULL, 13ULL, 143ULL, 169ULL, 221ULL, 403ULL, 587ULL,
        11219ULL, 178357ULL, 222157ULL, 85762339ULL,
        1086336563ULL, 8005332049ULL, 15081597011ULL
    };
    static const uint64_t expected_prefix[] = {
        1ULL, 13ULL, 143ULL, 169ULL, 221ULL, 403ULL, 587ULL,
        11219ULL, 178357ULL, 222157ULL
    };

    size_t known_count = sizeof(known) / sizeof(known[0]);
    for (size_t i = 0; i < known_count; i++) {
        if (!is_a029483_term(known[i])) {
            fprintf(stderr, "self-test failed: known term %" PRIu64 " rejected\n", known[i]);
            return false;
        }
    }

    size_t found = 0;
    for (uint64_t k = 1; k <= 250000; k++) {
        bool is_term = is_a029483_term(k);
        bool expected = (found < sizeof(expected_prefix) / sizeof(expected_prefix[0]) &&
                         k == expected_prefix[found]);

        if (is_term != expected) {
            fprintf(stderr, "self-test failed at k=%" PRIu64
                    ": observed=%d expected=%d\n", k, is_term ? 1 : 0,
                    expected ? 1 : 0);
            return false;
        }

        if (expected) {
            found++;
        }
    }

    if (found != sizeof(expected_prefix) / sizeof(expected_prefix[0])) {
        fprintf(stderr, "self-test failed: prefix found count mismatch\n");
        return false;
    }

    printf("self-test ok: known terms accepted and prefix through 250000 verified\n");
    return true;
}

static double monotonic_seconds(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return (double)clock() / (double)CLOCKS_PER_SEC;
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

int main(int argc, char **argv) {
    options_t opt;

    if (!parse_options(argc, argv, &opt)) {
        print_usage(argv[0]);
        return 2;
    }

    if (opt.self_test) {
        return run_self_test() ? 0 : 1;
    }

    hit_set_t hit_set = {0};

    if (!load_existing_hits(opt.out_path, &hit_set)) {
        return 2;
    }

    FILE *out = fopen(opt.out_path, "a");
    if (out == NULL) {
        fprintf(stderr, "error: cannot open output '%s': %s\n",
                opt.out_path, strerror(errno));
        hit_set_free(&hit_set);
        return 2;
    }

    if (!fsync_parent_dir(opt.out_path)) {
        fprintf(stderr, "warning: cannot fsync output parent directory for '%s': %s\n",
                opt.out_path, strerror(errno));
    }

    counters_t total = {0, 0, (uint64_t)hit_set.count};
    uint64_t block_index = 0;
    double t0 = monotonic_seconds();

    for (uint64_t lo = opt.start; lo <= opt.end; ) {
        uint64_t remaining = opt.end - lo + 1;
        uint64_t span = remaining < opt.block_size ? remaining : opt.block_size;
        uint64_t hi = lo + span - 1;

        double b0 = monotonic_seconds();
        counters_t block;
        if (!scan_segment(lo, hi, out, &hit_set, opt.threads, &block)) {
            if (fclose(out) != 0) {
                fprintf(stderr, "error: closing output '%s': %s\n",
                        opt.out_path, strerror(errno));
            }
            hit_set_free(&hit_set);
            return 3;
        }
        double b1 = monotonic_seconds();

        total.raw += block.raw;
        total.survivors += block.survivors;
        total.hits += block.hits;
        block_index++;

        uint64_t next_start = (hi == UINT64_MAX) ? UINT64_MAX : hi + 1;
        double elapsed = monotonic_seconds() - t0;

        if (!write_checkpoint(opt.checkpoint_path, next_start, hi, total, elapsed)) {
            if (fclose(out) != 0) {
                fprintf(stderr, "error: closing output '%s': %s\n",
                        opt.out_path, strerror(errno));
            }
            hit_set_free(&hit_set);
            return 3;
        }

        if (!opt.quiet && (block_index % opt.progress_every) == 0) {
            double block_seconds = b1 - b0;
            double raw_per_sec = block_seconds > 0.0 ?
                (double)block.raw / block_seconds : 0.0;
            fprintf(stderr,
                    "segment=%" PRIu64 " range=[%" PRIu64 ",%" PRIu64 "] "
                    "raw=%" PRIu64 " survivors=%" PRIu64 " hits=%" PRIu64
                    " block_sec=%.3f raw/sec=%.3f total_hits=%" PRIu64 "\n",
                    block_index, lo, hi, block.raw, block.survivors, block.hits,
                    block_seconds, raw_per_sec, total.hits);
        }

        if (hi == opt.end || hi == UINT64_MAX) {
            break;
        }
        lo = hi + 1;
    }

    if (fclose(out) != 0) {
        fprintf(stderr, "error: closing output '%s': %s\n",
                opt.out_path, strerror(errno));
        hit_set_free(&hit_set);
        return 3;
    }
    hit_set_free(&hit_set);

    if (!opt.quiet) {
        double elapsed = monotonic_seconds() - t0;
        fprintf(stderr,
                "done raw=%" PRIu64 " survivors=%" PRIu64 " hits=%" PRIu64
                " elapsed_sec=%.3f\n",
                total.raw, total.survivors, total.hits, elapsed);
    }

    return 0;
}
