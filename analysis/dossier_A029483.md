# Dossier - OEIS A029483

**Date:** 2026-05-29
**Framework:** Carlo's OEIS workflow, Phase 0 + Phase 1
**Hardware target:** MinisForum UM790 Pro (Ryzen 9 7940HS, 64 GB DDR5, 16 threads)
**Input files:** `01_INPUT_TEMPLATE.md` (DeepSeek triage), `A029483_OEIS.md` (OEIS home page snapshot)

## 0. Input summary

### 0.1 OEIS home page (key facts)

[OEIS] A029483 is the sequence of numbers \(k\) that divide the left concatenation of all numbers \(\le k\) written in base 14, with the most significant digit on the left.

[OEIS] The known terms in the supplied snapshot are:

\[
1, 13, 143, 169, 221, 403, 587, 11219, 178357, 222157,
85762339, 1086336563, 8005332049, 15081597011.
\]

[OEIS] The snapshot states: "No other terms below \(3 \cdot 10^{10}\)."

[OEIS] The offset is \(1,2\). The keyword line includes `nonn,base,more`.

[OEIS] The page gives a Mathematica program using base \(b=14\), with the list of digits of the current integer prepended to the current concatenation.

[OEIS] Cross-references place A029483 in the family A029447-A029542 and A061931-A061978, covering right/left concatenations and digit-direction variants across bases.

[OEIS] The author is Olivier Gerard. The supplied snapshot records extensions by Larry Reeves, Max Alekseyev, and Jason Yuen, with \(a(12)-a(14)\) attributed to Jason Yuen on Jun 04 2024.

### 0.2 DeepSeek triage verdict

[PLAUSIBLE] The triage classifies the problem as a rare-term search with irregular growth, but then labels the computational type as Type B, deterministic table extension.

[PLAUSIBLE] The proposed computational core is a scan over \(k\), testing whether the base-14 left concatenation is congruent to zero modulo \(k\).

[PLAUSIBLE] The triage expects little RAM pressure, high parallelizability, and wall-clock time in the broad class 1-24 h for ranges around or somewhat above \(3 \cdot 10^{10}\), subject to a real benchmark.

[TO-VERIFY] The triage raises five checks: the provenance of the \(3 \cdot 10^{10}\) bound, the numeric type threshold, inner-loop throughput, parallel initialization, and empirical hit probability.

### 0.3 Consistency check

[OEIS] The definition, known terms, and stated bound in the triage match the supplied OEIS snapshot.

[PLAUSIBLE] There is no conflict between the two inputs on the mathematical definition or the known values.

[TO-VERIFY] There is a classification issue for the convenience index. The triage labels the task Type B, but Carlo's framework defines Type A as a term-hunting campaign where the outcome is either a new term or a certified bound extension. A029483 fits Type A better than Type B once the target is "search beyond \(3 \cdot 10^{10}\)".

[PLAUSIBLE] The triage's statement that GMP may be needed above \(2^{32}\) is too conservative for the proposed ranges. For \(k < 2^{64}\), residues fit in `uint64_t`; modular multiplication should use a wider intermediate such as `__uint128_t`. GMP is not needed for campaigns up to \(10^{12}\).

### 0.4 Bibliographic search

The following queries were authorized by Carlo and run on 2026-05-29:

- `"A029483" concatenation base 14 divisibility`
- `"Numbers k that divide the left concatenation" "base 14"`
- `"Olivier Gerard" "Larry Reeves" "A029483"`
- `"Jason Yuen" "A029483" "No other terms below"`
- `"concatenation of all numbers <= k" divisibility OEIS`

Search outcome:

- [CANDIDATE-REFERENCE] OEIS index page for "n divides concatenation of all numbers up through n", listing A029483 and adjacent family members: https://oeis.org/wiki/Index_to_OEIS%3A_Section_N
- [CANDIDATE-REFERENCE] OEIS A029480, adjacent base-11 left-concatenation sequence, with related recent extension/bound comments: https://oeis.org/A029480
- [CANDIDATE-REFERENCE] OEIS A029481, adjacent base-12 left-concatenation sequence, with a similar \(3 \cdot 10^{10}\) bound: https://oeis.org/A029481
- [CANDIDATE-REFERENCE] OEIS A029484, adjacent base-15 left-concatenation sequence, with a similar \(3 \cdot 10^{10}\) bound and one modular exclusion comment for that base: https://oeis.org/A029484
- [CANDIDATE-REFERENCE] OEIS A029485, adjacent base-16 left-concatenation sequence, with a similar \(3 \cdot 10^{10}\) bound: https://oeis.org/A029485

