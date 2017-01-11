#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

char hostname[] = "127.0.0.1";
int portno = 20160;
struct sockaddr_in serveraddr;
int serverlen;

pthread_t thread_askco2;
pthread_t thread_readco2;
pthread_t thread_readpm;

int UART_CO2 = -1, UART_PM = -1, sockfd = -1;

void msleep(unsigned long ms){
    usleep(ms*1000);
}

void *askco2(void * pvoid){
    unsigned char askco2[7] = {0x42, 0x4D, 0xE3, 0x00, 0x00, 0x01, 0x72};
    while(1){
        write(UART_CO2, (void*)askco2, 7);
        sleep(1);
    }
}

unsigned char read_one(int fd){
    unsigned char rx_buffer[16]={0};
    while (read(fd, (void*)rx_buffer, 1) < 0){
        msleep(10);
    }
    return rx_buffer[0];
}

int co2 = 0, lastco2 = 0;
unsigned char co2s[12] = {0}, co2state = 0, update = 0;
void *readco2(void * pvoid){
    //42 4D 00 08 0A 4D 3C 7F 0A EA 02 9D
    while(1){
        co2s[co2state++] = read_one(UART_CO2);
        if( (co2state == 1) && (co2s[0] != 0x42) ) co2state = 0;
        else if( (co2state == 2) && (co2s[1] != 0x4d) ) co2state = 0;
        else if(co2state >= 12){
            co2 = (co2s[4]<<8) + co2s[5];
            co2state = 0;
            if(co2 != lastco2){
                lastco2 = co2;
                update = 1;
            }
        }
    }
}

unsigned char pm[32] = {0}, pmstate = 0;
int pm1_0=0, pm2_5=0, pm10=0;
void *readpm(void * pvoid){
    // 42 4D 00 1C 00 09 00 0B 00 0C 00 09 00 0B 00 0C 09 5A 01 E3 00 2B 00 04 00 01 00 00 91 00 02 F3
    while(1){
        pm[pmstate++] = read_one(UART_PM);
        if( (pmstate == 1) && (pm[0] != 0x42) ) pmstate = 0;
        else if( (pmstate == 2) && (pm[1] != 0x4d) ) pmstate = 0;
        else if(pmstate >= 32){
            pm1_0 = (pm[10]<<8) + pm[11];
            pm2_5 = (pm[12]<<8) + pm[13];
            pm10 = (pm[14]<<8) + pm[15];
            pmstate = 0;
            update = 1;
        }
    }
}

void error(char *msg) {
    perror(msg);
    exit(0);
}

int udpconnect(){
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server = (struct hostent *)gethostbyname(hostname);
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)(server->h_addr),
        (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    serverlen = sizeof(serveraddr);
    return sockfd;
}

void udpsend(unsigned char * data){
    int n = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&serveraddr, serverlen);
}

int main() {
    UART_CO2 = open("/dev/ttyAMA2", O_RDWR | O_NOCTTY | O_NDELAY);
    UART_PM = open("/dev/ttyAMA3", O_RDWR | O_NOCTTY | O_NDELAY);
    if ( (UART_CO2 == -1) || (UART_PM == -1) ){
        perror("Open Serial Port Error!\n");
        return -1;
    }

    struct termios options_co2, options_pm;

    tcgetattr(UART_CO2, &options_co2);
    options_co2.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options_co2.c_iflag = IGNPAR;
    options_co2.c_oflag = 0;
    options_co2.c_lflag = 0;
    options_co2.c_cc[VTIME] = 0;
    options_co2.c_cc[VMIN] = 1;
    tcflush(UART_CO2, TCIFLUSH);
    tcsetattr(UART_CO2, TCSANOW, &options_co2);

    tcgetattr(UART_PM, &options_pm);
    options_pm.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options_pm.c_iflag = IGNPAR;
    options_pm.c_oflag = 0;
    options_pm.c_lflag = 0;
    options_pm.c_cc[VTIME] = 0;
    options_pm.c_cc[VMIN] = 1;
    tcflush(UART_PM, TCIFLUSH);
    tcsetattr(UART_PM, TCSANOW, &options_pm);

    if(pthread_create(&thread_askco2, NULL, askco2, NULL) < 0)
        error("create thread error\n");

    if(pthread_create(&thread_readco2, NULL, readco2, NULL) < 0)
        error("create thread error\n");

    if(pthread_create(&thread_readpm, NULL, readpm, NULL) < 0)
        error("create thread error\n");

    while(udpconnect() < 0){
        printf("udp connect failed. rerty\n");
        sleep(1);
    }

    char buf[128];
    while(1){
        if(update != 0){
            update = 0;
            sprintf(buf, "PM1.0: %d, PM2.5: %d, PM10: %d, CO2: %d", pm1_0, pm2_5, pm10, co2);
            printf("%s\n", buf);
            udpsend(buf);
            msleep(10);
        }
    }
    return 0;
}