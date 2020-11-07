// $Id: file_sys.cpp,v 1.9 2020-10-26 21:32:08-07 - - $
// Juwon Kim, jkim419, asg2
// executing file for file system, implementing all the functions

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const{
      return static_cast<size_t> (type);
   }
};
ostream& operator<< (ostream& out, file_type type) {
   switch (type) {
      case file_type::PLAIN_TYPE: cout << "PLAIN_TYPE"; break;
      case file_type::DIRECTORY_TYPE: cout << "DIRECTORY_TYPE"; break;
      default: assert (false);
   };
   return out;
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
   root = make_shared<inode>(file_type :: DIRECTORY_TYPE);
   shared_ptr<directory> dirptr = make_shared<directory>();
   root->contents = dirptr;
   cwd = root;
   dirptr->getdirents().insert(pair<string,
      inode_ptr>(".",cwd));
   dirptr->getdirents().insert(pair<string,
      inode_ptr>("..",cwd));
}
inode_state::~inode_state(){
   root->getcontents()->rmr();
}
void directory::rmr(){
   map<string, inode_ptr> m = this->getdirents();
   for(auto a = m.begin(); a!= m.end(); a++){
      if(a->first != "." and a->first != ".."){
         a->second->getcontents()->rmr();
         dirents.erase(a->first);
      }
   }
}
const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

size_t inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


base_file_ptr inode::getcontents() {
   return contents;
}

void inode::setcontents(base_file_ptr ptr) {
   contents = ptr;
}

file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}


string plain_file::gettype() const{
   return error_file_type();
}
string directory::gettype() const{
   return error_file_type();
} 
size_t plain_file::size() const {
   size_t size {0};
   if(data.size() != 0){
      for(size_t i = 0; i < data.size(); i++){
        size += data[i].length();             //# bytes
      }
      size += data.size() - 1;
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) { 
   size_t len = words.size();
   for(size_t i = 2; i < len; i++){
      data.push_back(words[i]);
   }
   DEBUGF ('i', words);
}

size_t directory::size() const {
   size_t size {0};
   if(dirents.size() != 0){
      size = dirents.size();
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

void directory::setpath (const string& name) {
   DEBUGF ('i', name);
   path = name;
}

inode_ptr inode_state::getroot (){
   return root;
}
inode_ptr inode_state::getcwd (){
   return cwd;
}

void inode_state::setcwd (inode_ptr ptr) {
   cwd = ptr; 
}

void inode_state::prompt(const wordvec& s){
   auto len = s.size();
   string newstr;
   for(size_t i = 1; i < len; i++){ 
      newstr = newstr + (s[i]);
      newstr += " ";
   }
   prompt_ = newstr;
}
 
void directory::eraseone (const string& name) {
   dirents.erase(name);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode_ptr dirptr = make_shared<inode>(file_type :: DIRECTORY_TYPE);
   shared_ptr<directory> newdir = make_shared<directory>();
   dirptr->setcontents(newdir);
   newdir->getdirents().insert(pair<string,
      inode_ptr>(".", dirptr));
   return dirptr;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr fileptr = make_shared<inode>(file_type :: PLAIN_TYPE);
   shared_ptr<plain_file> newfile = make_shared<plain_file>();
   fileptr->setcontents(newfile);
   return fileptr;
}
