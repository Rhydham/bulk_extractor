#!/usr/bin/env python
# coding=UTF-8
"""
Regression system: 
 - Run bulk_extractor on one of three disk images. 
 - The feature files are sorted (to deal with multi-threading issues).
 - Total number of features are reported and compared with the archives.
"""

__version__ = "1.3.0"

b'This module needs Python 2.7 or later.'

import os,sys
try:
    sys.path.append(os.getenv("DOMEX_HOME") + "/src/bulk_extractor/trunk/python/") # add the library
except TypeError as e:
    pass
sys.path.append("../python/")      # add the library
sys.path.append("python/")      # add the library

from subprocess import Popen,call,PIPE
import os.path,glob,zipfile,codecs
import bulk_extractor_reader

default_infile    = "nps-2009-ubnist1/ubnist1.gen3.raw"
fast_infile      = "nps-2009-canon2/nps-2009-canon2-gen6.raw"
full_infile = "nps-2009-domexusers/nps-2009-domexusers.raw"
exe = "src/bulk_extractor"
BOM = codecs.BOM_UTF8.decode('utf-8')

def find_file(fn):
    if os.path.exists(fn): return fn
    chk = []
    for ne in ('.E01','.aff','.000','.001'):
        tfn = fn.replace(".raw",ne)
        if os.path.exists(tfn):
            return tfn
        chk.append([tfn])
    raise RuntimeError("Cannot find file "+tfn)


if sys.version_info < (2,7):
    raise "Requires Python 2.7 or above"

def analyze_linebyline(outdir):
    """Quick analysis of an output directory"""
    import xml.etree.cElementTree
    import xml.dom.minidom
    fn = os.path.join(outdir,"report.xml")
    lines = 0
    inprocess = dict()
    for line in open(fn):
        line = line.replace("debug:","debug")
        try:
            doc = xml.dom.minidom.parseString(line)
            start = doc.getElementsByTagName("debugwork_start")
            if start:
                threadid = int(start[0].attributes['threadid'].firstChild.wholeText)
                inprocess[threadid] = start[0]
            end = doc.getElementsByTagName("debugwork_end")
            if end:
                threadid = int(start[0].attributes['threadid'].firstChild.wholeText)
                inprocess[threadid] = start[0]
        except xml.parsers.expat.ExpatError as e:
            print(e)
            pass
        lines += 1
    print("Total lines: {}".format(lines))
    exit(0)

def reproduce_flags(outdir):
    """Craft BE flags to quickly reproduce a crash"""
    import xml.etree.cElementTree
    import xml.dom.minidom
    filename = None
    offset = None
    fn = os.path.join(outdir,"report.xml")
    active_offsets = set()
    last_start_offset = None
    for line in open(fn):
        line = line.replace("debug:","debug")
        try:
            doc = xml.dom.minidom.parseString(line)
            prov_filename = doc.getElementsByTagName("provided_filename")
            if prov_filename:
                filename = str(prov_filename[0].firstChild.wholeText)
            start = doc.getElementsByTagName("debugwork_start")
            if start:
                offset = int(start[0].attributes['pos0'].firstChild.wholeText)
                last_start_offset = offset
                active_offsets.add(offset)
            end = doc.getElementsByTagName("debugwork_end")
            if end:
                offset = int(end[0].attributes['pos0'].firstChild.wholeText)
                try:
                    active_offsets.remove(offset)
                except KeyError:
                    pass
        except xml.parsers.expat.ExpatError as e:
            #print(e)
            pass
    if len(active_offsets) < 1:
        print("*** Warning: no unfinished sectors found; using best guess of last sector started")
        offset = last_start_offset
    else:
        offset = min(active_offsets)
    return "-Y {offset} {filename}".format(offset=offset, filename=filename)

