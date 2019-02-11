FROM ubuntu:14.04
RUN apt-get update
RUN apt-get install -y software-properties-common
RUN apt-add-repository 'http://downloads.makerbot.com/makerware/ubuntu'
COPY dev@makerbot.com.gpg.key /
RUN apt-key add dev@makerbot.com.gpg.key
RUN apt-get update
RUN apt-get install -y makerware
RUN apt-get install -y mesa-utils libqt5gui5 libqt5opengl5 libqt5webkit5 libyajl2 libopencv-calib3d2.4 libopencv-highgui2.4
COPY run-conveyor-and-makerware /bin/
RUN ln /dev/null /dev/raw1394
CMD run-conveyor-and-makerware
