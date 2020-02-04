// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "commands.h"
#include "debug.h"

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
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
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
   map<string, inode_ptr> dirents = 
      state.getCWD()->getContents()->getDirents();
   map<string, inode_ptr>::iterator dd;
   for(dd = dirents.begin(); dd != dirents.end(); dd++){
      if(words[1] == dd->first) {
         dd->second->getContents()->readfile();
         return;
      }
   }

   throw command_error("cat: "+words[1]+": No such file or directory"); 
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() > 2)
      throw command_error ("cd: too many operands");
   else if(words.size() == 1)
      state.getCWD() = state.getRoot();
   else{  
      map<string, inode_ptr>::iterator cd;
      map<string, inode_ptr> dirents = 
      state.getCWD()->getContents()->getDirents();
      bool foundDirectory = false;
      for(cd = dirents.begin(); cd != dirents.end(); cd++){
         if(cd->first == words[1]){ 
            if(cd->second->getContents()->getType() == "directory"){
               state.getCWD() = cd->second;
               foundDirectory = true;
            }
            else
               throw command_error(
                  "cd: '" + words[1] + "' is a plain file");
        }
      }
     if(foundDirectory == false)
        throw command_error("cd: '" + words[1] + "' does not exist");
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
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   inode_ptr temp = state.getCWD();
   if(words.size() == 1){
      cout << state.getCWD()->getPath() << ":" << endl;  
      state.getCWD()->getContents()->printDirents();
   }
   else if(words[1] == "/"){
      state.getCWD() = state.getRoot();
      cout << "/:" << endl;
      state.getCWD()->getContents()->printDirents();
      state.getCWD() = temp;
   }
   else if(words[1] == ".") {
      cout << ".:" << endl;
      state.getCWD()->getContents()->printDirents();
   }
   else{
      map<string, inode_ptr>::iterator d2p;
      map<string, inode_ptr> dirents = 
         state.getCWD()->getContents()->getDirents();
      for(d2p = dirents.begin(); d2p != dirents.end(); d2p++){
         if(d2p->second->getPath() == words[1] or 
            d2p->first == words[1]) {
            if(words[1] != ".."){
               cout << "/" << words[1] << ":" << endl;
               d2p->second->getContents()->printDirents();
               break;
            }
            else{
               cout << words[1] << ":" << endl;
               d2p->second->getContents()->printDirents();
               state.getCWD() = temp;
               break;
            }
         }
      }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   inode_ptr temp = state.getCWD();
   map<string, inode_ptr>::iterator d2p;
   map<string, inode_ptr> dirents = 
      state.getCWD()->getContents()->getDirents();
   for(d2p = dirents.begin(); d2p != dirents.end(); d2p++){
      if(d2p->second->getContents()->getType() == "directory" 
         and d2p->first != "." and d2p->first != "..") {
         cout << d2p->second->getPath() << ":" << endl;
         d2p->second->getContents()->printDirents();
         state.getCWD() = d2p->second;
         fn_lsr(state, words);
         break;
      }
   }
   state.getCWD() = temp;  
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
}

void fn_make (inode_state& state, const wordvec& words){
   /*if(words.size() < 2) {
      throw command_error("make: invalid argument; missing words");
      return;
   }*/
   // to do: if there are no words, the file is empty
   inode_ptr new_file = state.getCWD()->getContents()->mkfile(
           words[1], state);
   new_file->getContents()->writefile(words);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   map<string, inode_ptr> dirents = 
      state.getCWD()->getContents()->getDirents();
   map<string, inode_ptr>::iterator dd;
   for(dd = dirents.begin(); dd != dirents.end(); dd++){
      if(words[1] == dd->first)
         throw command_error ("mkdir: cannot create directory '"
            + words[1] + "': File exists");
   }
   state.getCWD()->getContents()->mkdir(words[1], state);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.prompt() = (words[1] + " ");
}

void fn_pwd (inode_state& state, const wordvec& words){
   inode_ptr pwd = state.getCWD();
   
   cout << pwd->getPath() << endl;
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   map<string, inode_ptr> dirents = 
      state.getCWD()->getContents()->getDirents();
   map<string, inode_ptr>::iterator dd;
   for(dd = dirents.begin(); dd != dirents.end(); dd++){
      if(words[1] == dd->first) {
          if(dd->second->getContents()->getType() == "directory" 
             && dd->second->getContents()->size() > 2) {
             throw command_error ("rm: cannot remove has stuff in it");
          } else {
             state.getCWD()->getContents()->remove(words[1]);
             return;
          }
      }
   }

   throw command_error(
      "rm: "+words[1]+": No such file or directory");
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

