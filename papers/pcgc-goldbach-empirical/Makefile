# Ensure the PDF name matches your main tex file
FILE = pcgc-goldbach-empirical

all:
	latexmk -pdf -pdflatex="pdflatex -synctex=1 -interaction=nonstopmode -halt-on-error" $(FILE).tex

clean:
	latexmk -C
	rm -f *.bbl *.run.xml *.synctex.gz

