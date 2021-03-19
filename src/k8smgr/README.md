## k8s pod lifecycle
```
livenessProbe : container alive
readinessProbe : container ready to service
startupProbe : application now started, start check probes
```

## /k8proc/
```
/* default get cmd */
curl 0.0.0.0:32768/start			: get pod started
curl 0.0.0.0:32768/ready			: get pod ready to service
curl 0.0.0.0:32768/live/[proc_name] : get pod(container) alive
curl 0.0.0.0:32768/print/status		: print all status
/* additional set cmd */
curl -i --request PATCH 0.0.0.0:32768/live/PROC_A?status=0
curl -i --request PATCH 0.0.0.0:32768/live/PROC_A?status=1
curl -i --request PATCH 0.0.0.0:32768/ready?status=0
curl -i --request PATCH 0.0.0.0:32768/ready?status=1
```
