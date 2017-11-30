LATEX	= latex -shell-escape
DVIPS	= dvips
DVIPDF  = dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

EXAMPLES = $(wildcard *.c)
SRC	:= $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG	= $(SRC:%.tex=%.dvi)
PSF	= $(SRC:%.tex=%.ps)
PDF	= $(SRC:%.tex=%.pdf)

all: pdf problem1 problem2

pdf: $(PDF)

ps: $(PSF)

$(TRG): %.dvi: %.tex $(EXAMPLES)
	#one way of including source code is to use pygments
#	pygmentize -f latex -o __${EXAMPLES}.tex ${EXAMPLES}
	#requires that you \include{pygments.tex} in your preamble

	$(LATEX) $<
#	$(BIBTEX) $(<:%.tex=%)
	$(LATEX) $<
	$(LATEX) $<
	#remove the pygmentized output to avoid cluttering up the directory
	#rm __${SRC}.tex


$(PSF):%.ps: %.dvi
	$(DVIPS) -R -Poutline -t letter $< -o $@

$(PDF): %.pdf: %.ps
	ps2pdf $<

show: $(TRG)
	@for i in $(TRG) ; do $(XDVI) $$i & done

showps: $(PSF)
	@for i in $(PSF) ; do $(GH) $$i & done

problem1: problem1.c
	gcc problem1.c -o problem1 -lpthread
problem2: problem2.c
	gcc problem2.c -o problem2 -lpthread

#all: pdf

clean:
	rm -f *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.o

.PHONY: all show clean ps pdf showps
