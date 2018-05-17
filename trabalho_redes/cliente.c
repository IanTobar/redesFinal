#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUFLEN 503
#define PORT 8885

void die(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in si_me, si_other;

    int s, i, j, slen = sizeof (si_other), recv_len;
    char buf[BUFLEN];

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    memset((char *) &si_me, '1', sizeof (si_me));
    //printf("%d",si_me);

    si_me.sin_family = AF_INET;
    si_me.sin_port = PORT;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1) {
        die("bind");
    }
    //memset(buf,0,503);
    char fname[20];
    FILE *fp;
    recv_len = recvfrom(s, buf, 20, 0, (struct sockaddr *) &si_other, &slen);

    char fna[100];
    memset(buf, 0, 503);

    recv_len = recvfrom(s, buf, 20, 0, (struct sockaddr *) &si_other, &slen);

    strcpy(fna, buf);
    //printf("%c\n",fna);
    int len = strlen(fna);
    printf("%d", len);

    unsigned long mm = atoi(buf);
    //printf("mm value: %ld\n",mm);

    fp = fopen(fna, "wb");
    int itr = 1;
    memset(buf, 0, 503);
    while (itr * 503 < mm) {
        if ((recv_len = recvfrom(s, buf, 503, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
            die("recvfrom()");
        }
        fwrite(buf, 503, 1, fp);
        memset(buf, 0, 503);
        //printf("Loop no: %d",i)
        //for(i=0;i<=itr;i++);

        itr++;
    }

    //printf("Loop no: %d\n",i);
    printf("%d", (mm % 503));
    recv_len = recvfrom(s, buf, (mm % 503), 0, (struct sockaddr *) &si_other, &slen);

    fwrite(buf, (mm % 503), 1, fp);
    memset(buf, 0, 503);
    fclose(fp);
    close(s);

    return 0;
}