[TO-VERIFY] No non-OEIS paper, preprint, or independent algorithmic write-up specific to A029483 was found in this search round.

[TO-VERIFY] Carlo did not provide private, unpublished, or manually collected literature in this turn. This remains open before Phase 2 if such material exists.

No candidate reference above is integrated as a confirmed mathematical source in the analysis below.

## 1. Phase 0 - Fundamental mathematical analysis

### 1.1 Formal definition

[OEIS] The base is \(b=14\).

[PLAUSIBLE] Let

\[
\ell_b(n)=1+\lfloor \log_b n \rfloor
\]

be the number of base-\(b\) digits of \(n\), and let

\[
L_b(n)=\sum_{j=1}^{n}\ell_b(j), \qquad L_b(0)=0.
\]

The left concatenation of \(1,2,\ldots,k\) in base \(b\), with the current integer prepended as in the OEIS Mathematica program, is

\[
C_b(k)=\sum_{n=1}^{k} n b^{L_b(n-1)}.
\]

Thus

\[
A029483=\{k\ge 1 : C_{14}(k)\equiv 0 \pmod{k}\}.
\]

[PLAUSIBLE] This formula is computationally useful because it allows \(C_{14}(k)\bmod k\) to be evaluated without constructing \(C_{14}(k)\).

For fixed digit length \(d\), set \(a=14^{d-1}\), \(u=\min(k,14^d-1)\), \(p=14^d\), and \(s=L_{14}(a-1)\). The contribution of the block \(a\le n\le u\) is

\[
14^s \sum_{r=0}^{u-a} (a+r)p^r \pmod{k}.
\]

[PLAUSIBLE] The finite sums \(\sum p^r\) and \(\sum r p^r\) can be evaluated by binary splitting or doubling recurrences modulo \(k\), avoiding \(O(k)\) initialization per candidate.

### 1.2 Mathematical nature

[PLAUSIBLE] The sequence is nonlinear and not additive or multiplicative in \(k\).

[PLAUSIBLE] The defining predicate depends on base-14 digit lengths, modular powers of 14, and divisibility by the same integer \(k\) being tested.

[PLAUSIBLE] It is not a prime-only or factorization-only sequence. However, factor information about \(k\) can supply necessary congruence filters.

[PLAUSIBLE] The sequence is only weakly sieve-friendly. There are simple exclusions from common factors with the base, but no obvious dense residue-class sieve that would remove most candidates before the main modular computation.

### 1.3 Key arithmetic properties

[OEIS] No terms other than the listed terms occur below \(3\cdot 10^{10}\).

[OEIS] Many listed terms are divisible by 13, but not all; for example 587 and 178357 are listed terms and are not divisible by 13.

[PLAUSIBLE] Since \(14\equiv 0\pmod 2\) and \(14\equiv 0\pmod 7\), and the rightmost block of \(C_{14}(k)\) is the digit for 1, one has

\[
C_{14}(k)\equiv 1 \pmod 2,\qquad C_{14}(k)\equiv 1 \pmod 7.
\]

Therefore no \(k>1\) divisible by 2 or 7 can be a term.

[PLAUSIBLE] Since \(14\equiv 1\pmod{13}\),

\[
C_{14}(k)\equiv \sum_{n=1}^{k}n = \frac{k(k+1)}{2}\pmod{13}.
\]

This explains why divisibility by 13 is compatible with the predicate: if \(13\mid k\), the necessary congruence modulo 13 is automatically satisfied.

[PLAUSIBLE] Digit-length changes occur at powers of 14. Around the current bound:

\[
14^9=1475789056,\quad 14^{10}=20661046784,\quad 14^{11}=289254654976.
\]

Thus the interval \([3\cdot 10^{10}, 10^{11}]\) lies entirely in the 10-digit base-14 region, while a campaign to \(3\cdot 10^{11}\) crosses the \(14^{11}\) digit boundary.

[TO-VERIFY] There may be additional necessary conditions for candidates with small prime divisors \(q\nmid 14\), obtained by evaluating \(C_{14}(k)\bmod q\) when \(q\mid k\). Their pruning value needs measurement because factoring or sieving all candidates has its own cost.

