# cf-ddns

A small DDNS (Dynamic DNS) client written in C. It keeps a Cloudflare **A record**
pointing at your current public IPv4 address, so a machine on a connection with a
changing IP stays reachable through a stable domain name.

It uses [libcurl](https://curl.se/libcurl/) for HTTP and
[cJSON](https://github.com/DaveGamble/cJSON) (vendored) for JSON parsing.

## How it works

On each run the program:

1. Reads its settings from `config.txt`.
2. Fetches the machine's current public IPv4 from `https://api.ipify.org`.
3. Reads the IP currently stored in the Cloudflare DNS record.
4. Compares the two:
   - if they are equal, it does nothing;
   - if they differ, it sends a `PATCH` to the Cloudflare API to update the record.

It is designed to be run periodically (e.g. every minute via a systemd timer),
not as a long-running daemon.

## Project structure

```
cf-ddns/
├── src/                 # application code
│   ├── main.c           # program flow
│   ├── config.c/.h      # reads config.txt into a struct
│   ├── http.c/.h        # libcurl write callback + response buffer
│   ├── ipv4.c/.h        # fetches the public IP (ipify)
│   ├── cloudflare.c/.h  # reads and updates the DNS record
│   ├── json.c/.h        # parses the Cloudflare response (cJSON)
│   └── error.h          # shared error codes (ddns_error enum)
├── vendor/cjson/        # third-party cJSON library (vendored)
├── systemd/             # service + timer units for production
├── Makefile
├── config.txt           # your settings (gitignored — create it yourself)
└── README.md
```

## Requirements

- A C compiler (`gcc`), `make`, and `pkg-config`
- libcurl development headers

On Arch Linux:

```bash
sudo pacman -S --needed base-devel curl
```

On Debian/Ubuntu:

```bash
sudo apt update && sudo apt install -y build-essential libcurl4-openssl-dev
```

## Building

```bash
make          # compiles everything into ./cf-ddns
make run      # builds and runs it
make clean    # removes the ./cf-ddns binary
```

The build compiles all sources in one step; there are no intermediate object
files. The resulting `./cf-ddns` is a single self-contained executable.

## Configuration

Create a `config.txt` file **in the same directory the program runs from**. The
program reads three whitespace-separated values, one per line, in this exact order:

```
YOUR_CLOUDFLARE_API_TOKEN
YOUR_ZONE_ID
YOUR_RECORD_ID
```

| Value         | Where to find it |
|---------------|------------------|
| `api_token`   | Cloudflare dashboard → My Profile → API Tokens (needs DNS edit permission on the zone) |
| `zone_id`     | Cloudflare dashboard → your domain → Overview (right sidebar) |
| `record_id`   | Cloudflare API: `GET /zones/{zone_id}/dns_records` |

`config.txt` is listed in `.gitignore` — it holds a secret token and must never
be committed.

> **Note:** the config is read by relative path (`config.txt`), so the program
> must be started with its working directory set to the folder that contains the
> file. The systemd unit below handles this with `WorkingDirectory`.

## Running manually

```bash
./cf-ddns
```

Example output:

```
CURRENT IP: 203.0.113.10
RECORD IP: 203.0.113.4
IPs addresses are not equal!
Record ip address updated!
All done!
```

### Exit codes

The process exit code reflects what happened, which is handy in scripts and logs:

| Code | Meaning |
|------|---------|
| `0`  | success |
| `1`  | could not read `config.txt` |
| `2`  | network failure on a request |
| `3`  | Cloudflare returned an error (check token / zone / record) |
| `4`  | unexpected response (invalid JSON or missing field) |
| `99` | unknown/unclassified failure |

## Deploying to production (systemd)

The `systemd/` folder ships a **oneshot service** (does the work and exits) plus a
**timer** that runs it on a schedule. This example targets an Ubuntu server where
the project lives at `/home/ubuntu/cf-ddns` and runs as the `ubuntu` user — adjust
`User`, `WorkingDirectory`, and `ExecStart` in `systemd/cf-ddns.service` if your
paths differ.

### 1. Get the code and build it on the server

```bash
git clone https://github.com/jsantos43/cf-ddns.git /home/ubuntu/cf-ddns
cd /home/ubuntu/cf-ddns
sudo apt update && sudo apt install -y build-essential libcurl4-openssl-dev
make
```

> The binary and `config.txt` are gitignored, so they are **not** part of the
> clone. Always build on the target machine and create `config.txt` there.

### 2. Create the config

```bash
nano /home/ubuntu/cf-ddns/config.txt   # fill in the three values described above
```

### 3. Install and enable the timer

```bash
sudo cp systemd/cf-ddns.service systemd/cf-ddns.timer /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now cf-ddns.timer
```

Enable the **timer**, not the service — the timer is what triggers the service on
schedule. By default it runs one minute after boot and then every minute; change
`OnUnitActiveSec` in `systemd/cf-ddns.timer` to adjust the interval.

### 4. Verify

```bash
systemctl list-timers cf-ddns.timer      # next/last run
sudo systemctl start cf-ddns.service     # run once now, on demand
journalctl -u cf-ddns.service -n 20      # view output and errors
```

## License

Released under the [MIT License](LICENSE).

This project vendors [cJSON](https://github.com/DaveGamble/cJSON), which is also
MIT licensed; its copyright notice is retained in `vendor/cjson/`.
