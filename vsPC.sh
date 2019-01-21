#vsPC.sh
#PCとの通信を100回繰り返し
#その結果をファイルに出力する.

echo "<--start-->" >> vsPC
for ((i=0;i<100;i++))
do
    ./UDPClient_image_vsPC 172.31.210.128 60000 >> vsPC
    #sleep 5
done
