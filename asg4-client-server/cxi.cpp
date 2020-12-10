// $Id: cix.cpp,v 1.10 2020-07-18 23:33:51-07 - - $

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cxi_exit: public exception {};


unordered_map<string,cxi_command> command_map {
   {"exit", cxi_command::EXIT},
   {"help", cxi_command::HELP},
   {"ls"  , cxi_command::LS  },
   {"put" , cxi_command::PUT },
   {"get" , cxi_command::GET },
   {"rm" , cxi_command::RM },
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cxi_help() {
   cout << help;
}

void cxi_ls (client_socket& server) {
   cxi_header header;
   header.command = cxi_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cxi_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      size_t host_nbytes = ntohl (header.nbytes);
      auto buffer = make_unique<char[]> (host_nbytes + 1);
      recv_packet (server, buffer.get(), host_nbytes);
      outlog << "received " << host_nbytes << " bytes" << endl;
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}
void cxi_put (client_socket& server, string& filename){
   cxi_header header;
   header.command = cxi_command::PUT;
   strcpy(header.filename, filename.c_str());
   ifstream file(header.filename, ifstream::binary);
   if(file.fail()){
      outlog << "PUT: file failed to send" << endl;
   } else { 
      file.seekg(0, file.end);
      int length = file.tellg();
      file.seekg(0, file.beg);
      auto buffer = make_unique<char[]>(length);
      file.read(buffer.get(), length);
      header.nbytes = length;
      send_packet(server , &header, sizeof header);
      send_packet(server, buffer.get(), length);
      recv_packet(server, &header, sizeof header);
      cout.write(buffer.get(), length);
      file.close();
   }
}

void cxi_get (client_socket& server, string& filename){
   cxi_header header;
   header.command = cxi_command::GET;
   strcpy(header.filename, filename.c_str());
   ofstream file(header.filename, ofstream::binary);
   send_packet(server , &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   if(header.command == cxi_command::FILEOUT){ 
      outlog << "received " << header.nbytes << " bytes" << endl;
      if(header.nbytes != 0){
         auto buffer = make_unique<char[]> (header.nbytes + 1);
         recv_packet(server, buffer.get(), header.nbytes);
         file.write(buffer.get(), header.nbytes);
      }
      file.close();
   } else{
      outlog <<  "cxi_get: could not get file" << endl;
   }
}

void cxi_rm(client_socket& server, string& filename){
   cxi_header header;
   header.command = cxi_command::RM;
   strcpy(header.filename, filename.c_str());
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   if(header.command == cxi_command::NAK){
      cout <<  "cxi_rm: unable to delete the file" << endl;
   }
}


void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cxi_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cxi_server_host (args, 0);
   in_port_t port = get_cxi_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         vector<string> temp;
         size_t end = 0;
         for(;;){
            size_t start = line.find_first_not_of (" ", end);
            if(start == string::npos) break;
            end = line.find_first_of (" ", start);
            temp.push_back(line.substr (start, end - start));
         }
         string filename;
         if(temp.size() > 1){
            filename = temp[1];
         }
         if (cin.eof()) throw cxi_exit();
         outlog << "command " << line << endl;
         const auto& itor = command_map.find (temp[0]); 
         cxi_command cmd = itor == command_map.end()
                         ? cxi_command::ERROR : itor->second;
        temp.clear();
         switch (cmd) {
            case cxi_command::EXIT:
               throw cxi_exit();
               break;  
            case cxi_command::HELP:
               cxi_help();
               break;
            case cxi_command::RM:
               cxi_rm (server, filename);
               break;
            case cxi_command::LS:
               cxi_ls (server);
               break;
            case cxi_command::PUT:
               cxi_put (server,filename);
               break;
            case cxi_command::GET:
               cxi_get (server, filename);
               break;
            
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cxi_exit& error) {
      outlog << "caught cxi_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}

