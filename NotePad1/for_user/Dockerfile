FROM ubuntu:22.04@sha256:1f809e07a0402375e7b2ede95e4c43f5605a83c447d1a4ca9c9d3c4194440104

ENV user notepad
ENV chall_port 7882
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get -y install socat

RUN adduser $user

ADD ./flag /home/$user/flag
ADD ./$user /home/$user/$user

RUN chown -R root:root /home/$user
RUN chown root:$user /home/$user/flag
RUN chown root:$user /home/$user/$user

RUN chmod 755 /home/$user
RUN chmod 755 /home/$user/$user
RUN chmod 440 /home/$user/flag

WORKDIR /home/$user
USER $user
EXPOSE $chall_port
CMD socat -T 10 TCP-LISTEN:$chall_port,reuseaddr,fork EXEC:/home/$user/$user,pty,raw,echo=0
