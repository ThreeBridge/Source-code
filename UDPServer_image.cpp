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

#define ECHOMAX 60000       // エコー文字列の最大値

void DieWithError(char *errorMessage);

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
	std::vector<int> param = std::vector<int>(2);
	
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
	
	for (;;){
	  cliAddrLen = sizeof(echoClntAddr);
	  for(i=0;i<10;i++){
	    if((recvMsgSize = recvfrom(sock, echoBuffer+i*1000, 1000, 0,
				       (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
	  
	    //NULL,NULL)) < 0)

	    DieWithError("recvfrom() failed");
	  }
	  /*---受け取った画像データの処理---*/
	  cv::Mat image;
	  image.create(100,100,CV_8UC1);
	  count = 0;
	  for(j=0;j<100;j++){
	    for(i=0;i<100;i++){
	      image.data[count] = ~echoBuffer[count];
	      count++;
	    }
	  }
	  param[0]=cv::IMWRITE_PXM_BINARY;
	  param[1]=1;
	  cv::imwrite("recv.bmp",image,param);

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
	  for(i=0;i<10;i++){
	    if(sendto(sock, &image.data[i*1000], 1000, 0,
		    (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
	      DieWithError("sendto() sent a different number of bytes than expected");
	  }
	  
	}
        /*---ここには到達しない---*/
}

void DieWithError(char *errorMessage){
  perror(errorMessage);
  exit(1);
}
