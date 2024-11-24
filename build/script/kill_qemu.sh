pid=`lsof -i:1234 | grep "qemu-syst" | grep "IPv4" | awk '{print $2}'`
if [ ! -z "${pid}" ]; then
    kill -9 ${pid}
fi