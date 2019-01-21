#vsFPGA.sh
#FPGAとの通信を100回繰り返し
#その結果をファイルに出力する.

echo "<---start--->" >> vsFPGA_200x100

for ((i=0;i<1000;i++))
do
    ./UDPClient_image 172.31.210.160 172.31.210.161  60000 >> vsFPGA_200x100
    #echo "$i"
    #usleep 10
done 
