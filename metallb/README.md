## save -n kube-system kube-proxy configmap
- kubectl describe configmap -n kube-system kube-proxy > kube-proxy.configmap

## edif kube-proxy configmap
- kubectl edit configmap -n kube-system kube-proxy
- [edit]
```
apiVersion: kubeproxy.config.k8s.io/v1alpha1
kind: KubeProxyConfiguration
mode: "ipvs"
ipvs:
  strictARP: true
```
## get manifest & apply & generate secret (firt install only)
- caution metallb/v0.9.5 not work as well (address pool exceed & annotation feature)
- use metallb/v0.9.3
```
wget https://raw.githubusercontent.com/metallb/metallb/v0.9.3/manifests/namespace.yaml
wget https://raw.githubusercontent.com/metallb/metallb/v0.9.3/manifests/metallb.yaml
kubectl apply -f ./namespace.yaml
kubectl apply -f ./metallb.yaml

kubectl create secret generic -n metallb-system memberlist --from-literal=secretkey="$(openssl rand -base64 128)"
```
## check status
```
kubectl get deployment -n metallb-system
kubectl get pods -n metallb-system 
```