### 1.4 Computational reduction

[PLAUSIBLE] The direct problem is:

1. Enumerate candidate \(k\) above the certified bound \(3\cdot 10^{10}\).
2. Skip \(k>1\) with \(2\mid k\) or \(7\mid k\).
3. Compute \(C_{14}(k)\bmod k\) using digit-block sums.
4. Report \(k\) exactly when the residue is zero.

[PLAUSIBLE] The expensive operation is not memory, sorting, I/O, or primality testing. The expensive operation is a very large number of modular multiplications and modular reductions.

[PLAUSIBLE] The block-sum reduction removes the triage's possible \(O(k)\) startup cost per parallel block. Each \(k\) can be tested independently, so a worker can start at any \(k\) without inheriting a state from previous moduli.

[TO-VERIFY] The exact best implementation of the block sums should be benchmarked: binary splitting, closed-form modular division when allowed, and recurrence doubling may have different costs depending on modulus parity and divisibility by \(p-1\).

### 1.5 Qualitative theoretical complexity

[PLAUSIBLE] For each candidate \(k\), the number of digit blocks is \(O(\log_{14} k)\).

[PLAUSIBLE] If block sums are evaluated with binary splitting, each block costs \(O(\log k)\) modular multiplications, giving a qualitative per-candidate cost of

\[
O((\log_{14} k)(\log k))
\]

modular operations.

[PLAUSIBLE] Memory is \(O(1)\) per worker, aside from output buffers and optional small-prime filter tables.

[PLAUSIBLE] Parallelism is embarrassingly simple after the independent block formula is used. The main practical constraints are integer division/modular-reduction throughput, branch behavior, and load balancing near digit-boundary ranges.

### 1.6 Mathematical pruning levers

[PLAUSIBLE] Hard exclusion: skip every \(k>1\) divisible by 2 or 7. This removes approximately \(4/7\) of all integers.

[PLAUSIBLE] Digit-block decomposition: precompute base-14 powers and prefix digit lengths for the small number of digit ranges relevant to the campaign.

[PLAUSIBLE] Small-prime necessary filters may be possible: if a small prime \(q\mid k\), then \(C_{14}(k)\equiv 0\pmod q\) is necessary. Testing whether such filters save time requires benchmarking against the overhead of sieving/factor tagging.

[PLAUSIBLE] Candidate partitioning by residue class modulo 14 is useful because only \(k\equiv 1,3,5,9,11,13\pmod{14}\) can survive the base-factor exclusion.

[TO-VERIFY] There is no known monotone bound or recurrence that predicts the next term without scanning.

### 1.7 Preliminary feasibility judgment

Mathematical judgment: **GO**.

[PLAUSIBLE] The problem has a clean modular formulation, negligible memory use, and strong parallel independence.

[PLAUSIBLE] The main uncertainty is not mathematical correctness but empirical throughput and hit probability beyond the current bound.

[TO-VERIFY] The search remains a rare-term hunt. A negative result up to a larger bound is still publishable as an OEIS extension only if the implementation is validated carefully.

## 2. Phase 1 - Computational strategy

### 2.1 Computational problem synthesis

The target computation is to extend the certified range for A029483 beyond \(3\cdot 10^{10}\), preferably to at least \(3\cdot 10^{11}\), while detecting and recording every \(k\) such that

\[
C_{14}(k)\equiv 0\pmod{k}.
\]

The recommended campaign is a Type A search: success may be either a new term or a larger certified no-missing-terms bound.

### 2.2 Recommended main algorithm

Recommended algorithm: **independent digit-block modular concatenation scan**.

For each candidate \(k\):

- reject \(k>1\) if \(k\equiv 0\pmod 2\) or \(k\equiv 0\pmod 7\);
- split \(1,\ldots,k\) into base-14 digit-length blocks;
- evaluate each block contribution

\[
14^s \sum_{r=0}^{u-a}(a+r)(14^d)^r \pmod{k};
\]

- sum the block residues modulo \(k\);
- accept \(k\) if the final residue is zero.

[PLAUSIBLE] This algorithm is preferable to maintaining a sequential concatenation state across \(k\), because the modulus changes at every candidate. The independent formula avoids long startup costs for parallel chunks.

