.PHONY: all clean dist print

P = omnet_router_spec

default : $P.pdf

$P.dvi	: $(wildcard *.tex *.bib figures/*.eps)
	latex  $P < /dev/null || $(RM) $@
	bibtex $P < /dev/null || $(RM) $@
	latex  $P < /dev/null || $(RM) $@
	latex  $P < /dev/null || $(RM) $@

onerun  : $(wildcard *.tex *.bib figures/*)
	bibtex $P
	pdflatex --shell-escape $P

$P.pdf  : $(wildcard *.tex *.bib figures/*)
	pdflatex --shell-escape $P
	bibtex $P
	pdflatex --shell-escape $P
	pdflatex --shell-escape $P

$P.ps	: $P.dvi
	dvips -tletter -Ppdf $P.dvi -o $P.ps

$P.ps.gz: $P.ps
	$(RM) $P.ps.gz
	gzip -9 < $P.ps > $P.ps.gz

print:	$P.ps

dist:	$P.ps.gz

clean:
	$(RM) $P.pdf $P.log $P.aux $P.bbl $P.blg $P.out $P.dvi $P.ps $P.ps.gz texput.log
