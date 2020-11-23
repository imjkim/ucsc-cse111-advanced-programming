// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>
#include <cassert>
#include <fstream>
#include <cerrno>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main (int argc, char** argv) { //most work
 
   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
  
 
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      sys_info::execname (argv[0]);
      scan_options (argc, argv);
      ifstream infile(*argp);
      if(infile.fail()){
         int status = 1;
         cerr << argp << ": " << strerror (errno) << endl;
         return status;
      } else{
         int status = 0;
         int num = 1;
         str_str_map map;
         for (;;){
            string line;
            getline (infile, line);
            cout << "-: " << num << ": " << line << endl;
            smatch result;
            if(infile.eof()) break;            //^D will return eof
            if (regex_search (line, result, comment_regex)){
               cout << endl;
            } else if (regex_search (line, result, key_value_regex)){
               str_str_pair pair(result[1], result[2]);
            if(result[1] == "" and result[2] == ""){       // =
               map.printAll();
            } else if(result[2] == ""){                    // key =
               map.erase(map.find(result[1]));
            } else if(result[1] == ""){
               for (str_str_map::iterator itor = map.begin();
                  itor != map.end(); ++itor) {
                  if(itor->second == result[2]){
                     cout << itor->first << " = " << result[2] << endl;
                  }
               }
            }else{
               map.insert(pair);
               cout << result[1] << " = " << result[2] << endl;
            }
         } else if (regex_search (line, result, trimmed_regex)) {
            if(map.find(result[1]) != map.end()){
               cout << (map.find(result[1]))->first << " = " 
                  << (map.find(result[1]))->second << endl;
               } else{
                  cout << result[1] << ": key not found" << endl;
               }
            }
            num++;
         }
         infile.close();
         return status;
      }
   }
}

