/*--includeファイル--*/
#include <stdio.h>        // printf(),fprintf()
#include <string.h>       // memset()
#include <unistd.h>       // close()
#include <sys/types.h>    
#include <sys/socket.h>   // socket(),bind()
#include <netinet/in.h>
#include <arpa/inet.h>    // sockaddr_in,inet_ntoa()
#include <errno.h>
#include <stdlib.h>       // atoi()
#include <opencv2/opencv.hpp>
#include <time.h>

#define ECHOMAX 320000       // エコー文字列の最大値

void DieWithError(char *errorMessage);
void Show_Time(struct timespec, struct timespec, struct timespec, struct timespec);

int main(int argc, char *argv[]){
        int sock;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;
	unsigned int cliAddrLen;
	char echoBuffer[ECHOMAX];
	char echoBuffer_tx[ECHOMAX];
	unsigned short echoServPort;
	int recvMsgSize;
	int i,j;
	int count;
	int num_of_packet = 320;
	std::vector<int> param = std::vector<int>(2);

        struct timespec startTime_r, endTime_r, startTime_c, endTime_c;
	struct timespec recordTime[15];
	if (argc != 2){
	  fprintf(stderr,"Usage: %s <UDP SERVER PORT>\n",argv[0]);
	  exit(1);
	}

        echoServPort = atoi(argv[1]);  // 1つ目の引数:ローカルポート番号
	
	if ((sock  = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	  DieWithError("socket() failed");
        
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_port = htons(echoServPort);
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))<0)
	  DieWithError("bind() failed");

	cv::Mat image;
	image.create(400,800,CV_8UC1);
	
	for (;;){
	  cliAddrLen = sizeof(echoClntAddr);
	  for(i=0;i<num_of_packet;i++){
	    if((recvMsgSize = recvfrom(sock, echoBuffer+i*1000, 1000, 0,
				       (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0){
	      //NULL,NULL)) < 0)

	      DieWithError("recvfrom() failed");
	    }
	    if(i==0){
	      clock_gettime(CLOCK_MONOTONIC_RAW, &startTime_r);
	    }
	    
	  }
	  clock_gettime(CLOCK_MONOTONIC_RAW, &recordTime[0]);
	  /*---受け取った画像データの処理---*/
	  //cv::Mat image;
	  //image.create(100,100,CV_8UC1);
	  count = 0;
	  for(j=0;j<400;j++){
	    for(i=0;i<800;i++){
	      image.data[count] = ~echoBuffer[count];
	      count++;
	    }
	  }
	  param[0]=cv::IMWRITE_PXM_BINARY;
	  param[1]=1;
	  //cv::imwrite("recv.bmp",image,param);
	  clock_gettime(CLOCK_MONOTONIC_RAW, &recordTime[1]);
	  /*
	  printf("Handing client %s\n",inet_ntoa(echoClntAddr.sin_addr));
	  printf("Recieved : %s\n",echoBuffer);
	  */
	  /*
	  for (i=0;i<recvMsgSize;i++){
	    printf("%d\n",echoBuffer[i]);
	  }
	  */
	  /*---返信---*/
	  /*
	  count = 0;
	  for(j=0;j<100;j++){
	    for(i=0;i<100;i++){
	      echoBuffer_tx[count] = ~echoBuffer[count];
	      count++;
	    }
	  }
	  */
	  for(i=0;i<num_of_packet;i++){
	    if(sendto(sock, &image.data[i*1000], 1000, 0,
		    (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
	      DieWithError("sendto() sent a different number of bytes than expected");
	  }
	  clock_gettime(CLOCK_MONOTONIC_RAW, &endTime_r);
	  printf("----");
	  Show_Time(startTime_r, recordTime[0], startTime_c, endTime_c);
	  printf("----");
	  Show_Time(recordTime[0], recordTime[1], startTime_c, endTime_c);
	  printf("----");
	  Show_Time(startTime_r, endTime_r, startTime_c, endTime_c);

	  printf("\n\n");
	  cv::imwrite("recv.bmp",image,param);
	}
        /*---ここには到達しない---*/
}

void DieWithError(char *errorMessage){
  perror(errorMessage);
  exit(1);
}

void Show_Time(struct timespec startTime_r, struct timespec endTime_r, struct timespec startTime_c, struct timespec endTime_c)
{
    //printf("経過実時間 = ");

    if(endTime_r.tv_nsec < startTime_r.tv_nsec) {
       printf("%1ld.%09ld", endTime_r.tv_sec - startTime_r.tv_sec - 1
             ,endTime_r.tv_nsec + 1000000000 - startTime_r.tv_nsec);
    }else{
       printf("%1ld.%09ld", endTime_r.tv_sec - startTime_r.tv_sec
           ,endTime_r.tv_nsec - startTime_r.tv_nsec);
    }
    printf("\n");

    /*
    printf("CPU時間 = ");
    printf("%10ld.%09ld", endTime_c.tv_sec - startTime_c.tv_sec
                        ,endTime_c.tv_nsec - startTime_c.tv_nsec);
    printf("s\n");
    printf("---経過実時間---\n");
    printf("開始/終了(s) %10ld/%10ld\n",startTime_r.tv_sec,endTime_r.tv_sec);
    printf("開始/終了(ns) %.09ld/%.09ld\n",startTime_r.tv_nsec,endTime_r.tv_nsec);
    
    printf("\n---CPU時間---\n");
    printf("開始/終了(s) %10ld/%10ld\n",startTime_c.tv_sec,endTime_c.tv_sec);
    printf("開始/終了(ns) %.09ld/%.09ld\n",startTime_c.tv_nsec,endTime_c.tv_nsec);
    */
}
