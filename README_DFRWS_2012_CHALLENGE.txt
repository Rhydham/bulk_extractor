    Naval Postgraduate School Digital Evaluation and Exploitation
			      (NPS-DEEP)

		   DFRWS 2012 CHALLENGE SUBMISSION

			    June 30, 2012


Attached please find our DFRWS 2012 challenge submission.

1. We are entering as a team. The team members are:
  Simson Garfinkel (team leader)
  Bruce Allen
  Alex Eubanks
  Kristina Foster
  Tony Melaragno
  Joel Young
  Siddharth Gopal (*)
  Yiming Yang (*)
  Konstantin Salomatin (*)
  Jamie Carbonell (*)

(*) Provided the LIFT file type identification system, developed under DOD contract in 2011.

2. Our tool has a command line interface and will work out of the box
on MacOS and Linux.  

3. The tool has a corresponding API that can be incorporated as part
of other tools. The library is the bulk_extractor plug-in API. This
API is documented in the file src/bulk_extractor.h and
src/bulk_extractor_i.h. Briefly, the scanners provided by the tool can
be linked into any C++ program and called.

We have also provided a python API using the python script
dfrws_2012_challenge.py which is provided. This API is demonstrated by
the program dfrws_2012_api_demo.py.  However this API is very
inefficient and should only be used for demonstration purposes.


3. The source code is openly available (it is included with the tool)
and is distributed either as Public Domain (#PublicDomain) or under a
recognized open source license.

4. Third-party free software has been incorporated, specifically:

   * hibernation file decompression from Matthieu Suiche (updated by
     Simson Garfinkel)

   * FAT32 and NTFS data structures from SleuthKit

   * BASE64 and BASE85 decoding.

   * Gnu Flex

   * utf8.h implementation by Nemanja Trifunovic


5. Instructions for building the tool:

You must compile our tool in order to use it. The tool is provided as a 
file called bulk_extractor-1.3b1.tar.gz.  To compile the tool please execute these commands:

     $ tar xfvz bulk_extractor-1.3b1.tar.gz
     $ cd bulk_extractor-1.3b1
     $ ./configure
     $ make


Relevant dependencies:

* pthreads must be installed
* zlib developer libraries must be installed
* libewf must be installed to read E01 drive images. 


6. After the tool is built, it can be run from the "src" directory:

     $ cd src
     $ ./data_sniffer <target> <block_size> [<concurrency_factor>]

