# 2021_k8s

## ~/c_script  : container related scripts
 - add to $PATH in .bashrc
 - export REGISTRY_INFO="192.168.70.143:5000"
----------------------------------------
 - jq : simple json parser
 - build_container : create container & push
 - get_repo	: get image list
 - get_tags	: get image tags
 - label_check : check image labels
 - get_digest : check image digest (for del_image)
 - del_image : delete image

## run docker cli at non-root account prompt
----------------------------------------
```
groupadd -f docker
usermod -aG docker `user`
chown root:docker /var/run/docker.sock
```
