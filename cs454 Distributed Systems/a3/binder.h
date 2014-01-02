#ifndef __BINDER_H__
#define __BINDER_H__
/*
 * binder.h
 *
 * This file contains the header info for binder as well as some helpful classes for it
 * and other functions that get used throughout the project
 *
 */
#include <sstream>
#include <map>
#include <set>
#include "rpc.h"

#define REGISTER            1001
#define LOC_REQUEST         1002
#define LOC_SUCCESS         1003
#define LOC_FAILURE         1004
#define EXECUTE             1005
#define EXECUTE_SUCCESS     1006
#define EXECUTE_FAILURE     1007
#define TERMINATE           1008
#define REGISTER_SUCCESS    1009
#define REGISTER_FAILURE   -1010
#define REGISTER_PREVIOUS   1011 // what
#define REGISTER_OVERLOAD   1012 // the difference
#define REGISTER_DUPLICATE  1013 // between these?
#define ARRAY_TOO_LARGE    -1014
#define BAD_ARGTYPE        -1015
#define NO_SUCH_FN         -1016

//#define DEBUG // use this for debugging output

//#define DEBUGTYPECOMP
//#define DEBUGSKEL
//#define NOBINDER


// get the length of an int array
int atypelen(int * in){
    int i=0;
    //for (i=0;in[i]!=0;i++){}
    while(in[i]!=0) i++;
    //if (i==0) 
    return i;
    return i-1;
}


// cute to string function 'cause c/c++ sucks a whole bunch
std::string intToString(int i) {
   std::stringstream ss;
   ss << i;
   return ss.str();
}

// the grossest thing in the world
std::string typeString(int in) {
   union temp{
      int k;
      struct t {
         short bottom,top;
      } shorts;
      struct u {
         char a,b,type,d;
      } chars;
   };

   int k = in;          // really roundabout way
   temp t;              // to get the bottom two 
   t.k = k;             // bytes from argTypes

   // make it safe or whatever
   int test = 5000;
   int array = (htonl(test) == test ? t.shorts.top : t.shorts.bottom);
   int type = (htonl(test) == test ? (int)t.chars.b : (int)t.chars.type);
   int inOut = (htonl(test) == test ? (int)t.chars.a : (int)t.chars.d); // why isn't this 30/31?

   std::string isArray = "0"; // not array
   if(array != 0) {
      isArray = "1"; // is array
   }

   std::string isInOut = "0"; // input
   if(inOut == 64) {
      isInOut = "1"; // output
   }
   if(inOut == -64) {
      isInOut = "2"; // input/output
   }

   // eg. array of longs as output: "411"
   return intToString(type) + isArray + isInOut;
}

// Function class, this defines a function that a server might provide
class Function {

   public:

   std::string name;
   int *args;

   Function(std::string n, int *a) {
      this->name = n;
      this->args = a;
   }

   // function signature: "name\0411"
   std::string getSig() {
      std::string sig = this->name + "\0";
      int size = atypelen(this->args);

      for (int i = 0; i<size; i++) {
         int arg = this->args[i];

         sig += typeString(arg);
      }
      return sig;
   }

   // for testing
   void print() {
      std::cout << "[Function] name: " << name << " args: ";
      int size = atypelen(args);
      for(long k = 0; k < size; k++) {
         std::cout << std::hex << args[k] <<  " ";
      }
      std::cout << std::dec << std::endl;
      
   }

};

// Server class, this defines a server and maintains a list of functions and
// function names that the server has available
class Server {

   public:

   int fd;
   unsigned short port;
   std::string ip;

   // we keep track of the names so we know if we're overloading a previously declared
   // function, and then the set of functions is so we can see if the server has the
   // correct function
   std::set<std::string> functions;
   std::set<std::string> names;

   Server(int fd, unsigned short port, std::string ip) {
      this->fd = fd;
      this->port = port;
      this->ip = ip;
   }

   // server signature: "ip\0port"
   std::string getSig() {
      return ip + "\0" + intToString(port);
   }

   // takes a function and adds the name to the server's name list and
   // the function to the server's function list
   // returns the status of the registration as this is where it "all happens"
   int addFunction(Function* function) {
      int returnStatus = REGISTER_FAILURE;
      std::pair<std::set<std::string>::iterator, bool> ret;

      // add the name
      ret = names.insert(function->name);

      if(!ret.second) {
         returnStatus = REGISTER_OVERLOAD;
      }

      // add the function
      ret = functions.insert(function->getSig());
      
      if (!ret.second) {
         returnStatus = REGISTER_PREVIOUS;
      } else if(ret.second) {
         returnStatus = REGISTER_SUCCESS;
      }


      // return the status
      return returnStatus;
   }

   // for testing
   void print() {
      std::cout << "[Server] fd: " << fd << " host: "
      << ip << " port: " << port << std::endl;
   }

};

// binder stuff
void handleRegisterRequest(int fd);
void handleLocationRequest(int fd);
Server* getServer(std::string functionSig);
void printDb();
#endif
