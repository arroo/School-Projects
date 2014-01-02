#include "server_stuff.h"

int rpcInit() {
    listener = getacceptor();
    if(listener < 0) return listener;
#ifndef NOBINDER
    binder = getconnection(); // = 1;
    if(binder < 0) {
        close(listener); 
        return binder;
    }
#endif
    list.head=NULL;
    list.tail=NULL;
    return 0;
}
