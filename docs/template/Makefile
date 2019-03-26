../index.html: ../README.md prefix.html suffix.html
	cat prefix.html > index.html
	pandoc ../README.md --from=gfm -t html >> index.html
	cat suffix.html >> index.html
	mv index.html ../

.PHONY: clean

clean:
	rm -f ../index.html
