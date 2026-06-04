# Computational Extension of OEIS Sequence A029483

Repository supporting the computation of new terms of OEIS A029483 (numbers k that divide the left concatenation of 1,2,...,k written in base 14).

## Main result

The search over the interval:

[30000000001, 300000000000]

found exactly two new terms:

- 44445214853
- 243987299933

Extending the sequence through:

- a(15) = 44445214853
- a(16) = 243987299933

## Repository structure

- src/ : production C17 implementation and Makefile
- data/ : b-file and certified term tables
- results/ : production outputs and segment manifests
- validation/ : validation summaries and notes
- analysis/ : project analysis material
- scripts/ : independent certification utilities
- paper/ : manuscript source and PDF

## Method

The computation evaluates the base-14 left concatenation modulo k using digit-length blocks and arithmetic-geometric sums modulo k. The concatenated integer is never constructed explicitly.

## Validation

The production search was implemented in C17. Reported positive terms were independently certified with a Python checker based on a 3x3 matrix method independent from the production kernel.

Validation material included in this repository consists of selected validation summaries and certification documents.

## Reproducibility

Build:

```bash
cd src
make
```

Main source:

- src/a029483.c

Certification utility:

- scripts/certify_a029483.py

## Data files

- data/b029483.txt : proposed OEIS b-file extension including the newly computed terms
- data/certified_terms.tsv : independently certified positive terms reported in this work

## Results

- results/A029483_new_terms_u64.tsv : newly identified sequence terms
- results/A029483_official_run_segments.tsv : production search segment manifest
- results/a029483_checkpoint_final.txt : final checkpoint information from the production run

## Validation material

- validation/validation_summary.md
- validation/validation_notes.md
- validation/final_code_validation.md

## Paper

- paper/A029483.tex
- paper/A029483.pdf

## License

See LICENSE.

## Citation

If you use this repository, cite the accompanying paper and repository metadata in CITATION.cff.

## Repository status

Initial public release (v1.0).

Zenodo DOI: to be added after GitHub release and Zenodo archival.
