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

/*---define---*/
#define ECHOMAX 65000    // エコー文字列の最大値

void DieWithError(char *errorMessage);
void Show_Time(struct timespec, struct timespec, struct timespec, struct timespec);

int main(int argc, char *argv[]){
  int sock;                          // ソケット
  struct sockaddr_in echoServAddr;   // エコーサーバのアドレス
  struct sockaddr_in fromAddr;       // エコー送信元のアドレス
  unsigned short echoServPort;       // エコーサーバのポート番号
  unsigned int fromSize;             // recvfrom()のアドレスの入出力サイズ
  char *servIP;                      // サーバのIPアドレス
  int waitsec = 15.0;                // =15.0us=0.001ms
  
  //std::vector<unsigned char> i_imageBuffer;             // エコーサーバへ送信する画像データ
  char echoBuffer[ECHOMAX+1];   // エコー文字列の受信用バッファ
  int echoStringLen;                 // 文字列の長さ
  int respStringLen;                 // 受信した応答の長さ
  std::vector<int> param = std::vector<int>(2);
  int i,j;
  int high,width,datasize;

  struct timespec startTime_r, endTime_r, startTime_c, endTime_c;

  //unsigned char *imageBuffer;

  if((argc < 2)||(argc > 3)){
    fprintf(stderr,"Usage: %s <Server IP> [<Echo Port>]\n",argv[0]);
    exit(1);
  }
  
  servIP = argv[1];      // 1つ目の引数 : サーバのIPアドレス(ドット10進表記)
  //2018.11.29(del(0))
  //printf("宛先IPアドレス : %s\n",argv[1]);

  if(argc == 3)
    echoServPort = atoi(argv[2]);  // 指定のポート番号があれば使用
  else
    echoServPort = 7;  // 7はエコーサービスのwell-knownポート番号
  //del(0)
  //printf("ポート番号     : %s\n",argv[2]);

  /*---画像の取り込み---*/
  cv::Mat image = cv::imread("/home/tmitsuhashi/bin/opencv/kochi2.bmp",cv::IMREAD_GRAYSCALE);
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
  memset(&echoServAddr, 0, sizeof(echoServAddr));
  echoServAddr.sin_family = AF_INET;                // インターネットアドレスファミリ
  echoServAddr.sin_addr.s_addr = inet_addr(servIP); // サーバのIPアドレス
  echoServAddr.sin_port = htons(echoServPort);      // サーバのポート番号

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
  clock_gettime(CLOCK_MONOTONIC_RAW, &startTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime_c);
  //usleep(1000);
  /*del(0)
  clock_gettime(CLOCK_MONOTONIC_RAW, &endTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime_c);
  Show_Time(startTime_r, endTime_r, startTime_c, endTime_c);
  */

  for(i=0;i<10;i++){
    if(sendto(sock, &image.data[i*1000], 1000, 0, (struct sockaddr *)&echoServAddr,
	      sizeof(echoServAddr)) != 1000)
	DieWithError("sendto() sent a different number of bytes than expected");
    usleep(waitsec);
  }
  //free(imageBuffer);

  //  printf("debug\n");

  /*---応答を受信---*/
  
  for(i=0;i<10000;i++){
    echoBuffer[i] = 255;
  }

  fromSize = sizeof(fromAddr);
  //printf("%d\n",fromSize);
  for(i=0;i<10;i++){
    if((respStringLen = recvfrom(sock, echoBuffer+i*1000, 1000, 0,
	     (struct sockaddr *)&fromAddr, &fromSize)) != 1000)
      DieWithError("recvfrom() failed");
    //del(0)
    //printf("%d回目\n",i+1);
  }
  if(echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
    fprintf(stderr,"Error : received a packet from unknown source.\n");
    exit(1);
  }
  //printf("debug\n");

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
  recv_image.create(100,100,CV_8UC1);
  int count = 0;
  count = 0;
  for(j=0;j<100;j++){
    for(i=0;i<100;i++){
      recv_image.data[count] = echoBuffer[count];
      count++;
    }
  }
  param[0]=cv::IMWRITE_PXM_BINARY;
  param[1]=1;
  cv::imwrite("/home/tmitsuhashi/bin/opencv/recv_fpga.bmp",recv_image,param);

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
