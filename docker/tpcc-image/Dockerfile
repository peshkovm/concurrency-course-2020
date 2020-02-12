# Bionic Beaver
FROM ubuntu:18.04

COPY install_deps.sh /

RUN sh install_deps.sh

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

RUN echo "StrictHostKeyChecking=no" >> /etc/ssh/ssh_config
EXPOSE 22

RUN groupadd -g 712342 clion_user_group
RUN useradd -u 712342 -g 712342 -m user && yes password | passwd user

RUN mkdir /var/run/sshd

ENV CXX="/usr/bin/clang++-8"
ENV CC="/usr/bin/clang-8"

CMD ["/usr/sbin/sshd", "-D"]
