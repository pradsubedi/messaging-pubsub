mpirun -n 2 server &
sleep 5
mpirun -n 2 client
kill $!