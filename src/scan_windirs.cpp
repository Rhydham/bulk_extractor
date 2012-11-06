/**
 * Plugin: scan_windirs
 * Purpose: scan for Microsoft directory and MFT structures
 * FAT32 directories always start on sector boundaries. 
 */

#include "bulk_extractor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sstream>

#include "utf8.h"

#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wredundant-decls"

/* We have a private version of these #include files in case the system one is not present */
#include "tsk3/libtsk.h"
#include "tsk3/fs/tsk_fatfs.h"
#include "tsk3/fs/tsk_ntfs.h"

/**
 * code from tsk3
 */

using namespace std;

#if 0
bool static fat16charvalid[256];
const u_char *valid_fat16charvalid = (const u_char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789"
    "! # $ % & ' ( ) - @ ^ _ ` { } ~";


fat16chars_init()
{
	/* Set up the valid fat16 filename characters, per wikipedia */
	memset(fat16charvalid,0,sizeof(fat16charvalid));
	for(const u_char *cc=valid_fat16charvalid;*cc;cc++){fat16charvalid[*cc] = true;}
	for(int i=128;i<=255;i++){fat16charvalid[i]=true;}
}
#endif

inline uint16_t fat16int(const uint8_t buf[2]){
    return buf[0] | (buf[1]<<8);
}

inline uint32_t fat32int(const uint8_t buf[4]){
    return buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24);
}

inline uint32_t fat32int(const uint8_t high[2],const uint8_t low[2]){
    return low[0] | (low[1]<<8) | (high[2]<<16) | (high[3]<<24);
}


int fatYear(int x){ return (x & FATFS_YEAR_MASK) >> FATFS_YEAR_SHIFT;}
int fatMonth(int x){ return (x & FATFS_MON_MASK) >> FATFS_MON_SHIFT;}
int fatDay(int x){ return (x & FATFS_DAY_MASK) >> FATFS_DAY_SHIFT;}
int fatHour(int x){ return (x & FATFS_HOUR_MASK) >> FATFS_HOUR_SHIFT;}
int fatMin(int x){ return (x & FATFS_MIN_MASK) >> FATFS_MIN_SHIFT;}
int fatSec(int x){ return (x & FATFS_SEC_MASK) >> FATFS_SEC_SHIFT;}

std::string fatDateToISODate(const uint16_t d,const uint16_t t)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
	     fatYear(d)+1980,fatMonth(d),fatDay(d),
	     fatHour(t),fatMin(t),fatSec(t)); // local time
    return std::string(buf);
}



/* validate an 8.3 name */
bool valid_fat_dentry_name(const uint8_t name[8],const uint8_t ext[3])
{
    if( name[0]=='.' && name[1]==' ' && name[2]==' ' && name[3]==' '
	&& name[4]==' ' && name[5]==' ' && name[6]==' ' && name[7]==' '
	&& ext[0]==' ' && ext[1]==' ' && ext[2]==' ') return true;

    if( name[0]=='.' && name[1]=='.' && name[2]==' ' && name[3]==' '
	&& name[4]==' ' && name[5]==' ' && name[6]==' ' && name[7]==' '
	&& ext[0]==' ' && ext[1]==' ' && ext[2]==' ') return true;

    for(int i=0;i<8;i++){
	if(!FATFS_IS_83_NAME(name[i])) return false; // invalid name
    }

    for(int i=0;i<3;i++){
	if(!FATFS_IS_83_EXT(ext[i])) return false; // invalid exension
    }
    return true;
}


/**
 * Return 0 if the directory is invalid
 * 1 if the directory is valid dentry
 * 2 if the directory is valid LFN
 * 10 if the directory is valid and it's the last in the sector.
 * 20 if all null, so there are no more valid
 *
 * http://en.wikipedia.org/wiki/File_Allocation_Table
 */
