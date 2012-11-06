#include "bulk_extractor.h"
#include "histogram.h"
#include "beregex.h"


/* global alert_list and stop_list
 * These should probably become static class variables
 */
word_and_context_list alert_list;		/* shold be flagged */
word_and_context_list stop_list;		/* should be ignored */

/****************************************************************
 *** feature_recorder_set
 *** No mutex is needed for the feature_recorder_set because it is never
 *** modified after it is created, only the contained feature_recorders are modified.
 ****************************************************************/



const string feature_recorder_set::ALERT_RECORDER = "alerts";
feature_recorder  *feature_recorder_set::alert_recorder = 0;

/**
 * Create a properly functioning feature recorder set.
 */
feature_recorder_set::feature_recorder_set(const feature_file_names_t &feature_files,
					   const std::string &outdir_):
    flags(0),outdir(outdir_),frm(),Mstats(),scanner_stats()
{
    /* Create the requested feature files */
    for(set<string>::const_iterator it=feature_files.begin();it!=feature_files.end();it++){
	create_name(*it);
    }
}

/**
 * create a dummy feature_recorder_set with no output directory
 */
feature_recorder_set::feature_recorder_set(uint32_t flags_):
    flags(flags_),outdir(),frm(),Mstats(),scanner_stats()
{
    if(flags & DISABLED){
	create_name(ALERT_RECORDER);	// create a bogus alert recorder
	this->flags |=ONLY_ALERT;	// only return this one
	get_name(ALERT_RECORDER)->set_flag(feature_recorder::FLAG_DISABLED);
    }
}

feature_recorder_set::~feature_recorder_set()
{
    for(feature_recorder_map::iterator i = frm.begin();i!=frm.end();i++){
	delete i->second;
    }
}

void feature_recorder_set::flush_all()
{
    for(feature_recorder_map::iterator i = frm.begin();i!=frm.end();i++){
	i->second->flush();
    } 
}

void feature_recorder_set::close_all()
{
    for(feature_recorder_map::iterator i = frm.begin();i!=frm.end();i++){
	i->second->close();
    } 
}


bool feature_recorder_set::has_name(string name) const
{
    return frm.find(name) != frm.end();
}

/*
 * Gets a feature_recorder_set.
 */
feature_recorder *feature_recorder_set::get_name(string name) const
{
    if(flags & ONLY_ALERT){		// always return the alert recorder
	name = feature_recorder_set::ALERT_RECORDER;
    }

    feature_recorder_map::const_iterator it = frm.find(name);
    if(it!=frm.end()) return it->second;
    std::cerr << "feature_recorder::get_name(" << name << ") does not exist\n";
    assert(0);
    exit(0);
}


void feature_recorder_set::create_name(string name)
{
    feature_recorder *fr = new feature_recorder(outdir,name);

    frm[name] = fr;
    if(stop_list.size()>0){
	string name_stopped = name+"_stopped";

	fr->stop_list_recorder = new feature_recorder(outdir,name_stopped);
	frm[name_stopped] = fr->stop_list_recorder;
    }
    
    if(flags & DISABLED) return;	// don't open if we are disabled

    /* Open the output!*/
    fr->open();
    if(fr->stop_list_recorder) fr->stop_list_recorder->open();
}

feature_recorder *feature_recorder_set::get_alert_recorder()  const
{
    return get_name(feature_recorder_set::ALERT_RECORDER);
}

void feature_recorder_set::add_stats(string bucket,double seconds)
{
    cppmutex::lock lock(Mstats);
    class pstats &p = scanner_stats[bucket]; // get the location of the stats
    p.seconds += seconds;
    p.calls ++;
}

void feature_recorder_set::dump_stats(struct xml &x)
{
    x.push("scanner_times");
    for(scanner_stats_map::const_iterator it = scanner_stats.begin();it!=scanner_stats.end();it++){
	x.set_oneline(true);
	x.push("path");
	x.xmlout("name",(*it).first);
	x.xmlout("calls",(int64_t)(*it).second.calls);
	x.xmlout("seconds",(*it).second.seconds);
	x.pop();
	x.set_oneline(false);
    }
    x.pop();
}

