FROM scratch
LABEL desc="---k8s sysmgr/samd daemon---"
LABEL require1="namespace 'ariel'/secret 'ariel-secret'"
LABEL require2="$IV_HOME/data/sysconfig for proc/shm info"
LABEL require3="listen port 32768"
LABEL require4="need env=(IV_HOME, MY_POD_NAME, MY_SERVICE_NAME)"
LABEL version_info="2021-03-29"
ADD ./lib64 /lib64
ADD ./k8smgr /k8smgr
WORKDIR /
CMD ["/k8smgr"]