int valid_fat_directory_entry(const sbuf_t &sbuf)
{
    if(sbuf.bufsize != sizeof(fatfs_dentry)) return 0; // not big enough
    /* If the entire directory entry is the same character, it's not valid */
    if(sbuf.is_constant(sbuf[0])) return 20; // clearly not valid

    const fatfs_dentry &dentry = *(sbuf.get_struct_ptr<fatfs_dentry>(0));
    if((dentry.attrib & ~FATFS_ATTR_ALL) != 0) return 0; // invalid attribute bit set
    if(dentry.attrib == FATFS_ATTR_LFN){
	/* This may be a VFAT long file name */
	const fatfs_dentry_lfn &lfn = *(const fatfs_dentry_lfn *)sbuf.buf;
	if((lfn.seq & ~0x40) > 10) return 0;	// invalid sequence number
	if(lfn.reserved1 != 0) return 0; // invalid reserved1 (LDIR_Type)
	if(fat16int(lfn.reserved2)!=0) return 0; // LDIR_FstClusLO "Must be ZERO"
	return 2;				 // looks okay
    } else {
	if(dentry.name[0]==0) return 10; // "Entry is available and no subsequent entry is in use. "

	/* Look for combinations of times, dates and attributes that have been invalid */
	if((dentry.attrib & FATFS_ATTR_LFN)==FATFS_ATTR_LFN &&
	   (dentry.attrib != FATFS_ATTR_LFN)){
	    return 0;			// LFN set but DIR or ARCHIVE is also set
	}
	if((dentry.attrib & FATFS_ATTR_DIRECTORY) && (dentry.attrib & FATFS_ATTR_ARCHIVE)){
	    return 0;			// can't have both DIRECTORY and ARCHIVE set
	}

	if(!valid_fat_dentry_name(dentry.name,dentry.ext)) return 0; // invalid name
	if(dentry.ctimeten>199) return 0;	// create time fine resolution, 0..199
	uint16_t ctime = fat16int(dentry.ctime);
	uint16_t cdate = fat16int(dentry.cdate);
	uint16_t adate = fat16int(dentry.adate);
	uint16_t wtime = fat16int(dentry.wtime);
	uint16_t wdate = fat16int(dentry.wdate);
	if(ctime && !FATFS_ISTIME(ctime)) return 0; // ctime is null for directories
	if(cdate && !FATFS_ISDATE(cdate)) return 0; // cdate is null for directories
	if(adate && !FATFS_ISDATE(adate)) return 0; // adate is null for directories
	if(adate==0 && ctime==0 && cdate==0){
	    if(dentry.attrib & FATFS_ATTR_VOLUME) return 1; // volume name
	    return 0;					    // not a volume name
	}
	if(!FATFS_ISTIME(wtime)) return 0; // invalid wtime
	if(!FATFS_ISDATE(wdate)) return 0; // invalid wdate
	if(ctime && ctime==cdate) return 0; // highly unlikely
	if(wtime && wtime==wdate) return 0; // highly unlikely
	if(adate && adate==ctime) return 0; // highly unlikely
	if(adate && adate==wtime) return 0; // highly unlikely
    }
    return 1;
}


void scan_fatdirs(const sbuf_t &sbuf,feature_recorder *wrecorder)
{
    /* 
     * Directory structures are 32 bytes long and will always be sector-aligned.
     * So try every 512 byte sector, within that try every 32 byte record.
     */
    
    for(size_t base = 0;base<sbuf.pagesize;base+=512){
	sbuf_t sector(sbuf,base,512);
	if(sector.bufsize < 512){
	    return;			// no space left
	}

	int last_valid_entry_number = -1;
	int ret1_count = 0;
	int valid_year_count = 0;
	const int max_entries = 512/32;
	int slots[max_entries];
	memset(slots,0,sizeof(slots));
	for(ssize_t entry_number = 0; entry_number < max_entries; entry_number++){
	    sbuf_t n(sector,entry_number*32,32);
	    
	    int ret = valid_fat_directory_entry(n);
	    if(ret==20) break;		// no more valid
	    slots[entry_number] = ret;
	    if(ret==1){
		/* Attempt to validate the years */
		const fatfs_dentry &dentry = *n.get_struct_ptr<fatfs_dentry>(0);
		uint16_t ayear = fatYear(fat16int(dentry.adate));
		uint16_t cyear = fatYear(fat16int(dentry.cdate));
		uint16_t wyear = fatYear(fat16int(dentry.wdate));

		if( (ayear==0 || ((int)1980+ayear < (int)opt_last_year))
		    && (cyear==0 || ((int)1980+cyear < (int)opt_last_year))
		    && ((int)1980+wyear < (int)opt_last_year)){
			valid_year_count++;
		}
		ret1_count++;
	    }
	    if(ret==0){			// invalid; they are all bad
		//last_valid_entry_number = -1; // found an invalid directory entry
		break;
	    }
	    if(ret==1 || ret==2){	// valid; go to the next
		last_valid_entry_number = entry_number;
		continue;
	    }
	    if(ret==10){		// valid; no more remain
		last_valid_entry_number = entry_number;
		break;		
	    }
	}
	/* Now print the valid entry numbers */
	if(ret1_count==1 && valid_year_count==0) continue; // year is bogus
	if(last_valid_entry_number==1 && valid_year_count==0) continue; // year is bogus
	if(last_valid_entry_number>=0 && ret1_count>0){
	    for(ssize_t entry_number = 0;entry_number <= last_valid_entry_number && entry_number<max_entries;
		entry_number++){
		sbuf_t n(sector,entry_number*32,32);
		xml::strstrmap_t fatmap;
		
		if(valid_fat_directory_entry(n)==1){
		    const fatfs_dentry &dentry = *n.get_struct_ptr<fatfs_dentry>(0);
		    std::stringstream ss;
		    for(int j=0;j<8;j++){ if(dentry.name[j]!=' ') ss << dentry.name[j]; }
		    ss << ".";
		    for(int j=0;j<3;j++){ if(dentry.ext[j]!=' ') ss << dentry.ext[j]; }
		    std::string filename = ss.str();
		    fatmap["filename"] = filename;
		    fatmap["ctimeten"] = itos(dentry.ctimeten);
		    fatmap["ctime"]    = fatDateToISODate(fat16int(dentry.cdate),fat16int(dentry.ctime));
		    fatmap["atime"]    = fatDateToISODate(fat16int(dentry.adate),0);
		    fatmap["mtime"]    = fatDateToISODate(fat16int(dentry.wdate),fat16int(dentry.wtime));
		    fatmap["startcluster"] = itos(fat32int(dentry.highclust,dentry.startclust));
		    fatmap["filesize"] = itos(fat32int(dentry.size));
		    fatmap["attrib"]   = itos(dentry.attrib);
		    wrecorder->write(n.pos0,filename,xml::xmlmap(fatmap,"fileobject","src='fat'"));
		}
	    }
	}
    }
}

