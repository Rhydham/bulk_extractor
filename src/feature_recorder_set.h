#ifndef FEATURE_RECORDER_SET_H
#include "feature_recorder.h"

/** \addtogroup internal_interfaces
 * @{
 */
/** \file */

/**
 * \class feature_recorder_set
 * A singleton class that holds a set of recorders.
 * This used to be done with a set, but now it's done with a map.
 * 
 */
#include <tr1/unordered_map>

//typedef tr1::unordered_map<string,class feature_recorder *> feature_recorder_map;
typedef map<string,class feature_recorder *> feature_recorder_map;
typedef set<string>feature_file_names_t;
class feature_recorder_set {
private:
    /*** neither copying nor assignment is implemented ***
     *** We do this by making them private constructors that throw exceptions. ***/
    class not_impl: public exception {
	virtual const char *what() const throw() {
	    return "copying feature_recorder objects is not implemented.";
	}
    };
    feature_recorder_set(const feature_recorder_set &fs):
	flags(0),outdir(),frm(),Mstats(),scanner_stats(){ throw new not_impl(); }
    const feature_recorder_set &operator=(const feature_recorder_set &fs){ throw new not_impl(); }
    uint32_t flags;
public:
    // instance data //
    static feature_recorder *alert_recorder;
    string outdir;			// where output goes
    feature_recorder_map  frm;		// map of feature recorders
    cppmutex Mstats;
    class pstats {
    public:
	double seconds;
	uint64_t calls;
    };
    //typedef tr1::unordered_map<string,class pstats> scanner_stats_map;
    typedef map<string,class pstats> scanner_stats_map;
    scanner_stats_map scanner_stats;


    static const string   ALERT_RECORDER;	// the name of the alert recorder
    static const uint32_t DISABLED=0x02;	// the set is effectively disabled
    static const uint32_t ONLY_ALERT=0x01;	// always return the alert recorder

    feature_recorder_set(const feature_file_names_t &feature_files,const std::string &outdir);
    feature_recorder_set(uint32_t flags);
    virtual ~feature_recorder_set();

    //void add_feature_files_for_scanners(scanner_vector &scanners);

    void flush_all();
    void close_all();
    bool has_name(string name) const;	/* does the named feature exist? */
    void create_name(string name);
    void set_flag(uint32_t f){flags|=f;}
    void clear_flag(uint32_t f){flags|=f;}
    void add_stats(string bucket,double seconds);
    void dump_stats(class xml &xml);


    // NOTE:
    // only virtual functions may be called by plugins!
    virtual feature_recorder *get_name(string name) const;
    virtual feature_recorder *get_alert_recorder() const;
};


#endif
