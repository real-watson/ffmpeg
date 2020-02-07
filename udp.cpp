#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


typedef struct RTP_FIXED_HEADER{
    /* byte 0 */
    unsigned char csrc_len:4;       /* expect 0 */
    unsigned char extension:1;      /* expect 1 */
    unsigned char padding:1;        /* expect 0 */
    unsigned char version:2;        /* expect 2 */
    /* byte 1 */
    unsigned char payload:7;
    unsigned char marker:1;        /* expect 1 */
    /* bytes 2, 3 */
    unsigned short seq_no;
    /* bytes 4-7 */
    unsigned  long timestamp;
    /* bytes 8-11 */
    unsigned long ssrc;            /* stream number is used here. */
} RTP_FIXED_HEADER;

typedef struct MPEGTS_FIXED_HEADER {
    unsigned sync_byte: 8;
    unsigned transport_error_indicator: 1;
    unsigned payload_unit_start_indicator: 1;
    unsigned transport_priority: 1;
    unsigned PID: 13;
    unsigned scrambling_control: 2;
    unsigned adaptation_field_exist: 2;
    unsigned continuity_counter: 4;
} MPEGTS_FIXED_HEADER;

int main(int argc, char *argv[])
{

    int cnt=0;
    FILE *myout=stdout;
    FILE *fp1=fopen("output.mp4","wb+");

    int port=23456;
    int lfd,ret;
    lfd = socket(AF_INET,SOCK_DGRAM,0);

    //封装套结字地址结构
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    /*接收端端口号*/
    saddr.sin_port = htons(port);
    /*接收端IP，此处设置为回环地址*/
    saddr.sin_addr.s_addr = htons(INADDR_ANY);
    ret = bind(lfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret < 0)
    {
        perror("bind fail:");
        return -1;
    }

    sockaddr_in remoteAddr;
    //int nAddrLen = sizeof(remoteAddr);

        //How to parse?
    int parse_rtp=1;
    int parse_mpegts=1;

    printf("Listening on port %d\n",port);
    char recvData[10000];
    socklen_t addrlen;
    //connect to the server
    if (connect(lfd,(struct sockaddr*)&saddr,sizeof(struct sockaddr)) == -1)
    	return -1;

    while (1) {
	printf("helloworld\n");
        int pktSize=recvfrom(lfd,recvData,10000,0,(sockaddr *)&remoteAddr,&addrlen);
        if(pktSize>0){

            if(parse_rtp!=0){

                char payload_str[10]={0};
                RTP_FIXED_HEADER rtp_header;
                int rtp_header_size=sizeof(RTP_FIXED_HEADER);
                memcpy((void *)&rtp_header,recvData,rtp_header_size);

                char payload=rtp_header.payload;
                switch(payload){
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                case 4:
                                case 5:
                                case 6:
                                case 7:
                                case 8:
                                case 9:
                                case 10:
                                case 11:
                                case 12:
                                case 13:
                                case 14:
                                case 15:
                                case 16:
                                case 17:
                                case 18: sprintf(payload_str,"Audio");break;
                                case 31: sprintf(payload_str,"H.261");break;
                                case 32: sprintf(payload_str,"MPV");break;
                                case 33: sprintf(payload_str,"MP2T");break;
                                case 34: sprintf(payload_str,"H.263");break;
                                case 96: sprintf(payload_str,"H.264");break;
                                default:sprintf(payload_str,"other");break;
                                }

                unsigned int timestamp=ntohl(rtp_header.timestamp);
                unsigned int seq_no=ntohs(rtp_header.seq_no);
                fprintf(myout,"[RTP Pkt] %5d| %5s| %10u| %5d| %5d|\n",cnt,payload_str,timestamp,seq_no,pktSize);

                //RtpData
                char *rtp_data=recvData+rtp_header_size;
                int rtp_data_size=pktSize-rtp_header_size;
                fwrite(rtp_data,rtp_data_size,1,fp1);

                if(parse_mpegts!=0 && payload==33){

                    MPEGTS_FIXED_HEADER mpegpts_header;
                    for(int i=0;i<rtp_data_size;i++){

                        if(rtp_data[i]!=0x47)
                          break;
                        //MPEGTS Header;
                        memcpy((void *)&mpegpts_header,rtp_data+i,sizeof(MPEGTS_FIXED_HEADER));
                        fprintf(myout,"   [MPEGTS Pkt]\n");
                    }

                }else {

                    fprintf(myout,"[UDP Pkt] %5d| %5d|\n",cnt,pktSize);
                    fwrite(recvData,pktSize,1,fp1);
                }

                cnt++;

            }


        }
    }


    return 0;
}