/**
 * Examine an sbuf and see if it contains an NTFS MFT entry. If it does, then process the entry 
 */
void scan_ntfsdirs(const sbuf_t &sbuf,feature_recorder *wrecorder)
{
    for(size_t base = 0;base<sbuf.pagesize;base+=512){
	sbuf_t n(sbuf,base,1024);
	std::string filename;
	if(n.bufsize!=1024){
	    continue;	// no space
	}
	try{
	    if(n.get32u(0)==NTFS_MFT_MAGIC){ // NFT magic number matches
		if(debug & DEBUG_INFO) n.hex_dump(std::cerr);

		uint16_t nlink = n.get16u(16); // get link count
		if(nlink<10){ // sanity check - most files have less than 10 links

		    xml::strstrmap_t mftmap;
		    mftmap["nlink"] = itos(nlink);
		    mftmap["lsn"]   = itos(n.get64u(8)); // $LogFile Sequence Number
		    mftmap["seq"]   = itos(n.get16u(18));
		    size_t attr_off = n.get16u(20); // don't make 16bit!

		    // Now look at every attribute for the ones that we care about

		    int found_attrs = 0;
		    while(attr_off+sizeof(ntfs_attr) < n.bufsize){

			uint32_t attr_type = n.get32u(attr_off+0);
			uint32_t attr_len  = n.get32u(attr_off+4);

			if(debug & DEBUG_INFO){
			    std::cerr << "---------------------\n";
			    n.hex_dump(std::cerr,attr_off,128);
			    std::cerr << " attr_off=" << attr_off << " attr_type=" << attr_type
				      << " attr_len=" << attr_len;
			}
		    
			if(attr_len==0){
			    if(debug & DEBUG_INFO) std::cerr << "\n";
			
			    break;	// something is wrong; skip this entry
			}
		    
			// get the values for all entries
			int  res         = n.get8u(attr_off+8);
			size_t nlen      = n.get8u(attr_off+9);
			size_t name_off  = n.get16u(attr_off+10);
			uint32_t mft_flags = n.get16u(attr_off+12);
			uint32_t id        = n.get16u(attr_off+14);

			if(debug & DEBUG_INFO){
			    std::cerr << " res=" << (int)res << " nlen=" << (int)nlen << " name_off="
				      << name_off << " mft_flags="<< mft_flags << " id=" << id << "\n";
			}

			if(res!=NTFS_MFT_RES){ // we can only handle resident attributes
			    attr_off += attr_len;
			    continue;
			}

			if(attr_type==NTFS_ATYPE_SI){
			    found_attrs++;
			    if(debug & DEBUG_INFO) std::cerr << "NTFS_ATYPE_SI ignored\n";
			}
		    
			if(attr_type==NTFS_ATYPE_ATTRLIST){
			    found_attrs++;
			    if(debug & DEBUG_INFO) std::cerr << "NTFS_ATTRLIST ignored\n";
			}
		    
			if(attr_type==NTFS_ATYPE_FNAME ){ 
			    found_attrs++;
			    if(debug & DEBUG_INFO) std::cerr << "NTFS_ATYPE_FNAME\n";

			    // Decode a resident FNAME fields
			    // Previously all of the get16u's were put into uint16_t, but that
			    // turned out to cause overflow problems, so don't do that.

			    size_t soff         = n.get16u(attr_off+20);

			    mftmap["par_ref"] = utos(n[attr_off+soff+0]
						     | (n[attr_off+soff+1]<<8)
						     | (n[attr_off+soff+2]<<16)
						     | (n[attr_off+soff+3]<<24)
						     | ((uint64_t)n[attr_off+soff+4]<<32)
						     | ((uint64_t)n[attr_off+soff+5]<<40));
			    mftmap["par_seq"] = utos(n.get16u(attr_off+soff+6));
			    mftmap["crtime"] = microsoftDateToISODate(n.get64u(attr_off+soff+8));
			    mftmap["mtime"]  = microsoftDateToISODate(n.get64u(attr_off+soff+16));
			    mftmap["ctime"]  = microsoftDateToISODate(n.get64u(attr_off+soff+24));
			    mftmap["atime"]  = microsoftDateToISODate(n.get64u(attr_off+soff+32));

			    // these can be sanity checked

			    static const uint64_t terabyte = uint64_t(1000) * uint64_t(1000) * uint64_t(1000) * uint64_t(1000);
			    uint64_t filesize_alloc = n.get64u(attr_off+soff+40);
			    if(filesize_alloc > (1000L * terabyte)) break;

			    mftmap["filesize_alloc"] = utos(filesize_alloc);

			    uint64_t filesize = n.get64u(attr_off+soff+48);
			    if(filesize > (1000L * terabyte)) break;
			    mftmap["filesize"]       = utos(terabyte);

			    mftmap["attr_flags"]     = utos(n.get64u(attr_off+soff+56));
			    size_t  fname_nlen   = n.get8u(attr_off+soff+64);
			    size_t  fname_nspace = n.get8u(attr_off+soff+65);
			    size_t  fname_npos   = attr_off+soff+66;

			    if(debug & DEBUG_INFO) std::cerr << " soff=" << soff << " fname_nlen=" << fname_nlen
							     << " fname_nspace=" << fname_nspace
							     << " fname_npos=" << fname_npos
							     << " (" << fname_npos-attr_off << "-attr_off) "
							     << "\n";

			    std::wstring utf16str;
			    for(size_t i=0;i<fname_nlen;i++){ // this is pretty gross; is there a better way?
				utf16str.push_back(n.get16u(fname_npos+i*2));
			    }
			    filename = safe_utf16to8(utf16str);
			    mftmap["filename"] = filename;
			}
		    
			attr_off += attr_len;
		    }		
		    if(mftmap.size()>3){
			if(filename.size()==0) filename="$NOFILENAME"; // avoids problems
			wrecorder->write(n.pos0,filename,xml::xmlmap(mftmap,"fileobject","src='mft'"));
		    }
		    if(debug & DEBUG_INFO) std::cerr << "=======================\n";
		}
	    }
	    //const ntfs_mft &mft = *(const ntfs_mft *)sbuf.buf;
	}
	catch ( sbuf_t::range_exception_t &e ){
	    /**
	     * If we got a range exception, then the region we were reading
	     * can't be a valid MFT entry...
	     */
	    continue;			
	}
    }
}


extern "C"
void scan_windirs(const class scanner_params &sp,const recursion_control_block &rcb)
{
    string myString;
    assert(sp.sp_version==scanner_params::CURRENT_SP_VERSION);
    if(sp.phase==scanner_params::startup){
        assert(sp.info->si_version==scanner_info::CURRENT_SI_VERSION);
	sp.info->name		= "windirs";
        sp.info->author         = "Simson Garfinkel";
        sp.info->description    = "Scans Microsoft directory structures";
        sp.info->scanner_version= "1.0";
	sp.info->feature_names.insert("windirs");
	//sp.info->flags = scanner_info::SCANNER_DISABLED; // disabled until it's working
	return;
    }
    if(sp.phase==scanner_params::shutdown) return;		// no shutdown
    if(sp.phase==scanner_params::scan){
	feature_recorder *wrecorder = sp.fs.get_name("windirs");
	scan_fatdirs(sp.sbuf,wrecorder);
	scan_ntfsdirs(sp.sbuf,wrecorder);
    }
}
