# docker build -t cortexm .
# docker run -ti -v $PWD:/opt --cap-add=SYS_PTRACE cortexm
FROM ubuntu:17.10
RUN apt-get update && apt-get install -y gdb-arm-none-eabi gcc-arm-none-eabi tmux make qemu-system-arm wget python-pygments
RUN wget https://raw.githubusercontent.com/cyrus-and/gdb-dashboard/master/.gdbinit -P ~
WORKDIR /opt
ENTRYPOINT [ "make" ]
CMD [ "test" ]
