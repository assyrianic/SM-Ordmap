#!/bin/bash -e
cd "$(dirname "$0")"
python3 ../configure.py --mms-path "/home/kevin/alliedmodders/mmsource-1.10" --sm-path "/home/kevin/alliedmodders/sourcemod" --sdks none
ambuild
