#!/bin/bash
cd $(dirname $0)

pkill -f teledraw_server
screen -d -m node teledraw_server.js

