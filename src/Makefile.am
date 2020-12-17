## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = serial-tests
bin_PROGRAMS = digitalbits-core

include $(top_srcdir)/common.mk
include $(srcdir)/src.mk

noinst_HEADERS = $(SRC_H_FILES)

if BUILD_TESTS
digitalbits_core_SOURCES = main/DigitalBitsCoreVersion.cpp $(SRC_CXX_FILES) $(SRC_TEST_CXX_FILES)
else # !BUILD_TESTS
digitalbits_core_SOURCES = main/DigitalBitsCoreVersion.cpp $(SRC_CXX_FILES)
endif # !BUILD_TESTS

digitalbits_core_LDADD = $(soci_LIBS) $(libmedida_LIBS)		\
	$(top_builddir)/lib/lib3rdparty.a $(sqlite3_LIBS)	\
	$(libpq_LIBS) $(xdrpp_LIBS) $(libsodium_LIBS) $(libunwind_LIBS)

TESTDATA_DIR = testdata
TEST_FILES = $(TESTDATA_DIR)/digitalbits-core_example.cfg $(TESTDATA_DIR)/digitalbits-core_standalone.cfg \
             $(TESTDATA_DIR)/digitalbits-core_testnet.cfg $(TESTDATA_DIR)/digitalbits-core_testnet_legacy.cfg \
	     $(TESTDATA_DIR)/digitalbits-history.testnet.6714239.json $(TESTDATA_DIR)/digitalbits-history.livenet.15686975.json $(TESTDATA_DIR)/digitalbits-core_example_validators.cfg \
	     $(TESTDATA_DIR)/digitalbits-history.testnet.6714239.networkPassphrase.json

BUILT_SOURCES = $(SRC_X_FILES:.x=.h) main/DigitalBitsCoreVersion.cpp $(TEST_FILES)

$(SRC_X_FILES:.x=.h): $(XDRC)
SUFFIXES = .x .h
.x.h:
	$(XDRC) -hh -o $@ $<

$(srcdir)/src.mk: $(top_srcdir)/make-mks
	cd $(top_srcdir) && ./make-mks

$(TESTDATA_DIR)/%.cfg : $(top_srcdir)/docs/%.cfg
	mkdir -p $(@D) && cp $< $@

$(TESTDATA_DIR)/%.json : $(top_srcdir)/src/history/serialize-tests/%.json
	mkdir -p $(@D) && cp $< $@

.PHONY: always
always:
	@:

# Always rebuild because .git/HEAD is a symbolic ref one can't depend on
main/DigitalBitsCoreVersion.cpp: always
	@vers=$$(cd "$(srcdir)" \
		&& git describe --always --dirty --tags 2>/dev/null \
		|| echo "$(PACKAGE) $(VERSION)"); \
		sed -e "s/%%VERSION%%/$$vers/" \
			< "$(srcdir)/main/DigitalBitsCoreVersion.cpp.in" > $@~
	@if cmp -s $@~ $@; then rm -f $@~; else \
	    mv -f $@~ $@ && printf "echo '%s' > $@\n" "$$(cat $@)"; fi

if USE_POSTGRES
TESTS=test/selftest-pg
else # !USE_POSTGRES
TESTS=test/selftest-nopg
endif # !USE_POSTGRES

if USE_CLANG_FORMAT
format: always
	cd $(srcdir) && $(CLANG_FORMAT) -style=file -i $(SRC_CXX_FILES) $(SRC_H_FILES) $(SRC_TEST_CXX_FILES) $(SRC_TEST_H_FILES)
endif # USE_CLANG_FORMAT

if USE_AFL_FUZZ
FUZZER_MODE ?= overlay

fuzz-testcases: digitalbits-core
	mkdir -p fuzz-testcases
	for i in `seq 1 10000`; do \
	    ./digitalbits-core gen-fuzz --mode=${FUZZER_MODE} fuzz-testcases/fuzz$$i.xdr ; \
	done
	mkdir -p min-testcases
	afl-cmin -i fuzz-testcases -o min-testcases -m 500 -t 250 ./digitalbits-core fuzz --ll ERROR --mode=${FUZZER_MODE} @@
	rm -Rf fuzz-testcases

# when running in parallel,
# run the same command than below replacing `-M main` with `-S worker_N`
# and `--process-id 0` with `--process-id N`

fuzz: fuzz-testcases digitalbits-core
	mkdir -p fuzz-findings
	afl-fuzz -m 500 -M main -t 250 -i min-testcases -o fuzz-findings \
	    ./digitalbits-core fuzz --ll ERROR --process-id 0 --mode=${FUZZER_MODE} @@

fuzz-clean: always
	rm -Rf fuzz-testcases fuzz-findings

distclean-local: fuzz-clean
endif # USE_AFL_FUZZ

CLEANFILES = $(BUILT_SOURCES) *~ */*~ digitalbits*.log
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in $(srcdir)/*~ $(srcdir)/*/*~

EXTRA_DIST = $(SRC_X_FILES) test/selftest-nopg test/selftest-pg
