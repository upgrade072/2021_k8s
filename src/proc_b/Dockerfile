FROM scratch
LABEL desc="---test proc_a keepalive [NOT] increase---"
LABEL require1=""
LABEL version_info="2021-03-17"
ADD ./lib64 /lib64
ADD ./proc_b /proc_b
WORKDIR /
CMD ["/proc_b"]
