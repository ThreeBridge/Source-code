#vsPC.sh
#PCとの通信を1000回繰り返し
#その結果をファイルに出力する.

echo "<--start-->" >> vsPC_800x400
for ((i=0;i<100;i++))
do
    ./UDPClient_image_vsPC 172.31.210.128 60000 >> vsPC_800x400
    #sleep 5
done
