FROM scratch
LABEL desc="---k8s sysmgr/samd daemon---"
LABEL require1="/proc dir for check container"
LABEL require2="$IV_HOME/data/sysconfig for proc/shm info"
LABEL require3="listen port 32768"
LABEL require4="need env=(MY_POD_NAME, MY_SERVICE_NAME, MY_POD_NAMESPACE)"
LABEL version_info="2021-03-19"
ADD ./lib64 /lib64
ADD ./k8smgr /k8smgr
WORKDIR /
CMD ["/k8smgr"]
