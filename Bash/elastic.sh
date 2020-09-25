#!/usr/bin/env bash
# Copyright (C) Yuyang Chen (Wine93)

###########################  GLOBAL VARIABLES
os=`uname -s`
version="7.8.1"
tarball="elasticsearch-${version}-${os,,}-x86_64.tar.gz"
url="http://c.nmc.nasa.org:9301/downloads/$tarball"
app="app/elasticsearch/elasticsearch-${version}"

###########################  BASE FUNCTIONS
cwd()
{
    cd `dirname $0`
}

create_dir()
{
    local dirs=('download' 'app/elasticsearch' 'pid' 'logs' 'data')
    for dir in ${dirs[*]}
    do
        mkdir -p $dir
    done
}

download_bin()
{
    local tarball="download/$tarball"
    if [ ! -f "$tarball" ]; then
        wget $url -O $tarball
        tar zxvf $tarball -C app/elasticsearch
    fi
}

set_env()
{
    declare -A env=()

    env["ES_PATH_CONF"]="config"
    env["ES_PATH_DATA"]="data"
    env["ES_PATH_LOGS"]="logs"
    env["ES_NODE_NAME"]=$HOSTNAME
    env["ES_NETWORK_HOST"]=$HOSTNAME
    env["ES_SEED_HOST"]=$HOSTNAME

    for key in ${!env[@]}
    do
        local value=${env[${key}]}
        local command="export \"$key=$value\""
        echo $command && eval "$command"
    done
}

############################  PHASE FUNCTIONS
get_options()
{
    return
}

init()
{
    cwd
    create_dir
    download_bin
    set_env
}

run()
{
    exec ${app}/bin/elasticsearch --pidfile "pid/elasticsearch.pid"
}

main() {
    get_options "$@"
    init
    run
}

############################  MAIN
main "$@"
