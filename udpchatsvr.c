
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define BUFF_SIZE 256


struct sockaddr_storage their_addr;


#define CHECK(X) ({int __val = (X); (__val == -1 ? \
                 ({fprintf(stderr,"ERROR ("__FILE__":%d) --%s\n", __LINE__,strerror(errno)); \
                 exit(-1);-1;}) : __val); })

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


char* timecalc() {
    
    char *t = malloc(26);
    time_t timer;
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    strftime(t, 26, "%Y/%m/%d %H:%M:%S", tm_info);
    return t;
}


void servbusy(int sockfd){

    struct sockaddr_storage drop_addr;
    socklen_t addr_len;
    addr_len = sizeof drop_addr;
    int xsend;
    char buffexit[BUFF_SIZE];

    CHECK(recvfrom(sockfd, buffexit, sizeof(buffexit) , 0, (struct sockaddr *)&drop_addr, &addr_len));

    strcpy(buffexit,"<<Server Busy>>\n");

    struct sockaddr_in *sin = (struct sockaddr_in *)&their_addr;
    struct sockaddr_in *sin2 = (struct sockaddr_in *)&drop_addr;

    if (sin != sin2){
        CHECK(xsend = sendto(sockfd, buffexit, sizeof(buffexit), 0,  (struct sockaddr *)&drop_addr, addr_len));
    }


}



int main(int argc, char const *argv[])
{

    if(argc < 2){ fprintf(stderr, "Usage: %s <server port>\n\n", argv[0]); exit(-1);}
    printf("CMPE 207 HW4 udpchatsvr Siddharth Sharma 334\n");

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
    
    socklen_t addr_len;
    addr_len = sizeof their_addr;  
    
    char s[INET6_ADDRSTRLEN];
    int bsend, brecv;           //number of bytes sent and recieve
    char buffr[BUFF_SIZE];      // recieve buffer
    char buffs[BUFF_SIZE];      // send buffer
    char* timenow = malloc(26); //time at that instant 
   

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Server: failed to bind socket\n");
        return 2;
    }



    printf("Server: waiting to recvfrom...\n");


    //fd_set rmaster;    // master file descriptor list
    //fd_set wmaster;
    fd_set write_fds;
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax=sockfd +1;        // maximum file descriptor number
    int nfds;


    //FD_ZERO(&rmaster);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    //FD_ZERO(&wmaster);

    
    while (1){ 

        
        
        FD_SET(fileno(stdin), &read_fds);
        FD_SET(sockfd, &read_fds);
        //FD_SET(fileno(stdout), &write_fds);

        CHECK(nfds = select(fdmax, &read_fds, NULL, NULL, NULL));


        if(FD_ISSET(sockfd, &read_fds)) {  
            FD_CLR(sockfd, &read_fds);
            bzero(buffr,256);
            CHECK( brecv = recvfrom(sockfd, buffr, sizeof(buffr) , 0, (struct sockaddr *)&their_addr, &addr_len) );
            printf("%s <%s>:\t%s\n",timecalc(), inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s), buffr );
            
            if(FD_ISSET(sockfd, &read_fds)) {
                FD_CLR(sockfd, &read_fds);
                servbusy(sockfd);
            }

       } else if (FD_ISSET(fileno(stdin), &read_fds)) {
            FD_CLR(fileno(stdin), &read_fds);
            bzero(buffs,256);
            fgets(buffs,BUFF_SIZE,stdin);
            CHECK(bsend = sendto(sockfd, buffs, sizeof(buffs), 0,  (struct sockaddr *)&their_addr, addr_len));
            printf("%s <localhost>:\t%s\n", timecalc(), buffs);

            if(FD_ISSET(sockfd, &read_fds)) {
                FD_CLR(sockfd, &read_fds);
                servbusy(sockfd);
            }
            
        }
   
    }
    freeaddrinfo(servinfo);
    close(sockfd);
    return 0;
}
