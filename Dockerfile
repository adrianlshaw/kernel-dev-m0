# docker build -t cortexm .
# docker run -ti -v $PWD:/opt --cap-add=SYS_PTRACE cortexm
FROM ubuntu:17.10
RUN apt-get update && apt-get install -y gdb-arm-none-eabi gcc-arm-none-eabi tmux make qemu-system-arm wget python-pygments
RUN wget https://raw.githubusercontent.com/cyrus-and/gdb-dashboard/master/.gdbinit -P ~
WORKDIR /opt
COPY ./ /opt/
ENV CC arm-none-eabi-gcc
ENV CROSS_COMPILE arm-none-eabi-
ENTRYPOINT [ "/bin/bash", "-c", "make && timeout --preserve-status 4 qemu-system-arm -d guest_errors -M lm3s811evb -m 8K -nographic -kernel main.elf > out 2> err || true && grep handled out > /dev/null" ]