### 2.3 Alternative strategies

Alternative 1: sequential per-\(k\) construction modulo \(k\) by iterating all \(n\le k\). This is simple and useful only as a small-range validator. It is not viable for \(k\sim 10^{10}\) or larger.

Alternative 2: incremental scans within a fixed modulus. This can validate one chosen \(k\), but it does not naturally scan all moduli because the modulus changes with the candidate.

Alternative 3: add a small-prime prefilter layer. This may be worthwhile if the filter rejects many candidates at low cost, but it should be optional until benchmarked.

### 2.4 Data structures and numeric types

[PLAUSIBLE] For campaign bounds up to \(10^{12}\), every candidate \(k\) and every residue modulo \(k\) fits in `uint64_t`.

[PLAUSIBLE] Modular multiplication should use a `__uint128_t` intermediate, because the product of two residues below \(10^{12}\) can be about \(10^{24}\), well above \(2^{64}\).

[PLAUSIBLE] GMP is unnecessary below \(2^{64}\), unless a later campaign intentionally crosses the 64-bit candidate range.

[PLAUSIBLE] Required data structures are small: arrays of powers \(14^d\), prefix digit lengths \(L_{14}(14^d-1)\), per-thread counters, and an append-only hit log.

[PLAUSIBLE] A bitset or residue-class loop can encode the exclusion of multiples of 2 and 7, but a simple wheel modulo 14 may be enough.

### 2.5 Hardware feasibility

[PLAUSIBLE] RAM use is negligible relative to 64 GB.

[PLAUSIBLE] Disk I/O is negligible if progress checkpoints are coarse and hits are rare.

[PLAUSIBLE] The bottleneck is CPU integer arithmetic, especially modular multiplication/reduction.

[PLAUSIBLE] The closest framework anchor is the 64-bit modular-operations class, \(1.3\cdot 10^{13}\) operations/hour on 16 threads. The candidate-level throughput depends on how many modular operations the block evaluator needs per \(k\).

[PLAUSIBLE] A realistic candidate-throughput bracket before measurement is roughly \(5\cdot 10^9\) to \(2\cdot 10^{10}\) tested candidates/hour after the modulo-14 wheel, depending on the block-sum implementation.

### 2.6 Aggregate throughput

Recommended chunking: chunks of \(10^8\) candidate integers, internally skipping excluded residues modulo 14.

For the interval \((3\cdot 10^{10},3\cdot 10^{11}]\), the raw range size is \(2.7\cdot 10^{11}\), giving 2700 chunks of size \(10^8\). This is much larger than \(16\times 10=160\), so load balancing is adequate.

At \(5\cdot 10^9\) candidates/hour, the raw 270-billion candidate range would take about 54 h before accounting for skipped residues. With the \(2/7\) and \(7\)-divisibility exclusion, only about \(3/7\) of integers survive, giving an effective estimate near 23 h if rejection is implemented cheaply.

At \(1.0\cdot 10^{10}\) candidates/hour raw, the same range would take about 27 h raw and about 12 h on surviving residues.

The recommended planning value for \((3\cdot 10^{10},3\cdot 10^{11}]\) is therefore **20 h wall-clock**, pending a real benchmark.

[TO-VERIFY] A micro-prototype must measure actual candidates/hour on windows near \(3\cdot 10^{10}\), \(10^{11}\), and \(3\cdot 10^{11}\).

### 2.7 Preliminary verdict (GO / BORDERLINE / NO-GO)

**GO**.

The project is suitable for Phase 2, with a mandatory micro-prototype before launching a full \(3\cdot 10^{11}\) campaign.

### 2.8 Convenience index

Classification: **Type A - term hunting**.

Rationale: the campaign may find a new term, but it may also only extend the no-other-terms bound. The value of a negative result is captured by the bound factor.

The current certified bound is \(3\cdot 10^{10}\) in the supplied OEIS snapshot. The probabilities below are planning assumptions, not proven statistical estimates. They must be revisited after a micro-prototype and after Carlo confirms whether any private search history exists.

#### Conservative scenario - certify/search up to \(10^{11}\)

Parameters:

- \(P_{\mathrm{hit}}=20\%\)
- \(T_{\mathrm{wall}}=6.0\) h
- bound factor \(=10^{11}/(3\cdot 10^{10})=3.33\)

Full script output:

