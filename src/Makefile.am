bin_PROGRAMS   = bulk_extractor 
EXTRA_PROGRAMS = stand
CLEANFILES     = scan_accts.cpp scan_email.cpp scan_gps.cpp scan_base16.cpp *.d
EXTRA_DIST     = MyFlexLexer.h dfrws_2012_challenge.py data_sniffer 

# scan_lift/type_identifier.tex

AM_CPPFLAGS = -I${srcdir}/re2 

#extradir = .

TSK3INCS = \
	tsk3/auto/tsk_auto.h\
	tsk3/base/tsk_base.h\
	tsk3/base/tsk_os.h\
	tsk3/fs/tsk_fatfs.h\
	tsk3/fs/tsk_fs.h\
	tsk3/fs/tsk_ntfs.h\
	tsk3/hashdb/tsk_hashdb.h\
	tsk3/img/tsk_img.h\
	tsk3/libtsk.h\
	tsk3/tsk_incs.h\
	tsk3/vs/tsk_vs.h

flex_scanners = \
	sbuf_flex_scanner.h \
	scan_accts.flex \
	scan_base16.flex \
	scan_email.flex \
	scan_gps.flex 

bulk_scanners = \
	scan_aes.cpp \
	scan_base64.cpp \
	scan_bulk.cpp\
	scan_ccns2.cpp scan_ccns2.h \
	scan_elf.cpp \
	scan_exif.cpp exif_reader.h exif_reader.cpp exif_entry.cpp\
	scan_exiv2.cpp \
	scan_find.cpp \
	scan_kml.cpp \
	scan_gzip.cpp \
	scan_hiberfile.cpp pyxpress.c pyxpress.h \
	scan_json.cpp \
	scan_windirs.cpp \
	scan_extx.cpp \
	scan_winpe.cpp \
	scan_net.cpp \
	scan_pdf.cpp \
	scan_vcard.cpp \
	scan_winprefetch.cpp \
	scan_wordlist.cpp \
	scan_zip.cpp \
	$(flex_scanners) 

SCAN_LIFT = scan_lift.cpp \
		scan_lift/base.cpp \
		scan_lift/base.h \
		scan_lift/classifier_config.h \
		scan_lift/element.h \
		scan_lift/gflags.h \
		scan_lift/gflags_completions.h \
		scan_lift/irbase.cpp \
		scan_lift/lift_config.cpp \
		scan_lift/lift_config.h \
		scan_lift/lift_svm.cpp \
		scan_lift/lift_svm.h \
		scan_lift/lift_svm_trained.cpp \
		scan_lift/lift_utils.h \
		scan_lift/linear_binary_svm.cpp \
		scan_lift/linear_binary_svm.h \
		scan_lift/linear_ova_svm.cpp \
		scan_lift/linear_ova_svm.h \
		scan_lift/myheader.h \
		scan_lift/rawdefines.h \
		scan_lift/rawheaders.h \
		scan_lift/term_weighting.cpp \
		scan_lift/term_weighting.h \
		scan_lift/tools.cpp 


REGEX = glibc/regex.c glibc/regexec.c glibc/regex.h  glibc/regex_internal.h
EXTRA_DIST += glibc/regex_internal.c

RE2 = \
	re2/util/arena.cc \
	re2/util/arena.h \
	re2/util/atomicops.h \
	re2/util/flags.h \
	re2/util/hash.cc \
	re2/util/logging.h \
	re2/util/mutex.h \
	re2/util/rune.cc \
	re2/util/sparse_array.h \
	re2/util/sparse_set.h \
	re2/util/stringpiece.cc \
	re2/util/stringprintf.cc \
	re2/util/strutil.cc \
	re2/util/utf.h \
	re2/util/util.h \
	re2/util/valgrind.cc \
	re2/util/valgrind.h \
	re2/re2/bitstate.cc \
	re2/re2/compile.cc \
	re2/re2/dfa.cc \
	re2/re2/filtered_re2.cc \
	re2/re2/filtered_re2.h \
	re2/re2/mimics_pcre.cc \
	re2/re2/nfa.cc \
	re2/re2/onepass.cc \
	re2/re2/parse.cc \
	re2/re2/perl_groups.cc \
	re2/re2/prefilter.cc \
	re2/re2/prefilter.h \
	re2/re2/prefilter_tree.cc \
	re2/re2/prefilter_tree.h \
	re2/re2/prog.cc \
	re2/re2/prog.h \
	re2/re2/re2.cc \
	re2/re2/re2.h \
	re2/re2/regexp.cc \
	re2/re2/regexp.h \
	re2/re2/set.cc \
	re2/re2/set.h \
	re2/re2/simplify.cc \
	re2/re2/stringpiece.h \
	re2/re2/tostring.cc \
	re2/re2/unicode_casefold.cc \
	re2/re2/unicode_casefold.h \
	re2/re2/unicode_groups.h \
	re2/re2/unicode_groups.cc \
	re2/re2/variadic_function.h \
	re2/re2/walker-inl.h 

