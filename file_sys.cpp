// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $
// Olivia Wong omwong, Ryan Kim rysukim

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);

   root->getPath() = "/";
   root->getContents()->getDirents().insert
      (pair<string, inode_ptr>(".", root));
   root->getContents()->getDirents().insert
      (pair<string, inode_ptr>("..", root));
  
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\""); 
}

string& inode_state::prompt() { return prompt_; }

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

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
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

inode_ptr base_file::mkdir (const string&, inode_state& state) {
   DEBUGF ('z', state);
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&, inode_state& state) {
   DEBUGF ('z', state);
   throw file_error ("is a " + error_file_type());
}

size_t plain_file::size() const {
   size_t size {0};
   int s = 0;
   int i = 0;
   for (string word: data) {
      i++;
      if (i < 3) { continue; }
      s += (word.length());
   }
   s += (data.size() - 3); 
   DEBUGF ('i', "size = " << size);
   return s;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data)
   
   int i = 0;
   for (string word: data) {
      i++;
      if (i < 3) { continue; }
      cout << word << " ";
   }
   cout << endl;
   return data; 
}

void plain_file::writefile (const wordvec& words) {
   this->data = words;
   DEBUGF ('i', words);
}

size_t directory::size() const {
   int size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) {
  DEBUGF ('i', "removing: " << filename << endl);
  this->getDirents().erase(filename);
}

inode_ptr directory::mkdir (const string& dirname, inode_state& state) {
   inode_ptr node = make_shared<inode>(file_type::DIRECTORY_TYPE);
   if(state.getCWD()->getPath() == "/")
      node->getPath() = state.getCWD()->getPath() + dirname;
   else
      node->getPath() = state.getCWD()->getPath() + "/" + dirname;
   node->getContents()->getDirents().insert(
           pair<string, inode_ptr>(".", node));
   node->getContents()->getDirents().insert(
           pair<string, inode_ptr>("..",  state.getCWD()));
   this->getDirents().insert(
      pair<string, inode_ptr>(dirname, node));
   return node;
}

inode_ptr directory::mkfile(const string& filename,inode_state& state){
   inode_ptr node = make_shared<inode>(file_type::PLAIN_TYPE);
   node->getPath() = state.getCWD()->getPath() + filename;

   this->getDirents().insert(
           pair<string, inode_ptr>(filename, node));
   DEBUGF ('i', filename);
   return node;

}

map<string, inode_ptr>& directory::getDirents() { return dirents; }

map<string, inode_ptr>& base_file::getDirents() {
   throw file_error ("is a " + error_file_type());
}
void base_file::printDirents() { 
   throw file_error ("is a " + error_file_type());
}

void directory::printDirents() {
   map<string, inode_ptr>::iterator d2p;
   for(d2p = dirents.begin(); d2p != dirents.end(); d2p++){
         if(d2p->second->getContents()->getType() == "directory"
            and d2p->first != "." and d2p->first !="..")
            cout << "     " << d2p->second->get_inode_nr() << "       "
               << d2p->second->getContents()->size() << "  "
               << d2p->first << "/" << endl;
         else
            cout << "     " << d2p->second->get_inode_nr() << "       "
               << d2p->second->getContents()->size() << "  "
               << d2p->first << endl;
   }
}
