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
#include "server_stuff.h"

// server calls this to grab a connection with the binder
int rpcInit() {
    listener = getacceptor();
    if (listener < 0) return listener;
    binder = getconnection();
    if(binder < 0) {
        close(listener); 
        return binder;
    }
    list.head=NULL;
    list.tail=NULL;
    inited = 1;
    return 0;
}

int rpcRegister(char* name, int* argTypes, skeleton f) {
    if (inited != 1) return -9; // need to call rpcInit first!
    // create first
    skel *new_skel = new skel(name,argTypes,f);
    if(new_skel==NULL) return -10;
    if(new_skel->flag) {
        delete new_skel;
        return -11;
    }
    // tell the binder all about this function
    unsigned long msgid = htonl(4);

    // message id
    msgid = htonl(REGISTER);
    send(binder,&msgid,4,0);
    
    // name length
    msgid = htonl(namelen+1);
    send(binder,&msgid,4,0);

    // hostname
    send(binder,hostname,namelen+1,0);

    // port
    msgid = htonl(listenport);
    send(binder,&msgid,4,0);

    // name length
    msgid = htonl(strlen(name)+1);
    send(binder,&msgid,4,0);

    // name
    send(binder,name,strlen(name)+1,0);

    // length of args
    msgid = htonl(atypelen(argTypes)+1);
    send(binder,&msgid,4,0);
    send(binder,argTypes,ntohl(msgid)*sizeof(int),0);

    // response
    char response[17];
    int res=0;
    bzero(&response,17);
    recv(binder,&res,4,0); // should care about trying to register again
    recv(binder,&res,4,0);
    res = ntohl(res);
    
    // add to list
    if (res >= 0)
        if (list.head == NULL) list.head = list.tail = new_skel;
        else {
            list.tail->next = new_skel;
            list.tail = new_skel;
        }
    else {
        delete new_skel;
        return -1;
    }
    registered += 1;
    return 0;
}
// this is the function that a server will call once all of its functions have been registered with the binder
// this function returns once the binder has received a TERMINATE command
// and it can work on multiple calls at the same time.
int rpcExecute(){
    if(inited != 1) return -12; // need to call rpcInit first
    if(registered < 1) return -13; // no use running a server with no functions registered

    struct child { // used for waiting in child threads
        pid_t pid;
        child * next;
    } *kids = NULL;
    int highestclient = std::max(binder,listener);
    int acceptor = listener; // easier than rewriting code from example server
    fd_set all, accepting;
    FD_ZERO(&all);
    FD_ZERO(&accepting);
    FD_SET(listener,&all);
    FD_SET(binder,&all); 
    
    int quit = 0; // once rpcTerminate is called by a client, shut down
    
    
    while (quit == 0) {
        accepting = all;
        if (select(highestclient + 1, &accepting, NULL, NULL, NULL) == -1) {
            // something went wrong while selecting a client
            return -1;
        }
        for(int i = 0; i <= highestclient; i++) if (FD_ISSET(i, &accepting)) // listen to sockets
            if (i == acceptor) { // listening port
                sockaddr_storage clientaddr;
                socklen_t clientlen = sizeof(clientaddr);
                int newclient = accept(acceptor,(sockaddr *)&clientaddr, &clientlen);
                if (newclient >= 0) {
                    highestclient = std::max(highestclient, newclient);
                    FD_SET(newclient, &all);
                }
            } else if (i == binder){ 
                int msg = 0;
                long recd;
                recd = recv(i, &msg, 4, 0);
                if (ntohl(msg) == TERMINATE) {
                    quit = 1;
                }
                if (recd == 0) {
                    std::cout << "the binder disconnected (shut down) but no informati"
                              << "on was specified in the assignment of what to do abo"
                              << "ut it so i'll just quit" << std::endl;
                    quit = 1;
                }
            } else { // client wants to perform a function 
                pid_t kid = fork();
                if (kid) { // let the child thread handle the request
                           // while parent listens for more connections
                    close(i);
                    FD_CLR(i,&all);
                    child *newkid = new child;
                    newkid->pid = kid;
                    newkid->next = kids;
                    kids = newkid;
                    continue;
                }
                
                close(binder);   // child doesn't
                close(acceptor); // need these
                int msg = 0;
                if(recv(i, &msg, sizeof(msg), 0) == 0) { // don't receive anything?
                    // then quit
                    close(i);
                    exit(0);
                }
                
                if (msg != htonl(EXECUTE)) { // there is no other reason for
                                             // a client to talk to a server
                                             // directly
                    msg = htonl(EXECUTE_FAILURE);
                    send(i, &msg, sizeof(msg), 0);
                    msg = htonl(-1101);
                    send(i, &msg, sizeof(msg), 0);
                    close(i);
                    exit(0);
                }
                
                int length = 0;
                recv(i, &length, 4, 0); // length of funciton name
                
                length = ntohl(length);
                char *fname;
                fname = new char[length];
                recv(i,fname,length,0); // function name
                
                recv(i,&length,4,0); // length of argument types
                
                length = ntohl(length);
                int *atypes,
                    arrg = length;
                char ** argsarr;
                atypes = new int[length];
                argsarr = new char*[length];
                
                recv(i,atypes,length*sizeof(int),0); // argument types array
                for(int j=0;j<length;j++) {
                    atypes[j] = ntohl(atypes[j]);
                }
                
                recv(i,&length,4,0); // length of arguments
                length = ntohl(length);
                char *args;
                args = new char[length];
                int totalarg = 0,
                    nbytes;
                for(; totalarg < length; totalarg += nbytes)
                    nbytes = recv(i, args, length, 0); // arguments
                totalarg = 0;
                
                for(int j = 0; j < arrg; j++) { // put arguments into array
                    argsarr[j] = totalarg + args;
                    totalarg += getsizelen(atypes[j]);
                }
                
                int argcomp = 0;
                skel *t;
                for(t = list.head; t != NULL; t = t->next) { // see which function
                                                             // the client wants
                    if(strcmp(t->name, fname) == 0) { // check name
                        argcomp = typecompatible(t->types,atypes); // check argument type
                        if(argcomp == 0)                           // compatibility
                            break;
                    }
                }
                if (t) {
                    int result = t->f(atypes,(void**)argsarr); // perform the function
                    
                    if (result == 0) { // good stuff!
                        int nbytes;
                        msg = htonl(EXECUTE_SUCCESS);
                        nbytes = send(i, &msg, 4, 0); // send success message

                        msg = htonl(strlen(fname) + 1);
                        nbytes = send(i, &msg, 4, 0); // send function name

                        nbytes = send(i, fname, ntohl(msg), 0); // send argument type length

                        msg = htonl(atypelen(atypes) + 1);
                        nbytes = send(i, &msg, 4, 0); // send argument types length

                        for (int j = 0; j < ntohl(msg); j++)
                            atypes[j] = htonl(atypes[j]);
                            
                        // send argument types
                        nbytes = send(i, atypes, ntohl(msg) * sizeof(int), 0);

                        totalarg = 0;
                        int k;
                        for(k = 0; k < ntohl(msg); k++) {
                            totalarg += getsizelen(ntohl(atypes[k]));
                        }

                        totalarg = htonl(totalarg);
                        nbytes = send(i, &totalarg, 4, 0); // send argument length

                        totalarg = ntohl(totalarg);
                        char backargs[totalarg];
                        for(int j = 0; atypes[j] != 0; j++)
                            atypes[j] = ntohl(atypes[j]);

                        copytoargs(atypes, (void**)argsarr, backargs);
                        nbytes = send(i, &backargs, totalarg, 0); // send arguments

                    } else { // function returned an error
                        int backresult = htonl(EXECUTE_FAILURE);
                        send(i, &backresult, 4, 0);
                        backresult = htonl(result);
                        send(i, &backresult, 4, 0);
                    }
                    //send message
                } else { // can't execute function

                    int reason = htonl(EXECUTE_FAILURE);
                    send(i, &reason, 4, 0);
                    if(argcomp < 0)
                        reason = htonl(ARRAY_TOO_LARGE);
                    else if (argcomp > 0)
                        reason = htonl(BAD_ARGTYPE);
                    else
                        reason = htonl(NO_SUCH_FN);
                    send(i,&reason,4,0);
                }
                
                close(i);
                delete args;
                delete fname;
                delete atypes;
                exit(0);
            } 
    }
    
    for(int i = 0; i < highestclient + 1; i++) if (FD_ISSET(i, &all)) {
        close(i); // stop listening to the clients
        FD_CLR(i,&all);
    }
    
    // clear the fd sets
    FD_CLR(binder,&all);
    FD_CLR(acceptor,&all);
    child *children = kids, *t;
    while (children) { // wait for children processes to finish
        int dunno;
        waitpid(children->pid,&dunno,0);
        t = children;
        children = children->next;
        delete t;
    }
    return 0;
}

