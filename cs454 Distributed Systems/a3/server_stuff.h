#ifndef __SERVER_STUFF_H__
#define __SERVER_STUFF_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sstream>
#include <iostream>

#include "rpc.h"
#include "binder.h"

// for getting the total size of an argument
int getsizelen(int in) {
    union temp{
		int k;
		struct t {
			short bottom,top;
		} shorts;
		struct u {
			char a,b,type,d;
		} chars;
    };
    int test = 5000;
	int k = in;          // really roundabout way
	temp t;              // to get the bottom two 
	t.k = k;             // bytes from argTypes

	int b = (htonl(test) == test ? // depending on the endianness
	            t.shorts.top : 
	            t.shorts.bottom
	        ); 
	int len = std::max(b, 1);
	int size = 1;
	k = (htonl(test) == test ?
	        (int)t.chars.b :
	        (int)t.chars.type
	    );

	switch(k) {
      case ARG_CHAR:
        size = sizeof(char);
        break;
      case ARG_SHORT:
        size = sizeof(short);
        break;
      case ARG_INT:
        size = sizeof(int);
        break;
      case ARG_LONG:
        size = sizeof(long);
        break;
      case ARG_DOUBLE:
        size = sizeof(double);
        break;
      case ARG_FLOAT:
        size = sizeof(float);
        break;
      default:
        return -1;
	}
	return len * size;
}


// for getting the size of all arguments
int arglen(int *types) {
    int total = 0;
    for(int i = 0; types[i]; i++)
        total += getsizelen(types[i]);
    return total;
}

// for copying the arguments to an array of char
// out must be big enough to handle all of the arguments
int copytoargs(int * types, void** args, char out[]) {
    int total = 0;
    for(int i = 0; types[i] != 0; i++) {
		int lensize = getsizelen(types[i]);
        memcpy(out + total, args[i], lensize);
		total += lensize;
    }
    return 0;
}

// a handy structure for storing function skeletons
struct skel {
    char *name; // name of the function
    int *types, // array of encoding of the arguments the function takes
        flag;   // keep state making sure everything is kosher
    skeleton f;
    skel *next;
    skel(char *n, int *t, skeleton f): f(f) { // copy the skeleton and information
        flag = 0;
        types = NULL;
        name = new char[strlen(n) + 1];
        if(name) 
            strcpy(name, n); 
        else 
            flag = 1;
        int poppa = atypelen(t);
        types = new int[poppa + 1];
        if (types) {
            for(int i = 0; i <= poppa; i++) {
                types[i] = t[i];
            }
        } else {
            delete name;
            flag = 1;
        }
        next = NULL;
    };
    ~skel(){
        if (name) delete name;
        if (types) delete types;
    }
};

struct skeleton_list { // a list of skeletons
    skel *head;
    skel *tail;
    ~skeleton_list(){
    	while(head){
    		tail = head->next;
    		delete head;
    		head = tail;
    	}
    }
} list;

unsigned int ip;           // 32-bit ip address
int binder,                // fd of the binder
    listener,              // fd of listening port
    namelen = 256,         // max length of hostname
    registered = 0,        // number of registered functions
    inited = 0;            // called rpcInit yet?
char *hostname;            // the hostname of this server
unsigned short listenport; // the port that this server is listening on

// allows server/client to connect to binder
int getconnection(char * name = NULL, char * port = NULL) {
    
    int connector = socket(AF_INET, SOCK_STREAM, 0); // get a free socket
    if (connector < 0) return -1;
    char * binder_info = (name == NULL ?
                             getenv("BINDER_ADDRESS") :
                             name
                         );
    if (binder_info==NULL) return -2; // need information to start connection

    hostent *server = gethostbyname(binder_info); // connect to the binder
    if (server == NULL) return -3;
    
    sockaddr_in servaddr;
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);
    binder_info = (port == NULL ?
                      getenv("BINDER_PORT") :
                      port
                  );
    if (binder_info == NULL) return -4; // no binder info
    servaddr.sin_port = htons(atoi(binder_info));
    
    if (connect(connector,(sockaddr *) &servaddr, sizeof(servaddr)) < 0) return -5; // can;t connect
    
    return connector;
}

