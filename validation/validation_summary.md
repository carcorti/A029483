# Validation Summary

Date: 2026-06-03

## Scope

This package supports a completed A029483 production search over:

```text
[30000000001, 300000000000]
```

The search tested `270000000000` raw integers. After the parity and
multiple-of-7 filters, `115714285714` candidate integers were evaluated by the
full modular test.

The only new A029483 terms found in the interval were:

```text
44445214853
243987299933
```

The final checkpoint records:

```text
completed_end=300000000000
next_start=300000000001
hits=2
```

The checkpoint's `raw_tested`, `survivors_tested`, and `elapsed_seconds` fields
refer to the final segment only. The full segmented campaign is summarized in
`results/A029483_official_run_segments.tsv`.

## Public Package Checks

Commands run from the original project root:

```bash
make -C GitHub/src -f Makefile clean
make -C GitHub/src -f Makefile test
make -C GitHub/src -f Makefile sanitize
GitHub/scripts/certify_a029483.py --known --new
GitHub/scripts/certify_a029483.py --file GitHub/results/A029483_new_terms_u64.tsv
```

All commands passed.

Additional data-format checks passed:

- `data/b029483.txt` has 16 consecutive one-based rows.
- `data/b029483.txt` values are strictly increasing.
- `data/certified_terms.tsv` records source and certificate status for all 16
  terms without using an OEIS a-file-style name.
- `results/A029483_new_terms_u64.tsv` contains exactly two integer rows after
  its comment header.
- Both result rows lie inside the completed production interval.
- Public source and Makefile contain no `vN` filename suffixes.

## Code Review And Tooling Summary

The production code had previously passed:

- Release build with strict warning flags.
- Self-test and Makefile regression suite.
- ASan/UBSan.
- TSan.
- Cross-thread determinism checks for thread counts 1, 2, 4, 8, and 16.
- Valgrind self-test and smoke run.
- scan-build with no bugs found.
- cppcheck with only a known defensive-wrap false positive.
- clang-tidy with only triaged standard C/POSIX portability warnings.
- A hyperfine throughput probe near `3 * 10^10`.

See `validation/final_code_validation.md` for the pre-campaign validation
record.

## Independent Certificates

The Python checker in `scripts/certify_a029483.py` uses a 3x3 affine matrix
method by digit-length block. This is independent of the C program's recursive
geometric/arithmetic series combination.

The two newly discovered values certify as:

```text
k=44445214853: residue=0 direct_residue=n/a PASS
k=243987299933: residue=0 direct_residue=n/a PASS
```

Neighbor checks were also run during the campaign for `243987299933`:

```text
k=243987299932: residue=196861283305 direct_residue=n/a FAIL
k=243987299933: residue=0 direct_residue=n/a PASS
k=243987299934: residue=150270907739 direct_residue=n/a FAIL
```

## SHA-256 Checksums

```text
bc18ca59555496a55fce4ebf05e9bc11c2000d624a579427ae80be1a121f4a78  GitHub/src/a029483.c
57d9bb4a440ff3e223816ac7028715e728dc5414518022002ef0669b19901ecf  GitHub/src/Makefile
c41701812ab38c28a8d521606f378a0d2824f82555c73effbf85497e21547800  GitHub/scripts/certify_a029483.py
1fd0e2130a87e2944bbf710dc21345adf7685460857b5371bdfccdf784176a47  GitHub/data/b029483.txt
8c7d80828d4b7f85f7246714092585d3e41767b021a67675059f890534d5f464  GitHub/data/certified_terms.tsv
b1dc04e3fdc0f8c56545273d20b4c3b56c520d4a6fbc595bb3cc091a8478d8cf  GitHub/results/A029483_new_terms_u64.tsv
13e47c4ef15bd89e6c98906498065ea381bd838bf62c6b549375acc20f3f7fc5  GitHub/results/A029483_official_run_segments.tsv
0772bf7abb65d57880e0179981f0aea5324a8346a32d500caea9668d83ade22b  GitHub/results/a029483_checkpoint_final.txt
612bb9b44cf2d382761ab7775c90c34122c3cf3353b0df420be98a3696c3e4ca  GitHub/analysis/dossier_A029483.md
c01ed45c4aefb43fbfd071813f254cf096d66bc194e11e943d533939670f60c5  GitHub/analysis/A029483_OEIS_source.md
cf0d048be2583a47bc4eea29123e6dd569dc52cb4c5bafb123712b875c7b5ae8  GitHub/validation/final_code_validation.md
fc9d38a8a8d41dfe2c1e28086b121ecf38bd979d7b034e16effe7efcc1d0ef85  GitHub/validation/validation_notes.md
b2c3391921dc1246c4252514fffe7fe6a759893aecf5d21a9b0c00d71fdd7b2b  GitHub/paper/paper_notes.md
2c9488151c7fd85433e124be1d4e543e2a4e29009d02c992f9a284174afe682d  GitHub/README.md
07d56e440818e39e6cb7f68127e7dc1fcbc5e3fdc6f6f6615e64673c265c64d6  GitHub/.gitignore
```
