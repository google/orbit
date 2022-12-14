FROM alpine:3.17

RUN apk add --no-cache openssh sudo socat

COPY ssh_host_rsa_key /etc/ssh/ssh_host_rsa_key
COPY ssh_host_ecdsa_key /etc/ssh/ssh_host_ecdsa_key
COPY ssh_host_ed25519_key /etc/ssh/ssh_host_ed25519_key
RUN chmod 600 /etc/ssh/*_key

EXPOSE 22

RUN adduser --disabled-password loginuser && echo 'loginuser:loginpassword' | chpasswd
COPY id_ed25519.pub /home/loginuser/.ssh/authorized_keys

# This is just a plain text file with some ASCII content that can be used by tests. Mainly needed for the SFTP tests.
COPY plain.txt /home/loginuser/plain.txt

COPY sudoers /etc/sudoers
RUN chmod 600 /etc/sudoers

CMD ["/usr/sbin/sshd", "-D", "-e", "-o", "AllowTcpForwarding=yes"]
