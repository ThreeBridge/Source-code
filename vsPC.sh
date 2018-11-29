#vsPC.sh
#PCとの通信を100回繰り返し
#その結果をファイルに出力する.

echo "start." >> test2
for ((i=0;i<100;i++))
do
    ./UDPClient_image 172.31.210.128 60000 >> test2
    #sleep 5
done
