/*
 * binder.c
 *
 * This file contains the binder functionality (and the server class methods as well)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "binder.h"

using namespace std;

std::map<string, Server*> db; // string is the serverSig
std::vector<string> serverOrder;  // the servers that have been registered
int nextServer; // keep track of where we are in the serverOrder

// for debugging
void printDb() {
   for (std::map<string,Server*>::iterator it = db.begin(); it != db.end(); ++it) {
      cout << "it: " << it->first << endl;
   }
}

// Handles a registration request:
//    - Processes info from the server
//    - Adds the server to the server list (if not there already)
//    - Registers the function and server in the database 
void handleRegisterRequest(int fd, fd_set &servers) {

   long type;
   int nbytes;
   int cumu=0;
   
   // len of hostname
   nbytes = recv(fd, &type, 4, 0);
   cumu+=nbytes;
   type = ntohl(type);
   cout << "Len of hostname: " << type << " | recd:" << nbytes << " total:" << cumu << endl;

   // hostname
   char *hostname;
   hostname = new char[type];
   nbytes = recv(fd, hostname, type, 0);
   cumu+=nbytes;
   cout << "Hostname: " << hostname << " | recd:" << nbytes << " total:" << cumu << endl;

   // port
   unsigned long port;
   nbytes = recv(fd, &port, 4, 0);
   cumu+=nbytes;
   port = ntohl(port);
   cout << "Port: " << port << " | recd:" << nbytes << " total:" << cumu << endl;

   // len of name
   nbytes = recv(fd, &type, 4, 0);
   cumu+=nbytes;
   type = ntohl(type);
   cout << "Len of name: " << type << " | recd:" << nbytes << " total:" << cumu << endl;

   // name
   char *name;
   name = new char[type];

   nbytes = recv(fd, name, type, 0);
   cumu+=nbytes;
   cout << "Name: " << name << " | recd:" << nbytes << " total:" << cumu << endl;

   // len of argtypes
   nbytes = recv(fd, &type, 4, 0);
   cumu+=nbytes;
   type = ntohl(type);
   cout << "Len of args: " << type << " | recd:" << nbytes << " total:" << cumu << endl;

   // argtypes
   int *args;
   args = new int[type*4+1];
   nbytes = recv(fd, args, type*4, 0);
   cumu+=nbytes;
   
   cout << "Argtypes: ";
   for(long k = 0; k < type; k++) {
      cout << hex << args[k] <<  " ";
   }
   cout << dec << endl << " | recd:" << nbytes << " total:" << cumu << endl; 
 
   int registerstatus;

   // make the server and function
   Server* server = new Server(fd, port, hostname);
   Function* func = new Function(string(name), args);

   // see if the server is in our db
   std::map<string, Server*>::iterator it;
   std::pair<std::map<string, Server*>::iterator, bool> ret;  

   it = db.find(server->getSig());

   if (it != db.end()) { // we already have the server in the db, so lets get it & update it
      server = it->second;
      registerstatus = server->addFunction(func);
      db.erase(it);
      ret = db.insert( std::pair<string, Server*>(server->getSig(), server));
      if (!ret.second) {
         // shouldn't get here ever
         cout << "UPDATED PREVIOUS???" << endl;
      }

   } else { // we don't have the server so add it, and add it to serverOrder
      
      registerstatus = server->addFunction(func);
      ret = db.insert( std::pair<string, Server*>(server->getSig(), server));
      
      if (!ret.second) {
         // shouldn't get here ever
         cout << "REGISTER PREVIOUS???" << endl;
      }

      serverOrder.push_back(server->getSig());
      
   }

   // testing
   string functionSig = func->getSig();
   string serverSig = server->getSig();
   cout << "function sig: " << functionSig << " server sig: " << serverSig;
   cout << " register status: " << registerstatus << endl;
   cout << "size of db: " << db.size() << endl;
   
   //send response
   send(fd, &registerstatus, 4, 0);
   cumu+=nbytes;

   // TODO: do we care about the int warning if we use registerstatus instead?
   int res = htonl(0); // warnings or other things
   send(fd, &res, 4, 0);
   cumu+=nbytes;

   FD_SET(fd,&servers);

   cout << "register status: " << registerstatus << "\n" << endl;

   delete args;
   delete hostname;
   delete name;
}

Server* findAServer(string functionSig) {

   int end = false;
   int start = nextServer;

   // go through the serverOrder seeing if each server has the functionSig
   int size = serverOrder.size() + nextServer;
   for (int i = nextServer; i < serverOrder.size(); i++) {

      if (end && i == start) {
         return NULL;
      }

      std::map<string, Server*>::iterator it;

      it = db.find(serverOrder.at(i));

      if (it != db.end()) { // found the server, check if it has the function
         
         Server* server = it->second;
         
         std::set<string>::iterator it2;
         it2 = server->functions.find(functionSig);

         if(it2 != server->functions.end()) { // we found the function return the server!
            nextServer = i + 1;
            if (nextServer >= serverOrder.size()) nextServer = 0;
            return server;
         }
      } 
      
      if (i + 1 == serverOrder.size() && end == false) { // we reached the end, go again to the saved val
         end = true;
         i = 0;
      } else {
         return NULL;
      }
   }
}

void handleLocationRequest(int fd) {
   int nbytes;
   long type;

   // len of name
   nbytes = recv(fd, &type, 4, 0);
   type = ntohl(type);
   cout << "Len of name: " << type << endl;

   // name
   char *name;
   name = new char[type];
   nbytes = recv(fd, name, type, 0);
   cout << "Name: " << name << endl;

   // len of argtypes
   nbytes = recv(fd, &type, 4, 0);
   type = ntohl(type);
   cout << "Len of args: " << type << endl;

   // argtypes
   int *args;
   args = new int[type*4];
   nbytes = recv(fd, args, type*4, 0);
   
   cout << "Argtypes: ";
   for(long k = 0; k < type; k++) {
      cout << hex << args[k] <<  " ";
   }
   cout << dec << endl;
   
   // make the function and look it up
   std::map<string, Server*>::iterator it;
   Function* func = new Function(string(name), args);
   string sig = func->getSig();

   Server* server = findAServer(sig);

   if (server != NULL) { // there is a server!
      cout << "there is a server!" << endl;
      // send it to the client
      int retstatus = htonl(LOC_SUCCESS);
      send(fd, &retstatus, 4, 0);

      // send namelen
      retstatus = htonl(server->ip.size()+1);
      send(fd, &retstatus, 4, 0);
      retstatus = ntohl(retstatus);

      // send name
      char *servername = new char[retstatus];
      for(int i = 0; i <= retstatus; i++)
          servername[i] = server->ip[i];
      send(fd, servername, retstatus, 0);

      // send port
      retstatus = htonl(server->port);
      send(fd, &retstatus, 4, 0);
      
      delete servername;

   } else { // no server, let the client know
      cout << "no server found for function sig: " << sig << endl;
      // send it to the client
      int retstatus = htonl(LOC_FAILURE);
      send(fd, &retstatus, 4, 0);
   }
   
   delete name;
   delete args;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
        if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/**
 *
 * So basically we run the binder first and then server can register stuff with it
 * and the client can request shit from it.
 *
 * (Note: This main function is almost entirely from Beej's tutorial page...)
 *
 **/
