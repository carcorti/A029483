#!/usr/bin/env python3
"""Independent certificate checker for OEIS A029483.

A029483 contains k such that k divides the left concatenation of 1..k in base
14.  This script does not reuse the C implementation's divide-and-conquer
series representation.  Instead, each digit-length block is evaluated by
raising a 3x3 affine transition matrix modulo k.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable


BASE = 14

KNOWN_TERMS = [
    1,
    13,
    143,
    169,
    221,
    403,
    587,
    11219,
    178357,
    222157,
    85762339,
    1086336563,
    8005332049,
    15081597011,
]

NEW_TERMS = [
    44445214853,
    243987299933,
]


Matrix = tuple[tuple[int, int, int], tuple[int, int, int], tuple[int, int, int]]
Vector = tuple[int, int, int]


def mat_mul(a: Matrix, b: Matrix, mod: int) -> Matrix:
    return tuple(
        tuple(sum(a[i][r] * b[r][j] for r in range(3)) % mod for j in range(3))
        for i in range(3)
    )  # type: ignore[return-value]


def mat_pow(m: Matrix, exp: int, mod: int) -> Matrix:
    result: Matrix = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    base = m
    while exp:
        if exp & 1:
            result = mat_mul(base, result, mod)
        exp >>= 1
        if exp:
            base = mat_mul(base, base, mod)
    return result


def mat_vec_mul(m: Matrix, v: Vector, mod: int) -> Vector:
    return tuple(sum(m[i][j] * v[j] for j in range(3)) % mod for i in range(3))  # type: ignore[return-value]


def block_sum_mod(block_start: int, count: int, q: int, mod: int) -> int:
    """Return sum_{i=0..count-1} (block_start+i) * q^i mod mod.

    The transition is applied to [p_i, i*p_i, acc_i]:

        p'    = q*p
        ip'   = q*p + q*ip
        acc'  = acc + block_start*p + ip

    Starting from [1, 0, 0], the accumulator after count steps is the desired
    arithmetic-geometric sum.  This is intentionally independent of the C
    program's recursive series-combination implementation.
    """

    if mod == 1 or count == 0:
        return 0

    a = block_start % mod
    q %= mod
    transition: Matrix = (
        (q, 0, 0),
        (q, q, 0),
        (a, 1, 1),
    )
    _, _, acc = mat_vec_mul(mat_pow(transition, count, mod), (1, 0, 0), mod)
    return acc


def left_concat_residue(k: int, base: int = BASE) -> int:
    """Return the base-left-concatenation of 1..k modulo k."""

    if k <= 0:
        raise ValueError("k must be positive")
    if k == 1:
        return 0

    residue = 0
    prefix_digits = 0
    block_start = 1
    next_power = base
    digit_len = 1

    while block_start <= k:
        block_end = min(k, next_power - 1)
        count = block_end - block_start + 1
        q = pow(base, digit_len, k)
        block = block_sum_mod(block_start, count, q, k)
        shift = pow(base, prefix_digits, k)
        residue = (residue + shift * block) % k

        if block_end == k:
            break

        prefix_digits += digit_len * count
        block_start = next_power
        next_power *= base
        digit_len += 1

    return residue


def is_term(k: int) -> bool:
    return left_concat_residue(k) == 0


def parse_values(paths: Iterable[Path]) -> list[int]:
    values: list[int] = []
    for path in paths:
        for line in path.read_text(encoding="utf-8").splitlines():
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            values.append(int(s.split()[0]))
    return values


def direct_left_concat_residue(k: int, base: int = BASE) -> int:
    """Slow literal checker for small k only."""

    digits: list[int] = []
    for n in range(1, k + 1):
        x = n
        nd: list[int] = []
        while x:
            nd.append(x % base)
            x //= base
        digits = list(reversed(nd or [0])) + digits

    value = 0
    for d in digits:
        value = (value * base + d) % k
    return value


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("values", nargs="*", type=int, help="explicit k values to certify")
    parser.add_argument("--known", action="store_true", help="include the 14 OEIS terms")
    parser.add_argument("--new", action="store_true", help="include locally discovered terms")
    parser.add_argument("--file", action="append", type=Path, default=[], help="read k values from a text file")
    parser.add_argument("--small-direct-limit", type=int, default=5000,
                        help="also run literal big-int/digit check for k up to this value")
    args = parser.parse_args()

    values: list[int] = []
    if args.known:
        values.extend(KNOWN_TERMS)
    if args.new:
        values.extend(NEW_TERMS)
    values.extend(parse_values(args.file))
    values.extend(args.values)

    if not values:
        values = KNOWN_TERMS + NEW_TERMS

    ok = True
    seen: set[int] = set()
    for k in values:
        if k in seen:
            continue
        seen.add(k)

        residue = left_concat_residue(k)
        term = residue == 0
        direct = "n/a"
        if k <= args.small_direct_limit:
            direct_residue = direct_left_concat_residue(k)
            direct = str(direct_residue)
            if direct_residue != residue:
                ok = False
                print(f"k={k}: MISMATCH matrix={residue} direct={direct_residue}")
                continue

        status = "PASS" if term else "FAIL"
        if not term:
            ok = False
        print(f"k={k}: residue={residue} direct_residue={direct} {status}")

    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
