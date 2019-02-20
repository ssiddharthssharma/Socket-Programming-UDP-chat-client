
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define STDIN 0
#define STDOUT 1


#define PATH_SIZE 200
#define HOST_SIZE 100
#define BUFF_SIZE 256


#define CHECK(X) ({int __val = (X); (__val == -1 ? \
                 ({fprintf(stderr,"ERROR ("__FILE__":%d) --%s\n", __LINE__,strerror(errno)); \
                 exit(-1);-1;}) : __val); })


char* timecalc() {
    
    char *t = malloc(26);
    time_t timer;
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    strftime(t, 26, "%Y/%m/%d %H:%M:%S", tm_info);
    return t;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    //char* timenow = malloc(26); //time at that instant   
    char buffs[BUFF_SIZE];      // send buffer
    char buffr[BUFF_SIZE];      // recieve buffer
    int bsend, brecv;           //number of bytes sent and recieved

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;

    char s[INET6_ADDRSTRLEN];
    //char ip[NI_MAXHOST], port[NI_MAXSERV];


    char path[PATH_SIZE];
    char host[HOST_SIZE];

    if (argc != 3){fprintf(stderr, "Usage: %s http://<server ip> <server port>\n", argv[0]); exit(-1);}
 
    if (sscanf(argv[1], "http://%99[^/]/%199[^\n]", host, path) == 2){}
    else if (sscanf(argv[1], "http://%99[^/]/[^\n]", host) == 1){}
    else if (sscanf(argv[1], "http://%99[^\n]", host) == 1){}
    else {fprintf(stderr, "the host address should be preceeded by http://\n"); exit(-1);}

    printf("CMPE 207 HW4 udpchatclient Siddharth Sharma 334\n");

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if ((rv = getaddrinfo(host, argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return 2;
    }


   
    fd_set write_fds;
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax=2;        // maximum file descriptor number
    int nfds;
    
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(STDOUT, &write_fds);

    while(1){

        FD_SET(STDIN, &read_fds);
        FD_SET(sockfd, &read_fds);

        CHECK(nfds = select(sockfd+1, &read_fds, NULL, NULL, NULL));

             if (FD_ISSET(STDIN, &read_fds)) {
                
                bzero(buffs,256);

                fgets(buffs,BUFF_SIZE,stdin);
                CHECK(bsend = sendto(sockfd, buffs, sizeof(buffs), 0, p->ai_addr, p->ai_addrlen));
                printf("\n%s <localhost>:\t%s\n", timecalc(), buffs);
                FD_CLR(STDIN, &read_fds);
             } else if (FD_ISSET(sockfd, &read_fds)){

                bzero(buffr,256);
                CHECK(brecv = recvfrom(sockfd, buffr, sizeof (buffr) , 0, (struct sockaddr *)&their_addr, &addr_len));
                if(strcmp(buffr,"<<Server Busy>>\n")==0){exit(0);}
                printf("\n%s <%s>:\t%s\n",timecalc(), inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s), buffr );
                FD_CLR(sockfd, &read_fds);

             }
            //
            //fgets(buffs,BUFF_SIZE,stdin);

            //CHECK(bsend = sendto(sockfd, buffs, sizeof(buffs), 0, p->ai_addr, p->ai_addrlen));
            //printf("%s <localhost>:\t%s\n", timecalc(), buffs);


    }


    freeaddrinfo(servinfo);
    close(sockfd);
    return 0;
}