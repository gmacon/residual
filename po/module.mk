POTFILE := $(srcdir)/po/residual.pot
POFILES := $(wildcard $(srcdir)/po/*.po)

updatepot:
	xgettext -f $(srcdir)/po/POTFILES -D $(srcdir) -d residual --c++ -k_ -k_s -k_c:1,2c -k_sc:1,2c \
		-kDECLARE_TRANSLATION_ADDITIONAL_CONTEXT:1,2c -o $(POTFILE) \
		--copyright-holder="Residual Team" --package-name=Residual \
		--package-version=$(VERSION) --msgid-bugs-address=residual-devel@lists.sf.net -o $(POTFILE)_

	sed -e 's/SOME DESCRIPTIVE TITLE/LANGUAGE translation for Residual/' \
		-e 's/UTF-8/CHARSET/' -e 's/PACKAGE/Residual/' $(POTFILE)_ > $(POTFILE).new

	rm $(POTFILE)_
	if test -f $(POTFILE); then \
		sed -f $(srcdir)/po/remove-potcdate.sed < $(POTFILE) > $(POTFILE).1 && \
		sed -f $(srcdir)/po/remove-potcdate.sed < $(POTFILE).new > $(POTFILE).2 && \
		if cmp $(POTFILE).1 $(POTFILE).2 >/dev/null 2>&1; then \
			rm -f $(POTFILE).new; \
		else \
			rm -f $(POTFILE) && \
			mv -f $(POTFILE).new $(POTFILE); \
		fi; \
		rm -f $(POTFILE).1 $(POTFILE).2; \
	else \
		mv -f $(POTFILE).new $(POTFILE); \
	fi;

%.po: $(POTFILE)
	msgmerge $@ $(POTFILE) -o $@.new
	if cmp $@ $@.new >/dev/null 2>&1; then \
		rm -f $@.new; \
	else \
		mv -f $@.new $@; \
	fi;

translations-dat: devtools/create_translations
	devtools/create_translations/create_translations $(POFILES)
	mv translations.dat $(srcdir)/gui/themes/

update-translations: updatepot $(POFILES) translations-dat

update-translations: updatepot $(POFILES)
	@$(foreach file, $(POFILES), echo -n $(notdir $(basename $(file)))": ";msgfmt --statistic $(file);)
	@rm -f messages.mo

.PHONY: updatepot translations-dat update-translations
