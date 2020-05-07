# NEAR Ledger App Docker

Based on https://github.com/ZondaX/ledger-docker-bolos (MIT license) and https://github.com/LedgerHQ/ledger-app-waves (MIT license).

## Requirements

* Docker installed
* Python 3 installed
* A Ledger Nano

## Steps to get this to work

1. Plug in the Ledger and set up following the on-screen instructions.
2. Copy the Ledger Nano SDK into `workdir/` (you can find it here: https://github.com/LedgerHQ/nanos-secure-sdk)
3. Link `BOLOS_SDK` env variable to the folder that you just copied. For instance, if this repo is in your $HOME dir, you would use:
  
  ```bash
  export BOLOS_SDK=$HOME/near-ledger-app/workdir/nanos-secure-sdk
  ```

4. Run `./build.sh`. You may have to log into Docker.
5. `./run.sh` – this starts shell in Docker.
6. `cd` into `workdir/near-ledger-app` (in Docker) 
7. Run `make` (in Docker).
8. `cd` into `workdir/near-ledger-app` (in local env) 
9. Create virtual environment to use python3 and install dependencies (install `pip` - install it via `brew`):

```bash
pip install virtualenv
virtualenv .env --python=python3
. .env/bin/activate
```

10. Run `pip install ledgerblue`
11. On Ledger open "Install app".
12. Run `make load-only` with the ledger app plugged in.
13. Accept the "insecure" app onto your ledger (!) by clicking through the prompts.
If you get an error, you may need to also set the following env var:
  
  ```bash
  export SCP_PRIVKEY=""
  ```

  And then run the make command again.
