/*---画像データをUDPデータとして送信する---*/
/*---         PCとの通信用          ---*/
/*---      UDPClient_image.cpp    ---*/

/*---includeファイル---*/
#include <stdio.h>       // printf(),fprintf()
#include <sys/socket.h>  // socket(),connect(),sendto(),recvfrom()
#include <arpa/inet.h>   // sockaddr_in,inet_addr()
#include <stdlib.h>      // atoi(),exit()
#include <string.h>      // memset()
#include <unistd.h>      // close()
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <unistd.h>      // usleep() <--2018.10.30
#include <time.h>        //
#include <pthread.h>

/*---define---*/
#define ECHOMAX  524288    // エコー文字列の最大値
#define IMG_SIZE 320000   // 画像のサイズ

void DieWithError(char *errorMessage);
void Show_Time(struct timespec, struct timespec, struct timespec, struct timespec);
void* Recv(void* argc);

/*---グローバル変数---*/
int sock;
struct sockaddr_in fromAddr;       // エコー送信元のアドレス
char echoBuffer[ECHOMAX+1];   // エコー文字列の受信用バッファ

int main(int argc, char *argv[]){
  //int sock;                          // ソケット
  struct sockaddr_in echoServAddr[2];   // エコーサーバのアドレス
  
  unsigned short echoServPort;       // エコーサーバのポート番号
  char *servIP0;                      // サーバのIPアドレス
  char *servIP1;
  int waitsec = 15.0;                // =15.0us=0.001ms
  int num_of_tx = 320;
  
  int echoStringLen;                 // 文字列の長さ
  //int respStringLen;                 // 受信した応答の長さ
  std::vector<int> param = std::vector<int>(2);
  int i,j;
  int high,width,datasize;

  struct timespec startTime_r, endTime_r, startTime_c, endTime_c;

  if((argc < 2)||(argc > 3)){
    fprintf(stderr,"Usage: %s <Server IP0> [<Echo Port>]\n",argv[0]);
    exit(1);
  }
  
  servIP0 = argv[1];      // 1つ目の引数 : サーバのIPアドレス(ドット10進表記)
  //printf("宛先IPアドレス0 : %s\n",servIP0);

  if(argc == 3)
    echoServPort = atoi(argv[2]);  // 指定のポート番号があれば使用
  else
    echoServPort = 7;  // 7はエコーサービスのwell-knownポート番号

  /*---画像の取り込み---*/
  cv::Mat image = cv::imread("/home/tmitsuhashi/bin/opencv/800x400.bmp",cv::IMREAD_GRAYSCALE);
  if(image.empty()){
    std::cout << "read error.\n";
    return -1;
  }
  /*---画像の高さ,幅を表示---*/
  //high = image.rows;
  //width = image.cols;
  //datasize = high * width;

  /*---UDPデータグラムソケットの作成---*/
  if((sock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  /*---サーバのアドレス構造体の作成---*/
  memset(&echoServAddr[0], 0, sizeof(echoServAddr[0]));
  echoServAddr[0].sin_family = AF_INET;                 // インターネットアドレスファミリ
  echoServAddr[0].sin_addr.s_addr = inet_addr(servIP0); // サーバのIPアドレス
  echoServAddr[0].sin_port = htons(echoServPort);       // サーバのポート番号

  /*---1000バイトずつ送る---*/
  for(i=0;i<IMG_SIZE;i++){
    echoBuffer[i] = 255;
  }

  /*---マルチスレッド化---*/
  pthread_t thread;
  pthread_create(&thread,NULL,Recv,(void *)NULL);

  clock_gettime(CLOCK_MONOTONIC_RAW, &startTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime_c);

  /*---1つ目---*/
  for(i=0;i<num_of_tx;i++){
    if(sendto(sock, &image.data[i*1000], 1000, 0, (struct sockaddr *)&echoServAddr[0],
	      sizeof(echoServAddr[0])) != 1000)
	DieWithError("sendto() sent a different number of bytes than expected");
    usleep(waitsec);
  }

  pthread_join(thread,NULL);

  clock_gettime(CLOCK_MONOTONIC_RAW, &endTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime_c);
  Show_Time(startTime_r, endTime_r, startTime_c, endTime_c);
  
  /*--受信データの処理----*/
  cv::Mat recv_image;
  recv_image.create(400,800,CV_8UC1);
  int count = 0;
  count = 0;
  for(j=0;j<400;j++){
    for(i=0;i<800;i++){
      recv_image.data[count] = echoBuffer[count];
      count++;
    }
  }
  param[0]=cv::IMWRITE_PXM_BINARY;
  param[1]=1;
  cv::imwrite("/home/tmitsuhashi/bin/opencv/recv_pc_2.bmp",recv_image,param);

  close(sock);
  exit(0);
}

void DieWithError(char *errorMessage){
  perror(errorMessage);
  exit(0);
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
}

void* Recv(void* argc){
  int respStringLen;
  unsigned int fromSize;
  int i;

  fromSize = sizeof(fromAddr);

  for(i=0;i<320;i++){
    if((respStringLen = recvfrom(sock, echoBuffer+i*1000, 1000, 0,
	     (struct sockaddr *)&fromAddr, &fromSize)) != 1000)
      DieWithError("recvfrom() failed");
  }

}
