#!/bin/bash
PROGRAM=$0
PRJ_DIR=$(dirname $(readlink -f "$0"))
JAVA_TAILOR_PROBES=$EXTEND_PROBES

INSTALL_FILES=""

while getopts ":b:c:" opt
do
    case $opt in
        b) INSTALL_PATH=$OPTARG;;
        c) CONF_PATH=$OPTARG;;
        ?) echo "unknow param"; exit 1;;
    esac
done

cd ${PRJ_DIR}
# tailor jvmprobe
if ! [[ $JAVA_TAILOR_PROBES =~ "jvm.probe" ]] ; then
    INSTALL_FILES+=" jvm.probe/src/jvmprobe"
    INSTALL_FILES+=" jvm.probe/JvmProbeAgent.jar"
fi

if [ ${INSTALL_PATH} ]; then
    mkdir -p ${INSTALL_PATH}
    for file in ${INSTALL_FILES}; do
        cp ${file} ${INSTALL_PATH}
    done
fi

