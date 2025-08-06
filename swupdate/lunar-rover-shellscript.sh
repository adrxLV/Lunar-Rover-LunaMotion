#!/bin/sh

do_preinst()
{
    echo "do_preinst"
    /etc/init.d/S91lunarrovernav stop
    exit 0
}

do_postinst()
{
    echo "do_postinst"
    /etc/init.d/S91lunarrovernav start
    exit 0
}

echo $0 $1 > /dev/ttyO0

case "$1" in
preinst)
    echo "call do_preinst"
    do_preinst
    ;;
postinst)
    echo "call do_postinst"
    do_postinst
    ;;
*)
    echo "default"
    exit 1
    ;;
esac