void printstuff(char * n,int c,int m){
    fprintf(stdout,"msg: ");
    for(int i=0;i<c;i++) {
        char k = n[i];
        fprintf(stdout,"%02X|",k);
    }
    fprintf(stdout,"\ncount:%i out of %i\n",c,m);
}

int rpcCall(char* name, int* argTypes, void** args){
    int server = // get a file descriptor to talk to the server from the binder
            getFd(name, argTypes, args);
    if (server <= 0) {
        return -40;
    }
    int nbytes; // number of bytes sent per message
    int msg = htonl(EXECUTE); // to send 4-byte messages
    nbytes = send(server,&msg,4,0); // send EXECUTE to server

    msg = htonl(strlen(name) + 1);
    nbytes = send(server, &msg, 4, 0); // send length of funciton name
    
    msg = ntohl(msg);
    nbytes = send(server,name,msg,0); // send function name

    msg = htonl(atypelen(argTypes) + 1);
    nbytes = send(server, &msg, 4, 0); // send length of argument types
    
    msg=ntohl(msg);
    for(int i=0;argTypes[i]!=0;i++)
        argTypes[i] = htonl(argTypes[i]);
    nbytes = send(server,argTypes,msg*sizeof(int),0); // send argument types
    
    for(int i=0;argTypes[i]!=0;i++)
        argTypes[i] = ntohl(argTypes[i]);

    if(msg){// if there are arguments to send - could be zero
        // send all the args
        msg = htonl(arglen(argTypes));
        nbytes = send(server,&msg,4,0); // send size of arguments
        
        msg=ntohl(msg);
        char *argsout;
        argsout = new char[msg];
        copytoargs(argTypes,args,argsout); // copy arguments to array of char
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
    recv(server, &returncode, sizeof(returncode), 0);
    returncode = ntohl(returncode);

    if (returncode != EXECUTE_SUCCESS) { // something went wrong
        recv(server, &returncode, sizeof(returncode), 0);
        returncode = ntohl(returncode);
        close(server); 
        return returncode;
    }
    //recv namelen,name,argtypelen,argtypes,arglen,args
    nbytes = recv(server,&msg,4,0); // length of name
    
    char backname[ntohl(msg)];
    nbytes = recv(server,backname,ntohl(msg),0); // name of function

    nbytes = recv(server,&msg,4,0); // length of argument types

    int backtypes[ntohl(msg)+1];
    int backarglen = ntohl(msg);
    nbytes = recv(server, backtypes, ntohl(msg) * sizeof(int), 0); // argument types
    for(int i = 0; i < ntohl(msg); i++)
        backtypes[i] = ntohl(backtypes[i]);

    nbytes = recv(server,&msg,4,0); // size of arguments

    char *backargs;
    backargs = new char[ntohl(msg)];
    int backtotal;
    for(backtotal = 0; backtotal < ntohl(msg); backtotal += nbytes) {
        nbytes = recv(server,backargs,ntohl(msg),0); // receive arguments
    }
    
    backtotal = 0;
    for(int i = 0; i < atypelen(backtypes); i++) { // copy to arguments given in args
        int x = getsizelen(backtypes[i]);
        memcpy(args[i], backargs + backtotal, x);
        backtotal += x;
    }
    
    delete [] backargs;
    close(server);
    return 0;
}

// any client can call this to shut the whole system down
int rpcTerminate() {
    int nbytes;
    binder = getconnection();
    if (binder<=0) return binder;
    // send terminate request to binder
    long msgid = htonl(TERMINATE);
    nbytes = send(binder, &msgid, 4, 0);
    if (nbytes < 4) return -20; // couldn't send whole message
    return 0;
}