int main(int argc, char *argv[]) {

   char * bin_addr = getenv("BINDER_ADDRESS");
   char * bin_port = getenv("BINDER_PORT");

   // stuff for handling for client data
   char buf[256];    
   int nbytes;
   char remoteIP[INET6_ADDRSTRLEN];
   

   fd_set master;
   fd_set read_fds;
   fd_set servers;
   int fdmax;

   int listener;
   int newfd;
   struct sockaddr_storage remoteaddr;
   socklen_t addrlen;

   struct addrinfo hints, *ai, *p;
   int i, j, rv;

   FD_ZERO(&master);
   FD_ZERO(&read_fds);
   FD_ZERO(&servers);

   // find a socket and bind it
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;

   if ((rv = getaddrinfo(NULL, 
#ifdef TESTING
   "5000"
#else
   "0"/*bin_port*/
#endif
   , &hints, &ai)) != 0) {
      fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
      exit(1);
   }

   for(p = ai; p != NULL; p = p->ai_next) {
      listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
         if (listener < 0) { 
            continue;
         }
     int reuse = 1;
     setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
     sockaddr_in tsock;
     socklen_t tsocklen = sizeof(tsock);
         if (bind(listener, p->ai_addr, p->ai_addrlen) < 0
                  || getsockname(listener,(sockaddr *)&tsock,&tsocklen)) {
            close(listener);
            continue;
         }
     int namelen=256;
     char name[namelen];
     gethostname(name,namelen);
     cout << "BINDER_ADDRESS " << name << "\nBINDER_PORT " << ntohs(tsock.sin_port) << endl;
	 break;
   }

   // if we get here we didn't bind :(
   if (p == NULL) {
      fprintf(stderr, "selectserver: failed to bind\n");
      exit(2);
   }

   freeaddrinfo(ai); // don't need this

   // listen to things
   if (listen(listener, 10) == -1) {
      perror("listen");
      exit(3);
   }

   // add listener to master fd set
   FD_SET(listener, &master);

   // keep track of the biggest fd
   fdmax = listener;

   // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we have stuff to do
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {

                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        
                        printf("binder: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                               get_in_addr((struct sockaddr*)&remoteaddr),
                               remoteIP, INET6_ADDRSTRLEN), newfd);

                    }
                } else {
                    // receive data from a client
                    long type;
                    if ((nbytes = recv(i, &type, 4, 0)) <= 0) {                        
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i); 
                            // check if server was the one who left
                            int wasserver = FD_ISSET(i,&servers);
                            if (wasserver) { // shit, gotta remove all of it's entries from the database
                               // TODO remove server from list 
                               printf("was a server\n");
                               FD_CLR(i,&servers);
                               FD_CLR(i,&master);
                            }
                        } else {
                            perror("recv");
                        }
                        close(i); 
                        FD_CLR(i, &master); // remove from master set

                    } else {
                        type = ntohl(type); // network to host long
                        cout << "Type: " ;//<< type << endl;
                        switch(type) {

                           case REGISTER:
                              cout << "REGISTER" << endl;
                              handleRegisterRequest(i,servers);
                              break;
                           case LOC_REQUEST:
                              cout << "LOC_REQUEST" << endl;
                              handleLocationRequest(i);
                              break;
                           case TERMINATE:
                              cout << "TERMINATE" << endl;
                              long msgid = htonl(TERMINATE);
                              for(int j = 0; j <= fdmax+1; j++) {
                                 if(FD_ISSET(j, &servers)) {
                                    nbytes = send(j, &msgid, 4, 0);
                                    cout << "sending "<<ntohl(msgid)<< " bytes:"<<nbytes <<" to fd: " << j << endl;
                                 }
                              }
                              //break; // this only gets you back to the outer infinite loop, need "multi-leveled exit" or
                                       // since this is where to prog ends, an exit(0) will work
                              exit(0);
                        }
                   }
                } 
             }        
          }     
       }     
    return 0;
}
