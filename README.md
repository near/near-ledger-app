# NEAR Ledger App Docker

Based on https://github.com/ZondaX/ledger-docker-bolos (MIT license) and https://github.com/LedgerHQ/ledger-app-waves (MIT license).

## Requirements

* Docker installed
* Python 2.7 installed
* A Ledger Nano

## Steps to get this to work

1. Plug in the Ledger and set up following the on-screen instructions.
2. Copy the Ledger Nano SDK into `workdir/` (you can find it here: https://github.com/LedgerHQ/nanos-secure-sdk)
3. Link `BOLOS_SDK` env variable to the folder that you just copied. For instance, if this repo is in your $HOME dir, you would use:
  
  ```bash
  export BOLOS_SDK=$HOME/near-ledger-app/workdir/nanos-secure-sdk
  ```

4. Run `sh build.sh`. You may have to log into docker.
5. `cd` into `workdir/near-ledger-app`
6. Run `pip install -r python2.7-dependencies.txt` (If you see errors, you may want to use `virtualenv`*)
7. run `make load-only` with the ledger app plugged in. If you get an error, you may need to also set the following env var:
  
  ```bash
  export SCP_PRIVKEY=""
  ```

  And then run the make command again.

### *Using virtual env

**you must be using bash to use the following workflow**

1. `pip install virtualenv`
2. cd into `workdir/near-ledger-app/`
3. `virtualenv env`
4. `source env/bin/activate` You should see something like `(env)bash$` where your bash profile normally is.
5. Go back and run the pip install from step 5 above.

## Testing it out

```bash
python python/ledger-near.py
```
