import sys
sys.path.append('..')

import MinerClient
import asyncio

use_ssl = True
keys_dir = ""
host = "cscoins.2017.csgames.org"

if len(sys.argv) > 1:
    keys_dir = sys.argv[1]

if len(sys.argv) > 2:
    host = sys.argv[2]

if len(sys.argv) > 3:
    use_ssl = sys.argv[3] != "0"

mc = MinerClient.MinerClient(keys_dir, host)
if not use_ssl: mc.ssl = False
asyncio.get_event_loop().run_until_complete(mc.client_loop())
