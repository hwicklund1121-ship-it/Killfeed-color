"""Bounded, deduplicated audit-event counters. This module has no Discord I/O."""
from collections import OrderedDict, deque

DEFAULT_LIMITS = {
    "channel_delete": 3,
    "channel_create": 5,
    "channel_update": 6,
    "role_delete": 1,
    "permission_escalation": 1,
    "role_create": 5,
    "role_update": 3,
    "ban": 3,
    "webhook_create": 3,
}


class Protection:
    def __init__(self, window=10.0, limits=None):
        self.window = window
        self.limits = dict(DEFAULT_LIMITS if limits is None else limits)
        self.events = OrderedDict()
        self.seen = OrderedDict()

    def observe(self, event_id, actor, action, now, exempt=False):
        # Prune even when most received events are exempt or untracked.
        for key, history in list(self.events.items()):
            while history and now - history[0] > self.window:
                history.popleft()
            if not history:
                del self.events[key]
        while self.seen and now - next(iter(self.seen.values())) > 120:
            self.seen.popitem(last=False)
        if exempt or action not in self.limits or event_id in self.seen:
            return False
        self.seen[event_id] = now
        while len(self.seen) > 10000:
            self.seen.popitem(last=False)
        key = (actor, action)
        history = self.events.setdefault(key, deque(maxlen=self.limits[action]))
        history.append(now)
        self.events.move_to_end(key)
        while len(self.events) > 2000:
            self.events.popitem(last=False)
        return len(history) >= self.limits[action]