// returns 0 if a could evaluate b, index of where they differ otherwise
// where a and b are encoded type arrays
int typecompatible(int *a, int *b) { 
    int alen = atypelen(a);
    int blen = atypelen(b);
    
    if (alen != blen) return -1; // different number of arguments
    
    for(int i = 0; i < alen; i++) {
        int asize = a[i] & 0xFFFF;
        int bsize = b[i] & 0xFFFF;
        int atype = a[i] & 0xFFFF0000;
        int btype = b[i] & 0xFFFF0000;
        if (atype == btype) // same types
            if (asize == bsize || asize > 0 && asize > bsize) {
                // array of compatible length
                continue;
            }
        return i;
    }    
    return 0;
}

// returns a fd that can be listened on for new connections
int getacceptor() {
    addrinfo hints, *addlist;
    bzero((char *)&hints, sizeof(hints));
    hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
    int addrerr = getaddrinfo(NULL, "0", &hints, &addlist);
    if (addrerr) return -6; // some error occurred
        
    int out = 0;
    
    // try all addrinfos
    for(addrinfo *temp = addlist; temp != NULL; temp = temp->ai_next) {
        out = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol); // try to open a socket
        if (out < 0) { 
            out = 0;
                continue;
        }
        int reuse = 1;
        setsockopt(out, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        sockaddr_in tsock;
        socklen_t tsocklen = sizeof(tsock);
        if (bind(out, temp->ai_addr, temp->ai_addrlen) < 0 || // try to bind the socket 
            getsockname(out, (sockaddr *)&tsock, &tsocklen)) {
            close(out);
            out = 0;
            continue;
        }
        hostname = new char[namelen];
        gethostname(hostname,namelen); // get own hostname
        namelen = strlen(hostname);
        listenport = ntohs(tsock.sin_port);
        ip = ntohl(
            ((sockaddr_in*)(temp->ai_addr))->sin_addr.s_addr
        );
        break;
    }
    if (!out) return -7;
    if (listen(out, 5) == -1) return -8;
    
    freeaddrinfo(addlist);
    return out;
}

/**
 * Returns the file descriptor to a server available to handle
 * the give function and arguments
 **/
int getFd(char*name, int* argTypes, void** args) {
    
    int nbytes;
    unsigned long fd = 0;
    binder = getconnection();

    // send location request to binder
    long msgid = htonl(LOC_REQUEST);
    nbytes = send(binder, &msgid, 4, 0);

    // name length
    msgid = htonl(strlen(name)+1);
    send(binder,&msgid,4,0);

    // name
    send(binder,name,ntohl(msgid),0);

    // argtypes length
    msgid = htonl(atypelen(argTypes)+1);
    send(binder,&msgid,4,0);

    // argtypes
    send(binder,argTypes,ntohl(msgid)*sizeof(int),0);

    // recieve server info back from binder
    int res=0;
    nbytes = recv(binder,&res,4,0);
    if(ntohl(res)!=LOC_SUCCESS) {
        nbytes = recv(binder,&res,4,0);
        close(binder);
        return res;
    }
    // return name & port
    nbytes = recv(binder,&msgid,4,0);
    msgid = ntohl(msgid);
    char servername[msgid];
    nbytes = recv(binder,servername,msgid,0);
    nbytes = recv(binder,&msgid,4,0); 
    std::string portstring = intToString(ntohl(msgid)); // <-port
                                                   // in case you want to 
                                                   // use the port as a string 

    //this is where you get it to get the connection to the server(servername) on port(portstring)
    int out = 0; // <-make this the fd to the server
    char *ports = new char[portstring.size()+1];
    for(int i=0;i<=portstring.size();i++)
       ports[i] = portstring[i];
    
    out = getconnection(servername,ports);
    delete [] ports;
    close(binder);
    if (out<=0) return -1;
    return out;
}

#endif // __SERVER_STUFF_H__
