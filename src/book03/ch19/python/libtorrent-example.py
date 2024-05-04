#!/usr/bin/env python3

import sys
import time
import libtorrent as lt


ses = lt.session({'listen_interfaces': '0.0.0.0:6881'})
info = lt.torrent_info(sys.argv[1])

h = ses.add_torrent({'ti': info, 'save_path': '.'})
s = h.status()
print('starting', s.name)

while not s.is_seeding:
    s = h.status()

    print(
        f'\r{s.progress * 100:.2f} complete '
        f'(down: {s.download_rate / 1000:.1f} kB/s '
        f'up: {s.upload_rate / 1000:.1f} kB/s peers: {s.num_peers}) '
        f'{s.state}',
        end=' ',
    )

    alerts = ses.pop_alerts()

    for a in alerts:
        if a.category() & lt.alert.category_t.error_notification:
            print(a)

    sys.stdout.flush()

    time.sleep(1)

print(h.status().name, 'complete')
