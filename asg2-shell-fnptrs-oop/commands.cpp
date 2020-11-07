// $Id: commands.cpp,v 1.19 2020-10-20 18:23:13-07 - - $
// Juwon Kim, jkim419, asg2
// executing file for commands

#include "commands.h"
#include "debug.h"
#include <iostream>
#include <iomanip>

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": command not found");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
      throw command_error (words[0] + ": needs pathname");
   } else{
      bool found;
      size_t len = words.size();
      for(size_t i = 1; i < len; i++){
         map<string, inode_ptr> dirents = state.getcwd()->getcontents()
            ->getdirents();
         for(auto j = dirents.begin(); j != dirents.end(); ++j){
            if(j->first == words[i]){
               wordvec output = j->second->getcontents()->readfile();
               size_t p = output.size();
               for(size_t k = 0; k < p; k++){
                  cout << output[k] << " ";
               }
               cout << endl;
               found = true;
               break;
            }
         }
         if(found != true){
            exec::status(1);
            throw command_error (words[0] + ": " + words[i] + 
               ": No such file or directory");
         }
         found = false;
      }
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
      inode_ptr root = state.getroot();
      state.setcwd(root);
   } else if (words.size() == 2){ 
      if(words[1] == "/"){
         state.setcwd(state.getroot());
      } else{
         map<string, inode_ptr> dirents = state.getcwd()->getcontents()
            ->getdirents();
         for(auto i = dirents.begin(); i != dirents.end(); ++i){
            if(i->first == words[1]){
               state.setcwd(i->second);
               break;
            }
         } 
      }   
   } else{
      throw command_error (words[0] + ": too many operands");
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 2){
      exec::status(stoi(words[1]));
   } 
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
      map<string, inode_ptr> dirents= state.getcwd()->getcontents()
         ->getdirents();
      if(state.getcwd() == state.getroot()){ 
         cout << "/";
      }
      cout << state.getcwd()->getcontents()->getpath() << ":" <<
         endl;
      for(auto i = dirents.begin(); i != dirents.end(); ++i){
         cout << right << setw(6) << i->second->get_inode_nr();
         cout << right << setw(6) << i->second->getcontents()->size(); 
         cout << "  " << i->first;
         if(i->second->getcontents()->gettype() == "directory"){
            cout << "/";
         }
         cout << endl;
      }
   } else if(words.size() == 2){
      if(words[1] == "/"){
         map<string, inode_ptr> dirents= state.getroot()->
            getcontents()->getdirents();
         if(state.getcwd() == state.getroot()){ 
            cout << "/";
         }
         cout << state.getcwd()->getcontents()->getpath() << ":" << 
            endl;
         for(auto i = dirents.begin(); i != dirents.end(); ++i){
            cout << right << setw(6) << i->second->get_inode_nr();
            cout << right << setw(6) << i->second->getcontents()
               ->size(); 
            cout << "  " << i->first;
            if(i->second->getcontents()->gettype() == "directory"){
               cout << "/";
            }
            cout << endl;
         } 
      } else{
         map<string, inode_ptr> find = state.getcwd()->getcontents()
            ->getdirents();
         cout << state.getcwd()->getcontents()->getpath() << ":" <<
            endl;
         for(auto i = find.begin(); i != find.end(); ++i){
            if(i->first == words[1]){
               map<string, inode_ptr> dirents = i->second->getcontents()
                  ->getdirents();
               for(auto j = dirents.begin(); j != dirents.end(); ++j){
                 cout << right << setw(6) << j->second->get_inode_nr();
                 cout << right << setw(6) << j->second->getcontents()
                    ->size(); 
                 cout << "  " << j->first;
                  if(j->second->getcontents()->gettype() == 
                    "directory"){
                     cout << "/";
                  }
                  cout << endl;
               }
            }
         }
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   if(words.size() == 1){
      throw command_error(words[0] + ": missing operand");
   }
   inode_ptr temp = state.getcwd();
   state.setcwd(state.getroot());
   wordvec lsw;
   lsw.push_back("ls");
   lsw.push_back(words[1]);
   fn_ls(state, lsw);
   state.setcwd(temp);
   wordvec pathname = split(words[1], "/");
   size_t len = pathname.size();
   if(len > 0){
      inode_ptr stop;
      inode_ptr now;
      now = state.getroot();
      map<string,inode_ptr> rootd;
      for(size_t i = 0; i < len; i++){
         rootd = now->getcontents()->getdirents();
         for(auto j = rootd.begin(); j != rootd.end(); j++){
            if(j->first == pathname[i]){
               if(i < len -1){
                  stop = j->second;
                  rootd = j->second->getcontents()->getdirents();
                  now = stop;
               } else{
                  stop = j->second;
               }
            }
         }   
      }
      if(stop->getcontents()->gettype() == "directory"){
         if(stop->getcontents()->getdirents().size() > 2){
            map<string,inode_ptr> subd;
            for(auto k = subd.begin(); k != subd.end(); k++){
               if(k->first != (".") and k->first != ("..")){
                  string word = words[1];
                  word += "/" + k->first;
                  wordvec newvec;
                  newvec.push_back("lsr");
                  newvec.push_back(word);
                  fn_lsr(state, newvec);
               }
            }
         }
      }   
   } else{
      map<string,inode_ptr> rootd;
      rootd = state.getroot()->getcontents()->getdirents();
      for(auto j = rootd.begin(); j != rootd.end(); j++){
         if(j->first != (".") and j->first != ("..")){
            if(j->second->getcontents()->gettype() == 
               "directory"){
               string word = "/" + j->first;
               wordvec newvec;
               newvec.push_back("lsr");
               newvec.push_back(word);
               fn_lsr(state, newvec);
            }
         }
      }   
   }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
      throw command_error("Improper execution of make");
   } 
   wordvec pathname = split(words[1], "/");
   size_t len = pathname.size();
   if(len > 1){
      inode_ptr stop;
      inode_ptr now;
      bool found;
      now = state.getroot();
      map<string,inode_ptr> rootd;
      for(size_t i = 0; i < len - 1; i++){
         rootd = now->getcontents()->getdirents();
         for(auto j = rootd.begin(); j != rootd.end(); j++){
            if(j->first == pathname[i]){
               if(i < len - 2){
                  stop = j->second;
                  rootd = j->second->getcontents()->getdirents();
                  now = stop;
               } else{
                  stop = j->second;      
               }
               found = true;
            }
         }
         if(found != true){
             throw command_error(words[0] + ": pathway doesn't exist");
         }
         found = false;
      }
      map<string,inode_ptr> find;
      find = stop->getcontents()->getdirents();
      for(auto k = find.begin(); k !=find.end(); k++){
         if(k->first == pathname[len-1]){
            if(k->second->getcontents()->gettype() ==
               "directory"){
            throw command_error(words[0] + ": " +words[1] + 
               " is a directory");
            }
         }
      }
      inode_ptr newfile = stop->getcontents()->mkfile(words[1]);
      now->getcontents()->getdirents().insert(pair<string,
      inode_ptr>(words[1], newfile));
   }
   inode_ptr newfile = state.getcwd()->getcontents()->mkfile(words[1]);
   state.getcwd()->getcontents()->getdirents().insert(pair<string,
      inode_ptr>(words[1], newfile));
   if(words.size() > 2){
      newfile->getcontents()->writefile(words);
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
      throw command_error (words[0] + ": missing operand");
   } 
   wordvec pathname = split(words[1], "/");
   size_t len = pathname.size();
   if(len > 1){
      bool found;
      inode_ptr stop;
      inode_ptr now;
      now = state.getroot();
      map<string,inode_ptr> rootd;
      for(size_t i = 0; i < len - 1; i++){
         rootd = now->getcontents()->getdirents();
         for(auto j = rootd.begin(); j != rootd.end(); j++){
            if(j->first == pathname[i]){
               if(i < len - 2){
                  stop = j->second;
                  rootd = j->second->getcontents()->getdirents();
                  now = stop;
               } else{
                  stop = j->second;      
               }
               found = true;
            }
         }
         if(found != true){
             throw command_error(words[0] + ": pathway doesn't exist");
         }
         found = false;
      }
      inode_ptr newdir = stop->getcontents()->mkdir(pathname[len - 1]);
      string newpath = stop->getcontents()->getpath() 
         + "/" + pathname[len - 1];
      newdir->getcontents()->setpath(newpath);
      newdir->getcontents()->getdirents().insert(pair<string, 
         inode_ptr>("..", now));
      stop->getcontents()->getdirents().insert(pair<string,
         inode_ptr>(pathname[len-1], newdir));
   } else {
      inode_ptr newdir = state.getcwd()->getcontents()->mkdir(words[1]);
      string newpath = state.getcwd()->getcontents()->getpath() 
         + "/" + words[1];
      newdir->getcontents()->setpath(newpath);
      newdir->getcontents()->getdirents().insert(pair<string, 
         inode_ptr>("..", state.getcwd()));
      state.getcwd()->getcontents()->getdirents().insert(pair<string,
         inode_ptr>(words[1], newdir));
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.prompt(words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(state.getcwd() == state.getroot()){
      cout<< "/" << endl;
   } else {
      cout << state.getcwd()->getcontents()->getpath() << endl;
   }
}

void fn_rm (inode_state& state, const wordvec& words){
   if(words.size() == 1){
       throw command_error (words[0] + ": missing operand");
   } else if(words.size() > 2){ 
       throw command_error (words[0] + ": too many operands");
   } 
   wordvec pathname = split(words[1], "/");
   size_t len = pathname.size();
   bool found;
   inode_ptr stop;
   inode_ptr now;
   now = state.getroot();
   map<string,inode_ptr> rootd;
   for(size_t i = 0; i < len; i++){
      rootd = now->getcontents()->getdirents();
      for(auto j = rootd.begin(); j != rootd.end(); j++){
         if(j->first == pathname[i]){
            if(i < len -1){
               stop = j->second;
               rootd = j->second->getcontents()->getdirents();
               now = stop;
            } else{
               stop = j->second;
               found = true;
            }
         }
      }
   }
   if(found != true){
      throw command_error (words[0] + ": pathname not found");
   }
   if(stop->getcontents()->gettype() == "directory"){
      if(stop->getcontents()->getdirents().size() > 2){
         throw command_error (words[0] + ": directory is not empty");
      } else{
         now->getcontents()->eraseone(pathname[len-1]); 
         stop->getcontents()->getdirents().clear();
      }
   } else{
      now->getcontents()->eraseone(pathname[len-1]);
   }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   if(words.size()==1){
      throw command_error (words[0] + ": a pathname is required");
   }
   wordvec pathname = split(words[1], "/");
   size_t len = pathname.size();
   if(len > 0){
      inode_ptr stop;
      inode_ptr now;
      now = state.getroot();
      map<string,inode_ptr> rootd;
      for(size_t i = 0; i < len; i++){
         rootd = now->getcontents()->getdirents();
         for(auto j = rootd.begin(); j != rootd.end(); j++){
            if(j->first == pathname[i]){
               if(i < len -1){
                  stop = j->second;
                  rootd = j->second->getcontents()->getdirents();
                  now = stop;
               } else{
                  stop = j->second;
               }
            }
         }   
      }
      stop->getcontents()->rmr();
   } else{
      map<string,inode_ptr> rootd;
      rootd = state.getroot()->getcontents()->getdirents();
      for(auto j = rootd.begin(); j != rootd.end(); j++){
         if(j->first != (".") and j->first != ("..")){
            j->second->getcontents()->rmr();
         }
      }   
   }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}
