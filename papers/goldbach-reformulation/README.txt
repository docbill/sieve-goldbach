Build:
  latexmk -pdf -interaction=nonstopmode -halt-on-error sieve_goldbach.tex
or on Overleaf: upload all files, set main file to sieve_goldbach.tex.

Notes:
  - CSV inputs must sit in the same directory as the .tex.
  - Tested with TeX Live 2022+; pgfplots compat ~1.18.
  - No shell-escape required unless you enable TikZ externalization.

