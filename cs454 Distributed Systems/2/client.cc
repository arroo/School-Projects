#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

struct child {
    pid_t pid;
    child * next;
} *kids = NULL;

int getconnection(){
    
    int connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector < 0) cerr << "failed getting socket" << endl, exit(0);
    
    hostent *server = gethostbyname(getenv("SERVER_ADDRESS"));
    if (server == NULL) cerr << "failed finding server" << endl, exit(0);
    
    sockaddr_in servaddr;
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);
    servaddr.sin_port = htons(atoi(getenv("SERVER_PORT")));
    
    if (connect(connector,(sockaddr *) &servaddr,sizeof(servaddr)) < 0) cerr << "failed connecting" << endl, exit(0);
    
    return connector;
}

void communicate(int connection, string message){
    unsigned long len = message.size()+1, nel = htonl(len), communicated;
    char msg[len];
    strcpy(msg,message.c_str());
    if (send(connection,&nel,4,0) < 0) cerr << "failed sending" << endl, exit(0);
    for (unsigned long total=0; total<len; total+=communicated){
        communicated = send(connection,msg+total,len-total,0);//strlen(buffer));
        if (communicated <= 0) cerr << "failed sending" << endl, exit(0);
    }

    bzero(msg,len);
    for(unsigned long total=0;total<len;total+=communicated){
            communicated = recv(connection,msg+total,len-total,0);
            if (communicated<0) cerr << "failed receiving" << endl, exit(0);
    }
    cout << "Server: " << msg << endl;
    close(connection);
    exit(0);
}

void cleanup(int con){
    child *children = kids, *t;
    while(children){
        int dunno;
        waitpid(children->pid,&dunno,0);
        t = children;
        children = children->next;
        delete t;
    }
    write(con,NULL,0);
    close(con);
}

int main(int argc, char *argv[]){
    int connection = getconnection();
    while(1){
        string buffer="";
        if(getline(cin,buffer)){
            pid_t kid = fork();
            if(kid){
                child *newkid = new child;
                newkid->pid = kid;
                newkid->next = kids;
                kids = newkid;
                sleep(2);
            } else {
                communicate(connection,buffer);
            }
        } else {
            break;
        }
    }
    cleanup(connection);
    return 0;
}