bulk_extractor_SOURCES = \
	aftimer.h \
	base64_forensic.cpp \
	base64_forensic.h \
	bulk_extractor.cpp \
	bulk_extractor.h \
	bulk_extractor_i.h \
	cppmutex.h \
	dig.cpp \
	dig.h \
	feature_recorder.cpp \
	feature_recorder.h \
	feature_recorder_set.cpp \
	feature_recorder_set.h \
	histogram.cpp \
	histogram.h \
	image_process.cpp \
	image_process.h \
	md5.c \
	md5.h \
	beregex.cpp \
	beregex.h \
	plugin.cpp \
	sbuf.cpp \
	sbuf.h \
	sbuf_private.h \
	sbuf_stream.cpp \
	sbuf_stream.h \
	support.cpp \
	threadpool.cpp \
	threadpool.h \
	unicode_escape.cpp\
	unicode_escape.h \
	utf8.h \
	utf8/checked.h \
	utf8/core.h \
	utf8/unchecked.h \
	utils.cpp \
	utils.h \
	word_and_context_list.cpp \
	word_and_context_list.h \
	xml.cpp \
	xml.h \
	$(bulk_scanners) $(TSK3INCS) 
#$(REGEX)

stand_SOURCES = \
	dig.cpp \
	feature_recorder.cpp \
	feature_recorder_set.cpp \
	histogram.cpp \
	histogram.h \
	md5.c \
	plugin.cpp \
	sbuf.cpp \
	scan_bulk.cpp \
	stand.cpp \
	support.cpp \
	word_and_context_list.cpp \
	word_and_context_list.h \
	xml.cpp 

#stand_word_and_context_list: word_and_context_list.h word_and_context_list.cpp
#	g++ -I.. -o stand_word_and_context_list -DSTAND word_and_context_list.cpp 

SUFFIXES = .flex

src/scan_lift_trained.cpp:
	./bulk_extractor -E lift -s liftcmd=a2c -s liftif=scan_lift/model -s liftof=scan_lift/lift_svm_trained.cpp

digtest$(EXEEXT): dig.cpp
	$(CXX) -DSTANDALONE -o digtest$(EXEEXT) -g dig.cpp $(CPPFLAGS) $(CXXFLAGS) -I..

unitest$(EXEEXT): unicode_escape.cpp
	$(CXX) -DSTANDALONE -o unitest$(EXEEXT) -g unicode_escape.cpp  $(CPPFLAGS) $(CXXFLAGS) -I..

dtest:
	echo test for DFRWS 2012
	python dfrws_2012_challenge.py /corp/drives/nps/nps-2010-emails/nps-2010-emails.raw 4096 

#
# Old Magic below is needed to get the LexerInput signature from the flex output. 
# Unfortunately, the flex output may contain a comment, and the /* foo */
# got expanded to a list of all the files in the root directory, so
# s:/.*/:: removes the comment
#.flex.o:
#	cat ${srcdir}/flex_header.h > `basename $@ .o`.cpp
#	flex -t $<   | sed 's/.FlexLexer.h./"MyFlexLexer.h"/' >> `basename $@ .o`.cpp 
#	(SFN=`basename $@ .o`.cpp; \
#	 SIG="`grep ::LexerInput $$SFN | tail -1 | sed s:/.*/:: | sed s/yyFlexLexer:://`"; \
#	 echo SIG=$$SIG; \
#	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I${top_srcdir} -I${srcdir} \
#		-DLEXERINPUTSIGNATURE="$$SIG" -I.. -I../../_build -c `basename $@ .o`.cpp)
# 
# I think that this will work...

FLEX_CXXFLAGS=-Wno-shadow -Wno-effc++ -Wno-redundant-decls -Wno-missing-noreturn
.flex.o:
	flex -o `basename $@ .o`.cpp $<  
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(FLEX_CXXFLAGS) -I${top_srcdir} -I${srcdir} -I.. -I../../_build -c `basename $@ .o`.cpp 
