import sys
sys.path.append('..')

import MinerClient
import asyncio

keys_dir = ""
wallet_name = "Sherbrooke"
host = "cscoins.2017.csgames.org"
use_ssl = True

ia = len(sys.argv)
i = 1

while i < ia:
    arg = sys.argv[i]
    if arg == '-k':
        if i + 1 < ia:
            keys_dir = sys.argv[i+1]
            i += 1
    elif arg == '-n':
        if i + 1 < ia:
            wallet_name = sys.argv[i+1]
            i += 1
    elif arg == '-h':
        if i + 1 < ia:
            host = sys.argv[i+1]
            i += 1
    elif arg == '-s':
            use_ssl = sys.argv[i+1] != '0'
            i += 1
    i += 1

while True:
    try:
        mc = MinerClient.MinerClient(keys_dir, host, wallet_name, use_ssl)
        asyncio.get_event_loop().run_until_complete(mc.client_loop())
    except:
        # can't monitor the miners during the night... Let's do it live!
        print("CRASHED! Restarting...")
        raise
