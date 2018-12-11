/*---画像データをUDPデータとして送信する---*/
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
  //unsigned int fromSize;             // recvfrom()のアドレスの入出力サイズ
  char *servIP0;                      // サーバのIPアドレス
  char *servIP1;
  int waitsec = 15.0;                // =15.0us=0.001ms
  int num_of_tx = 160;
  
  //std::vector<unsigned char> i_imageBuffer;             // エコーサーバへ送信する画像データ
  
  int echoStringLen;                 // 文字列の長さ
  //int respStringLen;                 // 受信した応答の長さ
  std::vector<int> param = std::vector<int>(2);
  int i,j;
  int high,width,datasize;

  struct timespec startTime_r, endTime_r, startTime_c, endTime_c;

  //unsigned char *imageBuffer;

  if((argc < 3)||(argc > 4)){
    fprintf(stderr,"Usage: %s <Server IP0> <Server IP1>  [<Echo Port>]\n",argv[0]);
    exit(1);
  }
  
  servIP0 = argv[1];      // 1つ目の引数 : サーバのIPアドレス(ドット10進表記)
  servIP1 = argv[2];      // add 2018.12.5
  printf("宛先IPアドレス0 : %s\n",servIP0);
  printf("宛先IPアドレス1 : %s\n",servIP1);

  if(argc == 4)
    echoServPort = atoi(argv[3]);  // 指定のポート番号があれば使用
  else
    echoServPort = 7;  // 7はエコーサービスのwell-knownポート番号
  //del(0)
  //printf("ポート番号     : %s\n",argv[2]);

  /*---画像の取り込み---*/
  cv::Mat image = cv::imread("/home/tmitsuhashi/bin/opencv/800x400.bmp",cv::IMREAD_GRAYSCALE);
  if(image.empty()){
    std::cout << "read error.\n";
    return -1;
  }
  /*---画像の高さ,幅を表示---*/
  high = image.rows;
  width = image.cols;
  datasize = high * width;
  /*<---del(0)
  std::cout << "画像の高さ     : " << image.rows << "\n";
  std::cout << "画像の幅       : " << image.cols << "\n";
  std::cout << "データサイズ    : " << datasize << "\n";
  --->*/
  //printf("%s\n",image.row(0));


  /*---メモリバッファに画像を書き出す---*/
  /*
  param[0]=cv::IMWRITE_PXM_BINARY;
  param[1]=0;
  imencode(".bmp", image, i_imageBuffer, param);
  */
  /*------*/
  /*
  printf("%d\n",i_imageBuffer.size());
  imageBuffer = (unsigned char*)malloc(i_imageBuffer.size());
  for(i=0;i<i_imageBuffer.size();i++) imageBuffer = i_imageBuffer[i];
  */
  /*
  printf("データサイズ   : %d\n",sizeof(imageBuffer));
  for(i=0;i<sizeof(imageBuffer);i++) printf("%d ",imageBuffer[i]);
  //printf("%s\n",imageBuffer);
  */

  /*---UDPデータグラムソケットの作成---*/
  if((sock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  /*---サーバのアドレス構造体の作成---*/
  //<---
  // 複数のデバイスに送信する
  //--->
  memset(&echoServAddr[0], 0, sizeof(echoServAddr[0]));
  echoServAddr[0].sin_family = AF_INET;                 // インターネットアドレスファミリ
  echoServAddr[0].sin_addr.s_addr = inet_addr(servIP0); // サーバのIPアドレス
  echoServAddr[0].sin_port = htons(echoServPort);       // サーバのポート番号

  memset(&echoServAddr[1], 0, sizeof(echoServAddr[1]));
  echoServAddr[1].sin_family = AF_INET;                 // インターネットアドレスファミリ
  echoServAddr[1].sin_addr.s_addr = inet_addr(servIP1); // サーバのIPアドレス
  echoServAddr[1].sin_port = htons(echoServPort);       // サーバのポート番号

  /*---画像データをサーバに送信---*/
  /*
  for(j=0;j<high;j++){
    for(i=0;i<width;i++){
      if(sendto(sock, image.data, datasize, 0, (struct sockaddr *)&echoServAddr,
      		sizeof(echoServAddr)) >= ECHOMAX)
      	DieWithError("sendto() sent a different number of bytes than expected");
    }
  }
  */
  /*---1000バイトずつ送る---*/

  for(i=0;i<IMG_SIZE;i++){
    echoBuffer[i] = 255;
  }


  //usleep(1000);
  /*del(0)
  clock_gettime(CLOCK_MONOTONIC_RAW, &endTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime_c);
  Show_Time(startTime_r, endTime_r, startTime_c, endTime_c);
  */

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

  // add 2018.12.6
  //fromSize = sizeof(fromAddr);

  /*---1つ目の応答---*/
  /*
  for(i=0;i<num_of_tx;i++){
    if((respStringLen = recvfrom(sock, echoBuffer+i*1000, 1000, 0,
	     (struct sockaddr *)&fromAddr, &fromSize)) != 1000)
      DieWithError("recvfrom() failed");
  }
  if(echoServAddr[0].sin_addr.s_addr != fromAddr.sin_addr.s_addr){
    fprintf(stderr,"Error : received a packet from unknown source.\n");
    exit(1);
  }
  */

  /*---2つ目---*/
  
  for(i=160;i<num_of_tx+160;i++){
    if(sendto(sock, &image.data[i*1000], 1000, 0, (struct sockaddr *)&echoServAddr[1],
	      sizeof(echoServAddr[1])) != 1000)
	DieWithError("sendto() sent a different number of bytes than expected");
    usleep(waitsec);
  }
  
  
  /*---2つ目の応答---*/
  /*
  for(i=10;i<num_of_tx+10;i++){
    if((respStringLen = recvfrom(sock, echoBuffer+i*1000, 1000, 0,
	     (struct sockaddr *)&fromAddr, &fromSize)) != 1000)
      DieWithError("recvfrom() failed");
  }
  */
  /*
  if(echoServAddr[1].sin_addr.s_addr != fromAddr.sin_addr.s_addr){
    fprintf(stderr,"Error : received a packet from unknown source.\n");
    exit(1);
  }
  */

  pthread_join(thread,NULL);

  clock_gettime(CLOCK_MONOTONIC_RAW, &endTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime_c);
  Show_Time(startTime_r, endTime_r, startTime_c, endTime_c);

  /*---受信データをNULL文字で終端させる---*/
  /*
  echoBuffer[respStringLen] = '\0';
  printf("Received : %s\n",echoBuffer);  // 引数のエコー文字列を表示
  */
  
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
  cv::imwrite("/home/tmitsuhashi/bin/opencv/recv_fpga_4.bmp",recv_image,param);

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
