#!/bin/sh
# gdbserver --once 0.0.0.0:1234 src/live-streamer -c/etc/config/live-streamer.conf \
# --pipe "videv0:sensor=imx327,online=2,\|vichn0-0\|isp0\|vpgrp0\|vpchn0-0" \
# --pipe "vpchn0-0\|vechn0:encoding=H265,br=2048" \
# --pipe "vpchn0-0\|vechn1:encoding=H265,br=512,res=640x480" \
# -vsrc vpchn0-0 \
# -venc vechn0 \
# -venc vechn1 \
# --pipe "acodec0:invol=56,outvol=0\|aidev0\|aichn0-0" \
# --pipe "aichn0-0\|aechn0:encoding=AAC,br=16" \
# --pipe "aichn0-0\|aechn1:encoding=G711A" \
# -asrc aichn0-0 \
# -aenc aechn0 \
# -aenc aechn1 \
# -stream 0:vechn0,aechn0 \
# -stream 1:vechn1,aechn0 

src/live-streamer -c/etc/config/live-streamer.conf \
--pipe "videv0:sensor=imx327,online=2,\|vichn0-0\|isp0\|vpgrp0\|vpchn0-0" \
--pipe "vpchn0-0|vechn0:vbcnt=2,resolution=1920x1080@25,encoding=H264,br=4096" \
--pipe "vpchn0-0|vechn1:vbcnt=2,resolution=1280x720@25,encoding=H264,br=2048" \
--pipe "vpchn0-0|vechn2:vbcnt=2,resolution=640x360@10,encoding=H264,br=128" \
--pipe "ircut:sensor=gpio66,ircutp=gpio41,ircutn=gpio40,irleden=53" \
--pipe "acodec0:samplerate=44100|aidev0|aichn0-0:enable" \
--pipe "aichn0-0|aechn0:encoding=AAC,bitrate=64" \
-vsrc vpchn0-0 \
-venc vechn0 \
-venc vechn1 \
-venc vechn2 \
-asrc aichn0-0 \
-aenc aechn0 \
-stream "0:vechn0,aechn0" \
-stream "1:vechn1,aechn0" \
-stream "2:vechn2,aechn0"