```text
  Sequenza:  A029483  (Tipo A)
    Input: P_hit=20%  T_wall=6.0h  W_bound=40%
    Calcolo: V=0.520  C=0.000  score = 5*V - 2.3*C = 2.60/5
    >>> VALUTA (marginale, decidi caso per caso)
```

#### Recommended scenario - certify/search up to \(3\cdot 10^{11}\)

Parameters:

- \(P_{\mathrm{hit}}=40\%\)
- \(T_{\mathrm{wall}}=20.0\) h
- bound factor \(=(3\cdot 10^{11})/(3\cdot 10^{10})=10\)

Full script output:

```text
  Sequenza:  A029483  (Tipo A)
    Input: P_hit=40%  T_wall=20.0h  W_bound=60%
    Calcolo: V=0.760  C=0.000  score = 5*V - 2.3*C = 3.80/5
    >>> PROCEDI (buon candidato)
```

#### Ambitious scenario - certify/search up to \(10^{12}\)

Parameters:

- \(P_{\mathrm{hit}}=60\%\)
- \(T_{\mathrm{wall}}=72.0\) h
- bound factor \(=10^{12}/(3\cdot 10^{10})=33.33\)

Full script output:

```text
  Sequenza:  A029483  (Tipo A)
    Input: P_hit=60%  T_wall=72.0h  W_bound=60%
    Calcolo: V=0.840  C=1.083  score = 5*V - 2.3*C = 1.71/5
    >>> SCONSIGLIATA (valore atteso basso vs costo)
    [!] T=72.0h oltre soglia stop (70h)
```

Recommended scenario: **search/certify up to \(3\cdot 10^{11}\)**. It gives a tenfold bound extension, remains below the 24 h comfort threshold under the planning estimate, and has a Type A score of 3.80/5, i.e. `PROCEDI (buon candidato)`.

Corner-case checks:

- No scenario uses \(P_{\mathrm{hit}}<15\%\), so the rigid low-probability Type A cap is not active.
- The ambitious scenario has \(T_{\mathrm{wall}}>70\) h, so the severe time penalty is active.
- No \(T_{\mathrm{wall}}<2.4\) h exception is relevant.

### 2.9 Minimum validation plan

1. Implement a small prototype of the digit-block modular evaluator.
2. Validate against a naive concatenation/modular implementation for small \(k\), at least through \(10^5\).
3. Validate that the prototype reproduces all supplied OEIS terms up to the largest known term and finds no extra terms in a smaller certified prefix.
4. Run benchmark windows of at least \(10^7\) raw candidates near \(3\cdot 10^{10}\), \(10^{11}\), and \(3\cdot 10^{11}\).
5. Measure raw candidates/hour, surviving candidates/hour after the modulo-14 wheel, and per-thread scaling from 1, 4, 8, and 16 threads.
6. Stop/go thresholds:
   - proceed to \(3\cdot 10^{11}\) if projected wall time is \(\le 24\) h;
   - reduce to \(10^{11}\) or optimize if projected wall time is 24-70 h;
   - do not launch a full campaign if projected wall time exceeds 70 h without a new mathematical filter.

## 3. Integrated verdict and recommended next action

Phase 1 verdict: **GO**.

Convenience-index verdict for the recommended scenario: **PROCEDI (buon candidato)**.

Recommended next action: proceed to Phase 2 by building a micro-prototype of the independent digit-block modular concatenation scan. Do not start the full campaign until the prototype confirms correctness on known terms and measures throughput on the real range.

Operational target for Phase 2: certify/search A029483 up to \(3\cdot 10^{11}\), unless the benchmark projects more than 24 h wall-clock, in which case use \(10^{11}\) as the first production bound.

## 4. Notes and open points

- [TO-VERIFY] Confirm whether Jason Yuen's \(3\cdot 10^{10}\) bound used an algorithm similar to the digit-block modular formula.
- [TO-VERIFY] Carlo should confirm whether there are private notes, unpublished code, or manual literature references to integrate.
- [TO-VERIFY] The Type A hit-probability assumptions in the convenience index are planning assumptions, not mathematical estimates.
- [TO-VERIFY] Benchmark the exact block-sum implementation before committing to the \(3\cdot 10^{11}\) run.
- [TO-VERIFY] Evaluate whether small-prime necessary-condition filters improve wall time after accounting for sieve/factor overhead.
