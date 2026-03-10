# Photon Proxy

High-performance async HTTPS proxy for Photon firmware devices using FastAPI.

## Features

✅ **Async/await** - Non-blocking I/O for better concurrency
✅ **Request deduplication** - Multiple requests for same ticker share one API call
✅ **Multi-instance** - Easy horizontal scaling with systemd templates
✅ **Load balancing** - nginx configuration included
✅ **Rate limiting** - Per-IP rate limits with async safety
✅ **Currency conversion** - FX rate caching with yfinance

## Files

```
proxy/
├── https_proxy.py                  # Main FastAPI proxy (async)
├── const.py                        # Configuration
├── requirements.txt                # Python dependencies
├── photon-proxy@.service           # Systemd template for multi-instance
├── nginx-photon-lb.conf            # nginx load balancer config
├── deploy-multi-instance.sh        # Deployment script
├── https_proxy_legacy.py           # Old blocking version (backup)
└── MULTI_INSTANCE_SETUP.md         # Detailed setup guide
```

## Quick Start

### Single Instance (Testing)

```bash
# Install dependencies
pip3 install -r requirements.txt

# Run locally
python3 https_proxy.py --port 8080

# Test it
curl http://localhost:8080/health
curl http://localhost:8080/stocks/AAPL
```

### Multi-Instance Production (Digital Ocean VM)

```bash
# Deploy 3 instances with load balancing
./deploy-multi-instance.sh
```

This will:
- Start 3 instances on ports 8080, 8081, 8082
- Configure nginx load balancer
- Enable auto-start on boot

## Service Management

### Multi-Instance Commands

```bash
# View all instances
sudo systemctl status 'photon-proxy@*'

# Start all instances
sudo systemctl start 'photon-proxy@*'

# Stop all instances
sudo systemctl stop 'photon-proxy@*'

# Restart all instances
sudo systemctl restart 'photon-proxy@*'

# View logs from all instances
sudo journalctl -u 'photon-proxy@*' -f
```

### Single Instance Commands

```bash
# Manage specific instance (0, 1, or 2)
sudo systemctl restart photon-proxy@0
sudo systemctl status photon-proxy@0
sudo journalctl -u photon-proxy@0 -f
```

## Monitoring Logs

```bash
# View live logs from all instances
sudo journalctl -u 'photon-proxy@*' -f

# View last 50 lines from instance 0
sudo journalctl -u photon-proxy@0 -n 50

# View logs since last hour
sudo journalctl -u 'photon-proxy@*' --since "1 hour ago"

# View errors only
sudo journalctl -u 'photon-proxy@*' -p err

# Watch for deduplication
sudo journalctl -u 'photon-proxy@*' -f | grep -i deduplicate

# Clear old logs (keep last 2 days)
sudo journalctl --vacuum-time=2d
```

## Testing

### Health Check

```bash
curl http://localhost:8080/health
```

### Stock Endpoints

```bash
# Basic stock quote
curl http://localhost:8080/stocks/AAPL

# With currency conversion
curl 'http://localhost:8080/stocks/VUAG?currency=USD'

# LSE ticker (auto-converted to .L suffix)
curl http://localhost:8080/stocks/VWRL
```

### Via nginx Load Balancer

```bash
# Requests automatically distributed across instances
curl http://localhost/stocks/AAPL
curl http://localhost/health
```

### API Documentation

Built-in Swagger UI available at:
```
http://YOUR_VM_IP/docs
```

## Configuration

Edit [const.py](const.py):

```python
# Server
PROXY_PORT = 8080
BIND_ADDRESS = '0.0.0.0'

# Rate limiting
RATE_LIMIT_ENABLED = True
RATE_LIMIT_REQUESTS = 100  # requests per window
RATE_LIMIT_WINDOW = 60     # seconds

# Allowed hosts (whitelist)
ALLOWED_HOSTS = [
    'api.kraken.com',
    'api.open-meteo.com',
]

# Logging
LOG_LEVEL = 'INFO'
LOG_FILE = '/var/log/photon-proxy.log'
```

## Scaling

### How Many Instances?

| Devices | Instances | VM Size |
|---------|-----------|---------|
| 1-10 | 1-2 | 2GB RAM |
| 10-50 | 2-3 | 4GB RAM |
| 50-100 | 3-5 | 4-8GB RAM |
| 100+ | 5+ | 8GB+ RAM |

FastAPI's async nature means you need **fewer instances** than traditional blocking servers.

### Add More Instances

Edit [deploy-multi-instance.sh](deploy-multi-instance.sh):

```bash
NUM_INSTANCES=5  # Increase this
```

Update [nginx-photon-lb.conf](nginx-photon-lb.conf):

```nginx
upstream photon_backend {
    server 127.0.0.1:8080;
    server 127.0.0.1:8081;
    server 127.0.0.1:8082;
    server 127.0.0.1:8083;  # Add new instances
    server 127.0.0.1:8084;
}
```

Then redeploy:

```bash
./deploy-multi-instance.sh
sudo nginx -t && sudo systemctl reload nginx
```

## Performance

### Request Deduplication

Multiple concurrent requests for the same ticker share one API call:

```bash
# 10 concurrent requests = 1 yfinance call
for i in {1..10}; do curl -s http://localhost:8080/stocks/AAPL & done; wait
```

Logs will show:
```
INFO: Stock AAPL for 192.168.1.10: current=150.0
INFO: Deduplicating request for AAPL:None
INFO: Deduplicating request for AAPL:None
...
```

### Load Testing

```bash
# Install Apache Bench
sudo apt install apache2-utils -y

# Test throughput
ab -n 1000 -c 50 http://localhost:8080/stocks/AAPL
```

## Troubleshooting

### Instance won't start

```bash
sudo systemctl status photon-proxy@0
sudo journalctl -u photon-proxy@0 -n 50
```

### Check if ports are available

```bash
sudo lsof -i :8080
sudo netstat -tlnp | grep 808
```

### Test yfinance directly

```bash
python3 -c "import yfinance; print(yfinance.Ticker('AAPL').fast_info.last_price)"
```

### nginx errors

```bash
sudo nginx -t
sudo tail -f /var/log/nginx/error.log
```

## See Also

- [MULTI_INSTANCE_SETUP.md](MULTI_INSTANCE_SETUP.md) - Detailed deployment guide
- `/docs` endpoint - Interactive API documentation (when proxy is running)