def analyze_outdir(outdir):
    """Print statistics about an output directory"""
    print("Analyze {}".format(outdir))

    b = bulk_extractor_reader.BulkReport(outdir)
    print("bulk_extractor version: {}".format(b.version()))
    print("Filename:               {}".format(b.imagefile()))
    
    # Determine if any pages were not analyzed
    proc = dict()
    for work_start in b.xmldoc.getElementsByTagName("debug:work_start"):
        threadid = work_start.getAttribute('threadid')
        pos0     = work_start.getAttribute('pos0')
        if pos0 in proc:
            print("*** error: pos0={} was started by threadid {} and threadid {}".format(pos0,proc[pos0],threadid))
        else:
            proc[pos0] = threadid
    for work_end in b.xmldoc.getElementsByTagName("debug:work_end"):
        threadid = work_end.getAttribute('threadid')
        pos0     = work_end.getAttribute('pos0')
        if pos0 not in proc:
            print("*** error: pos0={} was ended by threadid {} but never started!".format(pos0,threadid))
        elif threadid!=proc[pos0]:
            print("*** error: pos0={} was ended by threadid {} but ended by threadid {}".format(pos0,proc[pos0],threadid))
        else:
            del proc[pos0]
    
    for (pos0,threadid) in proc.items():
        print("*** error: pos0={} was started by threadid {} but never ended".format(pos0,threadid))
    
    # Print which scanners were run and how long they took
    scanner_times = []
    scanners = b.xmldoc.getElementsByTagName("scanner_times")[0]
    total = 0
    for path in scanners.getElementsByTagName("path"):
        name    = path.getElementsByTagName("name")[0].firstChild.wholeText
        calls   = int(path.getElementsByTagName("calls")[0].firstChild.wholeText)
        seconds = float(path.getElementsByTagName("seconds")[0].firstChild.wholeText)
        total   += seconds
        scanner_times.append((name,calls,seconds))
    print("Scanner paths by time and calls")
    scanner_times.sort(key=lambda a:a[2],reverse=True)

    print("  {0:>25}  {1:8}  {2:12}  {3:12}  {4:5}".format("name","calls","sec","sec/call","% total"))
    for (name,calls,seconds) in scanner_times:
        print("  {:>25}  {:8.0f}  {:12.4f}  {:12.4f}  {:5.2f}%".format(
                name,calls,seconds,seconds/calls,100.0*seconds/total))
    
    
    hfns = list(b.histograms())
    print("")
    print("Histogram Files:        {}".format(len(hfns)))

    def get_firstline(fn):
        """Returns the first line that is not a comment"""
        for line in b.open(fn,'rb'):
            if bulk_extractor_reader.is_comment_line(line):
                continue
            return line[:-1]

    for fn in sorted(hfns):
        h = b.read_histogram(fn)
        firstline = get_firstline(fn)
        if(type(firstline)==bytes and type(firstline)!=str):
            firstline = firstline.decode('utf-8')
        print("  {:>25} entries: {:>10,}  (top: {})".format(fn,len(h),firstline))

    ffns = list(b.feature_files())
    print("")
    print("Feature Files:        {}".format(len(ffns)))
    for fn in sorted(ffns):
        lines = 0
        for line in b.open(fn,'rb'):
            if not bulk_extractor_reader.is_comment_line(line):
                lines += 1
        print("  {:>25} features: {:>10,}".format(fn,lines))
    

def make_zip(dname):
    archive_name = dname+".zip"
    b = bulk_extractor_reader.BulkReport(dname)
    z = zipfile.ZipFile(archive_name,compression=zipfile.ZIP_DEFLATED,mode="w")
    print("Creating ZIP archive {}".format(archive_name))
    for fname in b.all_files:
        print("  adding {} ...".format(fname))
        z.write(os.path.join(dname,fname),arcname=os.path.basename(fname))
    

def make_outdir(outdir_base):
    counter = 1
    while True:
        outdir = outdir_base + ("-%02d" % counter)
        if not os.path.exists(outdir):
            return outdir
        counter += 1

def run_outdir(outdir,gdb=False):
    print("run_outdir: ",outdir)
    cargs=['-o',outdir]
    if args.jobs: cargs += ['-j'+str(args.jobs)]
    
    cargs += ['-e','all']    # enable all scanners
    #cargs += ['-e','wordlist']    # enable all scanners
    if args.extra:     cargs += [args.extra]

    if args.debug: cargs += ['-d'+str(args.debug)]
    cargs += ['-r','tests/alert_list.txt']
    cargs += ['-w','tests/stop_list.txt']
    cargs += ['-w','tests/stop_list_context.txt']
    cargs += ['-f','[a-z\.0-9]*@gsa.gov']
    cargs += ['-F','tests/find_list.txt']
    cargs += [args.image]

    # Now that we have a command, figure out how to run it...
    if gdb:
        with open("/tmp/cmds","w") as f:
            f.write("b malloc_error_break\n")
            f.write("run ")
            f.write(" ".join(cargs))
            f.write("\n")
            
        cmd = ['gdb','-e',args.exe,'-x','/tmp/cmds']
    else:
        cmd = [args.exe] + cargs
    print(" ".join(cmd))
    r = call(cmd)
    if r!=0:
        raise RuntimeError("{} crashed with error code {}".format(args.exe,r))

             
def sort_outdir(outdir):
    print("Now sorting files in "+outdir)
    for fn in glob.glob(outdir + "/*.txt"):
        if "histogram" in fn: continue
        fns  = fn+".sorted"
        os.environ['LC_ALL']='C' # make sure we sort in C order
        call(['sort',fn],stdout=open(fns,"w"))
        wcout = Popen(['wc','-l',fns],stdout=PIPE).communicate()[0].decode('utf-8')
        lines = int(wcout.strip().split(" ")[0])
        if lines>0:
            call(['tail','-1',fns],stdout=open(fn,"w")) # copy over the UTF-8 header
            cmd = ['head','-'+str(lines-1),fns]
            call(cmd,stdout=open(fn,"a"))
        os.unlink(fns)

