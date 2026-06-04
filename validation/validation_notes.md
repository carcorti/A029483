# Validation Notes

The raw interactive terminal transcript is not treated as the archival object.
The reproducible artifacts are the production source, the Makefile, the
independent certificate script, the final result TSV, the run-segment manifest,
the b-file extension, and the SHA-256 checksums in `validation_summary.md`.

The original workspace also contains per-segment logs under `results/logs/`.
For the GitHub raw package, those logs were condensed into
`results/A029483_official_run_segments.tsv` to keep the publication directory
compact and audit-friendly.

The final checkpoint after the production campaign was:

```text
sequence=A029483
completed_end=300000000000
next_start=300000000001
hits=2
```

The `raw_tested` and `survivors_tested` fields in that checkpoint refer to the
last segment only; the full campaign totals are recorded in
`validation_summary.md`.

