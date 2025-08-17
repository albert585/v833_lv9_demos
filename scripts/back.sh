echo ----kill robot----
killall robotd
killall robot_run_1
killall lvglsim
sleep 1
echo ----run demo----
chmod 777 lvglsim
./lvglsim
echo ----end demo----
