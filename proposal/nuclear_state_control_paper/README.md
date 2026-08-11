# Nuclear-State Control Paper

This directory is organized as an arXiv-friendly LaTeX project.

## Structure

- `main.tex` — master document. It contains title, authors, abstract, packages, and loads all sections.
- `sections/01_introduction.tex`
- `sections/02_control_framework.tex`
- `sections/03_physical_basis.tex`
- `sections/04_minimal_model.tex`
- `sections/05_candidate_system.tex`
- `sections/06_conclusions.tex`
- `references.bib` — BibTeX database.
- `figures/` — place figures here.

## Build

From the project directory:

```bash
pdflatex main.tex
bibtex main
pdflatex main.tex
pdflatex main.tex
```

or, if available:

```bash
latexmk -pdf main.tex
```

## Before submission

1. Replace `Author Name`, email, and affiliation in `main.tex`.
2. Review the abstract after the scientific claims are finalized.
3. Perform a dedicated prior-art search before making any strong novelty claim.
4. Verify every numerical statement against the cited primary source.
5. Add figures only after the mathematical framework is stable.
6. For arXiv, upload `main.tex`, the `sections/` directory, `references.bib`, and any figures.
