#!/bin/bash

PATH=/sbin:/bin

PWD=$(pwd)
DAEMON="${PWD}"/daemon
PIDFILE="${PWD}"/daemon.pid

start()
{
	rm -f ${PIDFILE}
	if [ -e ${DAEMON} ]
	then
		PID=$(${DAEMON})
		if [ "x${PID}" != "x" ]
		then
			echo $PID  >> ${PIDFILE}
		fi
	else
		echo "No daemon"
	fi
}

stop()
{
	kill $(cat ${PIDFILE})
	rm -f ${PIDFILE}
}

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	*)
		echo "Usage: $0 start | stop"
		;;
esac
