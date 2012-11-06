#include "bulk_extractor.h"
#include "beregex.h"
#include "word_and_context_list.h"

void word_and_context_list::add_regex(const string &pat)
{
    patterns.push_back(new beregex(pat,0));
}

/**
 * Insert a feature and context, but only if not already present.
 * Returns true if added.
 */
bool word_and_context_list::add_fc(const string &f,const string &c)
{
    context ctx(f,c);
    for(stopmap_t::const_iterator it = list.find(f);it!=list.end();it++){
	if((*it).second == ctx) return false;
    }
    list.insert(pair<string,context>(f,ctx));
    return true;
}

/** returns 0 if success, -1 if fail. */
int word_and_context_list::readfile(const string &filename)
{
    ifstream i(filename.c_str());
    if(!i.is_open()) return -1;
    printf("Reading context stop list %s\n",filename.c_str());
    string line;
    uint64_t total_context=0;
    uint64_t line_counter = 0;
    uint64_t features_read = 0;
    while(getline(i,line)){
	line_counter++;
	if(line.size()==0) continue;
	if(line_counter==1 && line.size()>3
	   && line[0]==feature_recorder::UTF8_BOM[0]
	   && line[1]==feature_recorder::UTF8_BOM[1]
	   && line[2]==feature_recorder::UTF8_BOM[2]){
	    line = line.substr(3);	// remove the UTF8 BOM
	}
	if(line[0]=='#') continue; // it's a comment
	if((*line.end())=='\r'){
	    line.erase(line.end());	/* remove the last character if it is a \r */
	}
	if(line.size()==0) continue;	// no line content
	++features_read;

	// If there are two tabs, this is a line from a feature file
	size_t tab1 = line.find('\t');
	if(tab1!=string::npos){
	    size_t tab2 = line.find('\t',tab1+1);
	    if(tab2!=string::npos){
		size_t tab3 = line.find('\t',tab2+1);
		if(tab3==string::npos) tab3=line.size();
		string f = line.substr(tab1+1,(tab2-1)-tab1);
		string c = line.substr(tab2+1,(tab3-1)-tab2);
		if(add_fc(f,c)){
		    ++total_context;
		}
	    } else {
		string f = line.substr(tab1+1);
		add_fc(f,"");		// Insert a feature with no context
	    }
	    continue;
	}

	// If there is no tab, then this must be a simple item to ignore.
	// If it is a regular expression, add it to the list of REs
	if(beregex::is_regex(line)){
	    patterns.push_back(new beregex(line,REG_ICASE));
	} else {
	    // Otherwise, add it as a feature with no context
	    list.insert(pair<string,context>(line,context(line)));
	}
    }
    std::cout << "Stop list read.\n";
    std::cout << "  Total features read: " << features_read << "\n";
    std::cout << "  List Size: " << list.size() << "\n";
    std::cout << "  Context Strings: " << total_context << "\n";
    std::cout << "  Regular Expressions: " << patterns.size() << "\n";
    return 0;
}

/** check() is threadsafe. */
bool word_and_context_list::check(const string &probe,const string &before,const string &after) const
{
    /* First check literals, because they are faster */
    for(stopmap_t::const_iterator it =list.find(probe);it!=list.end();it++){
	if((rstrcmp((*it).second.before,before)==0) &&
	   (rstrcmp((*it).second.after,after)==0) &&
	   ((*it).second.feature==probe)){
	    return true;
	}
    }

    /* Now check the patterns; do this second */
    for(beregex_vector::const_iterator it=patterns.begin(); it != patterns.end(); it++){
	if((*it)->search(probe,0,0,0)){
	    return true;		// yep
	}
    }
    return false;
};

bool word_and_context_list::check_feature_context(const string &probe,const string &context) const 
{
    string before;
    string after;
    context::extract_before_after(probe,context,before,after);
    return check(probe,before,after);
}

void word_and_context_list::dump()
{
    std::cout << "dump context list:\n";
    for(stopmap_t::const_iterator it =list.begin();it!=list.end();it++){
	std::cout << (*it).first << " = " << (*it).second << "\n";
    }
    std::cout << "dump RE list:\n";
    for(beregex_vector::const_iterator it=patterns.begin(); it != patterns.end(); it++){
	std::cout << (*it)->pat << "\n";
    }
}

#ifdef STAND
int  main(int argc,char **argv)
{
    cout << "testing contxt_list\n";
    word_and_context_list cl;
    while(--argc){
	argv++;
	if(cl.readfile(*argv)){
	    err(1,"Cannot read %s",*argv);
	}
    }
    cl.dump();
    exit(1);
}
#endif
