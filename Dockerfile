# docker build -t cortexm .
FROM ubuntu:16.04
MAINTAINER adrianlshaw
RUN apt-get update -qq && apt-get install -y wget libx11-6
RUN wget https://github.com/gnu-mcu-eclipse/qemu/releases/download/gae-2.8.0-20170301/gnuarmeclipse-qemu-debian64-2.8.0-201703022210-head.tgz
RUN tar xvf gnuarmeclipse-qemu-debian64-2.8.0-201703022210-head.tgz
RUN cp gnuarmeclipse/qemu/2.8.0-201703022210-head/bin/* /usr/local/bin
ENTRYPOINT ["/usr/local/bin/qemu-system-gnuarmeclipse"]
CMD [ "-board help" ]

