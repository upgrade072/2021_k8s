FROM scratch
LABEL desc="---small topology daemon---"
LABEL require="env EMS_TOPO_ADDR=192.168.70.1:8000"
LABEL version_info="2021-04-06"
ADD ./lib64 /lib64
ADD ./topo /topo
WORKDIR /
ENV IV_HOME "/"
CMD ["/topo"]
