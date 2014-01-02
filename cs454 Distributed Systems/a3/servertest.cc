#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

void titlecase(char *in, unsigned long size){
    int s = 1;
    for(unsigned long i=0;i<size;i++){
        if (in[i] == ' '){
            s=1;
            continue;
        }
        if(s){
            in[i] = toupper((int)(in[i]));
            s = 0;
        } else
            in[i] = tolower((int)(in[i]));
        
    }
}

int getacceptor(){
    addrinfo hints, *addlist;
    bzero((char *)&hints, sizeof(hints));
    hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    int addrerr = getaddrinfo(NULL, "0", &hints, &addlist);
    if (addrerr) {
        cerr << gai_strerror(addrerr) << endl;
        exit(-1);
    }
    int out = 0;
    for(addrinfo *temp = addlist; temp != NULL; temp = temp->ai_next) {
        out = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
        if (out < 0) { 
            out = 0;
	        continue;
        }
        int reuse = 1;
        setsockopt(out, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        sockaddr_in tsock;
        socklen_t tsocklen = sizeof(tsock);
        if (bind(out, temp->ai_addr, temp->ai_addrlen)<0 || getsockname(out,(sockaddr *)&tsock,&tsocklen)) {
            close(out);
            out = 0;
            continue;
        }
        int namelen=256;
        char name[namelen];
        gethostname(name,namelen);
        cout << "SERVER_ADDRESS " << name << "\nSERVER_PORT " << ntohs(tsock.sin_port) << endl;
        break;
    }
    if (!out) {
        cerr << "cannot bind" << endl;
        exit(-1);
    }
    
    freeaddrinfo(addlist);
    
    if (listen(out, 5) == -1) {
        cerr << "cannot listen" << endl;
        exit(-1);
    }
    return out;
}

int main(void){
    int acceptor = getacceptor();
    int highestclient = acceptor;
    
    fd_set all, accepting;
    FD_ZERO(&all);
    FD_ZERO(&accepting);
    FD_SET(acceptor, &all);
    highestclient = acceptor; 
    
    while(1) {
        accepting = all;
        if (select(highestclient+1, &accepting, NULL, NULL, NULL) == -1) {
            cerr << "could not select" << endl;
            exit(-1);
        }
        for(int i = 0; i <= highestclient; i++) {
            if (FD_ISSET(i, &accepting)) {
                if (i == acceptor) {
                    sockaddr_storage clientaddr;
                    socklen_t clientlen = sizeof(clientaddr);
                    int newclient = accept(acceptor,(sockaddr *)&clientaddr, &clientlen);
                    if (newclient >= 0) {
                        highestclient = max(highestclient,newclient);
                        FD_SET(newclient, &all);
                    }
                } else {
                    unsigned long messagelen;
                    long communicated = recv(i,&messagelen,4,0);
                    if(communicated <= 0){
                        close(i); 
                        FD_CLR(i, &all);
                        continue;
                    }
                    messagelen = ntohl(messagelen);
                    char *buf;
                    buf = new char[messagelen+1];
                    bzero(buf,sizeof(buf));
                    unsigned long total;
                    for(total=0; total<messagelen; total+=communicated){
                        communicated = recv(i,buf+total,messagelen-total,0);
                        if(communicated<=0) {
                            close(i);
                            FD_CLR(i,&all);
                            break;
                        }
                    }
                    if (total<messagelen) {
                        close(i);
                        FD_CLR(i,&all);
                        continue;
                    }
                    int pi;
                    for(int k=0;k<strlen(buf);k++){
                        pi = buf[k]-'0';
                        cout << hex << pi;
                    }
                    cout <<dec<< "  " << buf <<  endl;
                    delete buf;
                    
                } 
            } 
        }
    } 
    return 0;
}

