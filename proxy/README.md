# Photon Proxy

## Service Management

```bash
# Restart the proxy
sudo systemctl restart photon-proxy

# Check status
sudo systemctl status photon-proxy

# Stop the proxy
sudo systemctl stop photon-proxy

# Start the proxy
sudo systemctl start photon-proxy

# Enable auto-start on boot
sudo systemctl enable photon-proxy

# Disable auto-start
sudo systemctl disable photon-proxy
```

## Monitoring Logs

```bash
# View live logs (follow mode)
sudo journalctl -u photon-proxy -f

# View last 50 lines
sudo journalctl -u photon-proxy -n 50

# View logs since last boot
sudo journalctl -u photon-proxy -b

# View logs from last hour
sudo journalctl -u photon-proxy --since "1 hour ago"

# View errors only
sudo journalctl -u photon-proxy -p err

# Clear old logs (keep last 2 days)
sudo journalctl --vacuum-time=2d
```

## Testing Tickers

```bash
curl http://your-proxy-ip:8080/stocks/ticker
```
