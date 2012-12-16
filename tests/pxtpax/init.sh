#!/bin/bash
#
#	init.sh: this file is part of the elfix package
#	Copyright (C) 2011  Anthony G. Basile
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

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
