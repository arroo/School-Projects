#include "server_stuff.h"
int rpcCall(char* name, int* argTypes, void** args) {
    
    int nbytes;                               // current number of bytes sent in each message
    int msg = htonl(EXECUTE);                 // meta information about what's happening
    int server = getFd(name, argTypes, args); // use the binder to find a server
                                              // with the same funciton signature
                                              // (name, argtypes) as what is being called

    if (server <= 0) // something screwed up
        return -10;

    // the format of communicating with the server is
    // EXECUTE
    // length of the name of the function
    // name of the function
    // number of arguments (in case same name but different arguments)
    // types of the arguments
    // the length of each argument
    // each argument
    // 
    // then the server responds with a returncode
    // and if successful the proper arguments coming back
    
    nbytes = send(server,&msg, 4, 0); // send EXECUTE

    msg = htonl(strlen(name) + 1);
    nbytes = send(server, &msg, 4, 0); // send the length of the name of the function

    msg=ntohl(msg);
    nbytes = send(server, name, msg, 0); // send the name of the function

    msg = htonl(atypelen(argTypes) + 1);
    nbytes = send(server, &msg, 4, 0); // send the length of the argument types

    msg=ntohl(msg);
    for(int i = 0; argTypes[i] != 0; i++)
        argTypes[i] = htonl(argTypes[i]);
    nbytes = send(server, argTypes, msg * sizeof(int), 0); // send the argument types

    for(int i = 0; argTypes[i] != 0; i++)
        argTypes[i] = ntohl(argTypes[i]);

    if (msg) { // if there are arguments to send - could be zero
        // send all the arguments
        msg = htonl(arglen(argTypes));
        nbytes = send(server,&msg,4,0);
        msg=ntohl(msg);

        char *argsout; // all of the arguments to be sent to the server
        argsout = new char[msg];
        copytoargs(argTypes, args, argsout);
        int total = 0;
        for(int sent = 0; total < msg; total += sent) {
            sent = send(
                       server,
                       argsout + total,
                       msg - total,
                       0
                   );
        }
        delete [] argsout;
    }

    // now we get results
    
    int returncode;
    recv(server,&returncode,sizeof(returncode),0);
    returncode = ntohl(returncode);

    if (returncode != EXECUTE_SUCCESS) { // something went wrong in execution
        recv(server,&returncode,sizeof(returncode),0);
        returncode = ntohl(returncode);
        close(server); 

        return returncode;
    }
    //recv namelen, name, argtypelen, argtypes, arglen, args
    nbytes = recv(server,&msg,4,0); // length of the name of the function

    char backname[ntohl(msg)];
    nbytes = recv(server,backname,ntohl(msg),0); // name of the function
                                                 // don't need to do anything
                                                 // with it but that's the protocol

    nbytes = recv(server, &msg, 4, 0); // msg = the length of the argument types
    msg = ntohl(msg);
    int backtypes[msg + 1];
    int backarglen = msg;
    nbytes = recv(server, backtypes, msg * sizeof(int), 0); // get the argument types
    for(int i = 0; i < msg; i++)
        backtypes[i] = ntohl(backtypes[i]);

    nbytes = recv(server,&msg,4,0); // msg = length of the arguments
    msg = ntohl(msg);
    char *backargs;
    backargs = new char[msg];
    int backtotal;
    for(backtotal = 0; backtotal < msg; backtotal += nbytes) {
        nbytes = recv(server, backargs, msg, 0); // receive the arguments
    }
    
    //do some sort of copying?
    backtotal = 0;
    for(int i = 0; i < backarglen; i++) {
        args[i] = backtotal + backargs;
        backtotal += getsizelen(backargs[i]);
    }
    

    close(server);
    return 0;
}
