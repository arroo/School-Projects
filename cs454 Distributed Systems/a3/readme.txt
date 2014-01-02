This is a project from my Distributed Systems course. 

The project is to have a single Binder running on a server somewhere that will allow for
remote procedure calls and have multiple Function Servers connect to the Binder to register functions.
I did the work for the Function Servers, my partner did the Binder. 

The way to run the setup is to run the Binder and it will print to standard out its hostname and the
port it is running on, the servers and the clients are supposed to put those into environment variables
in order to connect.

In order to make additional servers, a programmer must first call rpcInit, that will tell the Binder 
to start listening, then set up a function signature. That means calling rpcRegister with the name of 
the function, the argument types, and a function skeleton. Finally the server calls rpcExecute which
returns only when the binder sends any message. Until then the server just runs, waiting for connections.

In order to make additional clients, a programmer must call rpcCall using a function signature, and 
the arguments in a single array. Any client may shut down the whole system by calling rpcTerminate.

This comes with a sample client and server that were given to the class for simple testing.


