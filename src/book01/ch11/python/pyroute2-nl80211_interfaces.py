#!/usr/bin/env python3

from pyroute2.iwutil import IW


with IW() as iw:
    for q in iw.get_interfaces_dump():
        print(
            f'{q.get_attr("NL80211_ATTR_IFINDEX")} phy{int(q.get_attr("NL80211_ATTR_WIPHY"))} '
            f'{q.get_attr("NL80211_ATTR_IFNAME")} {q.get_attr("NL80211_ATTR_MAC")}'
        )
