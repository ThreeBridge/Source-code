#vsFPGA.sh
#FPGAとの通信を10回繰り返し
#その結果をファイルに出力する.

for ((i=1;i<101;i++))
do
    ./UDPClient_image 172.31.210.130 60000 >> test0
    echo "$i"
done 
