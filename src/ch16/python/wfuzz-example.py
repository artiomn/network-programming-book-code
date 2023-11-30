#!/usr/bin/env python3

import wfuzz


URL = 'http://testphp.vulnweb.com/listproducts.php?cat=FUZZ'

for r in wfuzz.get_payload(range(100)).fuzz(hl=[97], url=URL):
    print(r)