def check(fn,lines):
    found_lines = len(file(fn).read().split("\n"))-1
    if lines!=found_lines:
        print("{:10} expected lines: {} found lines: {}".format(fn,found_lines,lines))


def asbinary(s):
    ret = ""
    count = 0
    for ch in s:
        ret += "%02x " % ch
        count += 1
        if count>5:
            count = 0
            ret += " "
    return ret
        

def valid_feature_file_line(line):
    if not line: return True    # empty lines are okay
    if line[0]=='#': return True # comments are okay
    if line[0:2]=='n=': return True     # currently all histograms are okay
    if line.count('\t')==2: return True # correct number of tabs
    return False
    
def validate_openfile(f):
    fn = f.name
    if fn.endswith('.xml') or fn.endswith('.dmp') or fn.endswith("_tags.txt") or "wordlist" in fn:
        is_feature_file = False
        return
    else:
        is_feature_file = True

    # now read
    linenumber = 0
    print("Validate ",fn)
    for lineb in f:
        linenumber += 1
        lineb = lineb[:-1]
        try:
            line = lineb.decode('utf-8')
        except UnicodeDecodeError as e:
            print("{}:{} {} {}".format(fn,linenumber,str(e),asbinary(lineb)))
        if bulk_extractor_reader.is_comment_line(line):
            continue        # don't test
        if bulk_extractor_reader.is_histogram_line(line):
            continue        # don't test
        if is_feature_file and not valid_feature_file_line(line):
            print("{}: {:8} Invalid feature file line: {}".format(fn,linenumber,line))


def validate_report(fn):
    """Make sure all of the lines in all of the files in the outdir are UTF-8"""
    import glob,os.path
    res = {}
    if os.path.isdir(fn) or fn.endswith(".zip"):
        b = bulk_extractor_reader.BulkReport(fn)
        for fn in sorted(b.files):
            validate_openfile(b.open(fn,'rb'))
    else:
        validate_openfile(open(fn,'rb'))
            
            
def diff(dname1,dname2):
    args.max = int(args.max)
    def files_in_dir(dname):
        return [fn.replace(dname+"/","") for fn in glob.glob(dname+"/*")]
    def lines_to_set(fn):
        return set(open(fn).read().split("\n"))
    
    files1 = set(files_in_dir(dname1))
    files2 = set(files_in_dir(dname2))
    if files1.difference(files2):
        print("Files only in {}:\n   {}".format(dname1," ".join(files1.difference(files2))))
    if files2.difference(files1):
        print("Files only in {}:\n   {}".format(dname2," ".join(files2.difference(files1))))

    # Look at the common files
    common = files1.intersection(files2)
    for fn in common:
        def print_diff(dname,prefix,diffset):
            if not diffset:
                return
            print("Only in {}: {} lines".format(dname,len(diffset)))
            count = 0
            for line in sorted(diffset):
                print("{}{}".format(prefix,line))
                count += 1
                if count>args.max:
                    print(" ... +{} more lines".format(len(diffset)-int(args.max)))
                    return

        print("regressdiff {} {}:".format(os.path.join(dname1,fn),os.path.join(dname2,fn)))
        lines1 = lines_to_set(os.path.join(dname1,fn))
        lines2 = lines_to_set(os.path.join(dname2,fn))
        print_diff(dname1,"<",lines1.difference(lines2))
        print_diff(dname2,">",lines2.difference(lines1))
        if lines1 != lines2:
            print("")



