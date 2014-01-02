#include "server_stuff.h"
// this is the function that a server will call once all of its functions have been registered with the binder
// this function returns once the binder has received a TERMINATE command
// and it can work on multiple calls at the same time.
int rpcExecute() {
    
    struct child { // a linked list of child threads
        pid_t pid;
        child * next;
    } *kids = NULL;
    
    int highestclient = std::max(binder, listener); // only two clients so far
    int acceptor = listener;
    
    fd_set all, accepting;
    
    FD_ZERO(&all); // every FD
    FD_ZERO(&accepting); // listening on this FD_SET
    
    FD_SET(listener,&all); // listening FD
    FD_SET(binder,&all); // a connection to the binder
    
    int quit=0; // used for when a client calls rpcTerminate
    while(1) {
        if (quit) break;
        accepting = all;
        if (select(highestclient+1, &accepting, NULL, NULL, NULL) == -1) {
            // something went wrong, exit
            return -1;
        }
        for(int i = 0; i <= highestclient; i++) if (FD_ISSET(i, &accepting))
            // a fairly simple but effective way to act on communicating clients
            if (i == acceptor) {
                sockaddr_storage clientaddr;
                socklen_t clientlen = sizeof(clientaddr);
                int newclient = accept(acceptor,(sockaddr *)&clientaddr, &clientlen);
                if (newclient >= 0) {
                    highestclient = std::max(highestclient,newclient);
                    FD_SET(newclient, &all);
                }
            } else if (i == binder) { // the only time the binder should talk to a server
                                      // is when rpcTerminate is called
                int msg;
                recv(i,&msg,4,0);
                if (ntohl(msg)==TERMINATE) quit = 1;
            } else { // 
                // receive message
                pid_t kid = fork();
                if (kid) { // allow the parent to continue listening for connections
                    close(i);
                    FD_CLR(i,&all);
                    child *newkid = new child; // add the child to the list
                    newkid->pid = kid;
                    newkid->next = kids;
                    kids = newkid;
                    continue;
                }
                close(binder);   // don't need to listen
                close(acceptor); // any more
                int msg = 0;
                if(recv(i, &msg, sizeof(msg), 0) == 0) { // something went wrong
                    close(i);
                    exit(0);
                }
                
                if (msg != htonl(EXECUTE)) { // there's nothing else that should
                                             // come from a client
                    msg = htonl(EXECUTE_FAILURE);
                    send(i,&msg,sizeof(msg),0);
                    msg = htonl(-1101);
                    send(i,&msg,sizeof(msg),0);
                    close(i);
                    exit(0);
                }
                
                int length = 0;
                recv(i,&length,4,0);
                length = ntohl(length); // length of the function name
                
                char *fname;
                fname = new char[length];
                recv(i,fname,length,0); // get the function name
                
                recv(i,&length,4,0); // the length of the argument types
                length = ntohl(length);
           
                int *atypes,
                    arrg = length;
                char ** argsarr;
                atypes = new int[length];
                argsarr = new char*[length];
           
                
                recv(i, atypes, length * sizeof(int), 0); // the argument types
                for(int j=0;j<length;j++)
                    atypes[j] = ntohl(atypes[j]);
                
                recv(i,&length,4,0); // the length of the arguments
                length = ntohl(length);
                
                char *args;
                args = new char[length]; // store the arguments in an 
                                         // array of char
                
                int totalarg = 0, // total arguments received
                    nbytes;       // bytes received at a time
                for(; totalarg < length; totalarg += nbytes)
                    nbytes = recv(i,args,length,0); // receive all of the arguments

                totalarg = 0;
                for(int j = 0; j < arrg; j++){
                    argsarr[j] = totalarg + args; // make an argument array for the function
                    totalarg += getsizelen(atypes[j]); // there are no delimiters so must determine
                                                       // the size of each argument on the fly
                }
                
                skel *t;
                for(t = list.head; t != NULL; t = t->next) { // compare the received function signature
                                                             // if it's a match, run the function
                    if (strcmp(t->name, fname) == 0) {
                        if(typecompatible(t->types, atypes) == 0)
                            break;
                    }
                }
                if (t) { // successful finding of a function to run
                    int result = t->f(atypes,(void**)argsarr); // run the function
                    
                    if (result == 0) { // good stuff!
                        int nbytes;
                        msg = htonl(EXECUTE_SUCCESS); // send success message
                        nbytes = send(i, &msg, 4, 0);

                        msg = htonl(strlen(fname)); // send function name, part of protocol
                        nbytes = send(i, &msg, 4, 0);

                        nbytes = send(i, fname, ntohl(msg), 0);

                        msg = htonl(atypelen(atypes)); // send argument types' length
                        nbytes = send(i, &msg, 4, 0);

                        for (int j = 0; j < ntohl(msg); j++)
                            atypes[j] = htonl(atypes[j]);
                        nbytes = send(i,atypes,ntohl(msg)*sizeof(int),0); // send the argument types

                        totalarg = 0;
                        int k;
                        for(k = 0;k < ntohl(msg); k++) {
                            totalarg += getsizelen(ntohl(atypes[k]));
                        }

                        totalarg = htonl(totalarg);
                        nbytes = send(i, &totalarg, 4, 0); // send the total argument size

                        totalarg = ntohl(totalarg);
                        char backargs[totalarg];
                        for(int j = 0; atypes[j] != 0; j++)
                            atypes[j] = ntohl(atypes[j]);

                        copytoargs(atypes,(void**)argsarr,backargs);
                        nbytes = send(i,&backargs,totalarg,0); // send the arguments back

                    } else { // execution resulted in an error, return the error
                        int backresult = htonl(EXECUTE_FAILURE);
                        send(i,&backresult, 4, 0);
                        backresult = htonl(result);
                        send(i,&backresult, 4, 0);
                    }
                } else { // the function signature didn't match any existing registered functions
                    int backresult = htonl(EXECUTE_FAILURE);
                    send(i,&backresult, 4, 0);
                    backresult = htonl(-3001);
                    send(i,&backresult, 4, 0);
                }
                // all done
                close(i);
                delete args;
                delete fname;
                delete atypes;
                exit(0);
            } 
    } 
    for(int i = 0; i < highestclient + 1; i++) if (FD_ISSET(i, &all)) {
        close(i); // cleanup
        FD_CLR(i,&all);
    }

    close(binder); // stop listening to the binder
    FD_CLR(binder,&all);

    close(acceptor);
    FD_CLR(acceptor,&all);
    child *children = kids, *t;
    while (children) { // wait for the children to finish executing
        int dunno;
        waitpid(children->pid, &dunno, 0);
        t = children;
        children = children->next;
        delete t;
    }
    return 0;
}
