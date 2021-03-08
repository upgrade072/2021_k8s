# 2021_k8s

----------------------------------------
## ~/c_script  : container related scripts
```
 - add to $PATH in .bashrc
 - export REGISTRY_INFO="192.168.70.143:5000"
```
```
 - jq : simple json parser
 - bc : create container & push
 - get_repo : get image list
 - get_tags : get image tags
 - label_check : check image labels
 - get_digest : check image digest (for del_image)
 - del_image : delete image
 ```

----------------------------------------
## run docker cli at non-root account prompt
```
groupadd -f docker
usermod -aG docker `user`
chown root:docker /var/run/docker.sock
```

----------------------------------------
## topo daemon test data
```
--- server

export EMS_TOPO_ADDR=192.168.70.143:8000
export MY_SYS_NAME=OMP

--- client

export EMS_TOPO_ADDR=192.168.70.143:8000
export MY_NODE_NAME=k8s-vm.5
export MY_POD_NAME=k8s-test-downward-0
export MY_POD_NAMESPACE=default
export MY_POD_IP=10.0.5.34
export MY_POD_SERVICE_ACCOUNT=default
export MY_NODE_IP=192.168.70.82
```
