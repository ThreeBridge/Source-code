/*---画像データをUDPデータとして送信する---*/
/*---UDPClient_image_color.cpp  ---*/

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
#include <opencv2/highgui.hpp>
#include <unistd.h>      // usleep() <--2018.10.30
#include <time.h>        //
#include <pthread.h>

/*---define---*/
#define ECHOMAX  640*480*3+1    // エコー文字列の最大値
#define IMG_SIZE 640*480*3+1    // 画像のサイズ

void DieWithError(char *errorMessage);
void Show_Time(struct timespec, struct timespec, struct timespec, struct timespec);
void* Recv(void* argc);

/*---グローバル変数---*/
int sock;
struct sockaddr_in fromAddr;       // エコー送信元のアドレス
char echoBuffer[ECHOMAX+1];   // エコー文字列の受信用バッファ

/*---main関数---*/
int main(int argc, char *argv[]){
  //int sock;                          // ソケット
  struct sockaddr_in echoServAddr[2];   // エコーサーバのアドレス
  
  unsigned short echoServPort;       // エコーサーバのポート番号
  //unsigned int fromSize;             // recvfrom()のアドレスの入出力サイズ
  char *servIP0;                      // サーバのIPアドレス
  char *servIP1;
  int waitsec = 0.0;                // FPGA側の改善により遅延なしで送信可
  int num_of_tx = 640;
  
  int echoStringLen;                 // 文字列の長さ
  std::vector<int> param = std::vector<int>(2);
  int i,j;
  int high,width,datasize;

  struct timespec startTime_r, endTime_r, startTime_c, endTime_c;

  //unsigned char *imageBuffer;

  if((argc < 2)||(argc > 3)){
    fprintf(stderr,"Usage: %s <Server IP0> [<Echo Port>]\n",argv[0]);
    exit(1);
  }
  
  servIP0 = argv[1];      // 1つ目の引数 : サーバのIPアドレス(ドット10進表記)

  if(argc == 3)
    echoServPort = atoi(argv[2]);  // 指定のポート番号があれば使用
  else
    echoServPort = 7;  // 7はエコーサービスのwell-knownポート番号

  /*---画像の取り込み---*/
  cv::Mat image = cv::imread("/home/tmitsuhashi/bin/opencv/color640x480.bmp",cv::IMREAD_COLOR);
  if(image.empty()){
    std::cout << "read error.\n";
    return -1;
  }
  /*---画像の高さ,幅を表示---*/
  high = image.rows;
  width = image.cols;
  datasize = high * width;

  /*---DEBUG---*/
  /*
  printf("\n");
  for (i=0;i<99;i=i+3){
      printf("B:%d,G:%d,R:%d\n",image.data[i],image.data[i+1],image.data[i+2]);
  }
  printf("\n");
  return 0;
  */

  /*---UDPデータグラムソケットの作成---*/
  if((sock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  /*---サーバのアドレス構造体の作成---*/
  memset(&echoServAddr[0], 0, sizeof(echoServAddr[0]));
  echoServAddr[0].sin_family = AF_INET;                 // インターネットアドレスファミリ
  echoServAddr[0].sin_addr.s_addr = inet_addr(servIP0); // サーバのIPアドレス
  echoServAddr[0].sin_port = htons(echoServPort);       // サーバのポート番号

  /*---画像データをサーバに送信---*/
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
    if(sendto(sock, &image.data[i*480*3], 480*3, 0, (struct sockaddr *)&echoServAddr[0],
	      sizeof(echoServAddr[0])) != 480*3){
	        DieWithError("sendto() sent a different number of bytes than expected");
    }
    //printf("%d  ",i);
    //usleep(waitsec);
  }
  pthread_join(thread,NULL);

  clock_gettime(CLOCK_MONOTONIC_RAW, &endTime_r);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime_c);
  Show_Time(startTime_r, endTime_r, startTime_c, endTime_c);

  /*---
  受信データの処理----*/
  cv::Mat recv_image;
  recv_image.create(480,640,CV_8UC3);
  int count;
  int c;  // チャンネル数
  count = 0;
  for(j=0;j<480;j++){
    for(i=0;i<640;i++){
      for(c=0;c<3;c++){
        recv_image.data[count+c] = echoBuffer[count+c];
      }
      count += 3;
    }
  }
  param[0]=cv::IMWRITE_PXM_BINARY;
  param[1]=1;
  cv::imwrite("/home/tmitsuhashi/bin/opencv/recv_pc_color.bmp",recv_image/*,param*/);

  /* test */
  /*
  cv::Mat test_image;
  test_image.create(480,640,CV_8UC3);
  for(j=0;j<480;j++){
    for(i=0;i<640;i++){
      test_image.data[count] = 255;
      count++;
    }
  }
  cv::imwrite("/home/tmitsuhashi/bin/opencv/testcolor.bmp",test_image);
  */
  close(sock);
  
  /*---取得画像表示---*/
  //cv::namedWindow("FPGA2PC",cv::WINDOW_AUTOSIZE);
  //cv::imshow("FPGA2PC",recv_image);
  // cv::waitKey(0);
  //cv::destroyWindow("FPGA2PC");

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
  int recv_cnt=640;

  fromSize = sizeof(fromAddr);

  for(i=0;i<recv_cnt;i++){
    if((respStringLen = recvfrom(sock, echoBuffer+i*480*3, 480*3, 0,
	     (struct sockaddr *)&fromAddr, &fromSize)) != 480*3){
          printf("パケット数%d\n",respStringLen);
          DieWithError("recvfrom() failed");
       }
  }

}
