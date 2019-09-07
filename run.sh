#!/bin/sh
docker run -v "$(pwd)"/workdir:/workdir -it ledger-sdk /bin/bash
