../index.html: ../index.md prefix.html suffix.html
	cat prefix.html > index.html
	showdown makehtml -i ../index.md >> index.html
	cat suffix.html >> index.html
	mv index.html ../

.PHONY: clean

clean:
	rm -f ../index.html