if __name__=="__main__":
    import argparse 
    global args
    import sys,time

    parser = argparse.ArgumentParser(description="Perform regression testing on bulk_extractor")
    parser.add_argument("--gdb",help="run under gdb",action="store_true")
    parser.add_argument("--debug",help="specify debug level",type=int)
    parser.add_argument("--corp",help="regress entire corpus",action="store_true")
    parser.add_argument("--outdir",help="specifies output directory",default="regress")
    parser.add_argument("--exe",help="Executable to run (default {})".format(exe),
                        default=exe)
    parser.add_argument("--image",
                      help=("Specifies image to regress (default %s)" % default_infile),
                      default=default_infile)
    parser.add_argument("--jobs",help="Specifies number of jobs",type=int)
    parser.add_argument("--fast",help="Run with "+fast_infile,action="store_true")
    parser.add_argument("--full",help="Run with "+full_infile,action="store_true")
    parser.add_argument("--extra",help="Specify extra arguments")
    parser.add_argument("--gprof",help="Recompile and run with gprof",action="store_true")
    parser.add_argument("--diff",help="diff mode. Compare two outputs",type=str,nargs='*')
    parser.add_argument("--max",help="Maximum number of differences to display",default="5")
    parser.add_argument("--memdebug",help="Look for memory errors",action="store_true")
    parser.add_argument("--analyze",help="Specifies a bulk_extractor output file to analyze")
    parser.add_argument("--linebyline",help="Specifies a bulk_extractor output file to analyze line by line")
    parser.add_argument("--save",help="Saves analysis for this image in regress.db",action="store_true")
    parser.add_argument("--zip",help="Create a zip archive of the report")
    parser.add_argument("--validate",help="Validate the contents of a report (do not run bulk_extractor)",
                        type=str,nargs='*')
    parser.add_argument("--sort",help="Sort the feature files",type=str,nargs='*')
    parser.add_argument("--reproduce",help="specifies a bulk_extractor output "
            + "file from a crash and produces bulk_extractor flags to quickly "
            + "reproduce the crash")

    args = parser.parse_args()

    # these are mostly for testing
    if args.validate:
        for v in args.validate:
            validate_report(v)
        exit(0)
    if args.analyze:
        import xml.parsers.expat
        try:
            analyze_outdir(args.analyze);
            exit(0)
        except xml.parsers.expat.ExpatError as e:
            print("%s does not contain a valid report.xml file" % (args.analyze))
            print(e)
        print("Attempting line-by-line analysis...\n");
        analyze_linebyline(args.analyze)
        exit(0)
    if args.reproduce:
        print(reproduce_flags(args.reproduce))
        exit(0)
    if args.linebyline:
        analyze_linebyline(args.linebyline)
        exit(0)

    if args.zip    : make_zip(args.zip); exit(0)

    if not os.path.exists(args.exe):
        raise RuntimeError("{} does not exist".format(args.exe))

    # Find the bulk_extractor version and add it to the outdir
    version = Popen([args.exe,'-V'],stdout=PIPE).communicate()[0].decode('utf-8').split(' ')[1].strip()
    args.outdir += "-"+version

    drives = os.getenv("DOMEX_CORP") + "/nps/drives/"

    if args.fast:
        args.image  = find_file(drives + fast_infile)
        args.outdir += "-fast"
    elif args.full:
        args.image  = find_file(drives+full_infile)
        args.outdir += "-full"
    else:
        args.image  = find_file(drives+default_infile)
        

    if not args.fast and not args.full:
        args.outdir += "-norm"

    if args.memdebug:
        fn = "/usr/lib/libgmalloc.dylib"
        if os.path.exist(fn):
            print("Debugging with libgmalloc")
            os.putenv("DYLD_INSERT_LIBRARIES",fn)
            os.putenv("MALLOC_PROTECT_BEFORE","1")
            os.putenv("MALLOC_FILL_SPACE","1")
            os.putenv("MALLOC_STRICT_SIZE","1")
            os.putenv("MALLOC_CHECK_HEADER","1")
            os.putenv("MALLOC_PERMIT_INSANE_REQUESTS","1")
            run(args)
            exit(0)

        print("Debugging with standard Malloc")
        os.putenv("MallocLogFile","malloc.log")
        os.putenv("MallocGuardEdges","1")
        os.putenv("MallocStackLogging","1")
        os.putenv("MallocStackLoggingDirectory",".")
        os.putenv("MallocScribble","1")
        os.putenv("MallocCheckHeapStart","10")
        os.putenv("MallocCheckHeapEach","10")
        os.putenv("MallocCheckHeapAbort","1")
        os.putenv("MallocErrorAbort","1")
        os.putenv("MallocCorruptionAbort","1")
        run(args)
        exit(0)

    if args.gprof:
        call(['make','clean'])
        call(['make','CFLAGS=-pg','CXXFLAGS=-pg','LDFLAGS=-pg'])
        outdir = run(args)
        call(['gprof',program,"gmon.out"],stdout=open(outdir+"/GPROF.txt","w"))

    if args.corp:
        outdir_base = "/Volumes/Drobo1/bulk_extractor_regression"
        for (dirpath,dirnames,filenames) in os.walk(os.getenv("DOMEX_CORP")):
            for filename in filenames:
                (root,ext) = os.path.splitext(filename)
                if filename in ['aff','E01','.001']:
                    args.image = os.path.join(dirpath,filename)
                    args.outdir = outdir_base + "/"+os.path.basename(fn)
                    run(args)
        exit(0)

    if args.diff:
        if len(args.diff)!=2:
            raise ValueError("--diff requires two arguments")
        diff(args.diff[0],args.diff[1])
        exit(0)
    if args.sort:
        for s in args.sort:
            sort_outdir(s)
        exit(0)

    outdir = make_outdir(args.outdir)
    run_outdir(outdir,args.gdb)
    sort_outdir(outdir)
    validate_report(outdir)
    analyze_outdir(outdir)
    print("Regression finished. Output in {}".format(outdir))


                    
        
