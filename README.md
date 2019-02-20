# Socket-Programming-UDP-chat-client
We use asynchronous I/O for UDP chat between Server and Client using non-blocking read and write.

To compile the code using gcc


 Use: 
 
      gcc udpchatsvr.c -o udpchatsvr
 
 
      gcc udpchatclient.c -o udpchatclient
      
Execute the server in 1st machine specifying the port,use sudo if using port number < 1024


      ./udpchatsvr <server port>
Execute the client in 2nd machine


      ./udpchatclient <server IP> <listening port>
  
  
