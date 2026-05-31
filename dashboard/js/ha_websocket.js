/**
 * HAWebSocket
 * Minimal Home Assistant WebSocket API client.
 *
 * Usage:
 *   const ws = new HAWebSocket('http://homeassistant.local:8123', 'LONG_LIVED_TOKEN');
 *   ws.onConnect    = () => { ... };
 *   ws.onDisconnect = () => { ... };
 *   ws.onError      = (err) => { ... };
 *   ws.connect();
 *   ws.subscribeState('sensor.foo', (newState) => { ... });
 */
class HAWebSocket {
  constructor(haUrl, token) {
    // Convert http(s):// URL to ws(s):// WebSocket URL
    this._wsUrl = haUrl.replace(/^http/, 'ws') + '/api/websocket';
    this._token = token;
    this._socket = null;
    this._msgId = 1;
    this._pending = new Map();     // id → { resolve, reject }
    this._subscriptions = new Map(); // id → callback
    this._stateSubscriptions = new Map(); // entityId → callback[]
    this._authenticated = false;

    this.onConnect    = null;
    this.onDisconnect = null;
    this.onError      = null;
  }

  connect() {
    this._socket = new WebSocket(this._wsUrl);

    this._socket.onopen = () => {
      // HA sends an auth_required message immediately on open; handled in onmessage
    };

    this._socket.onclose = () => {
      this._authenticated = false;
      if (this.onDisconnect) this.onDisconnect();
    };

    this._socket.onerror = (err) => {
      if (this.onError) this.onError(err);
    };

    this._socket.onmessage = (event) => {
      const msg = JSON.parse(event.data);
      this._handleMessage(msg);
    };
  }

  disconnect() {
    if (this._socket) this._socket.close();
  }

  /**
   * Subscribe to state_changed events for a specific entity.
   * callback(newState: string) is called on every change.
   * Returns an unsubscribe function.
   */
  subscribeState(entityId, callback) {
    if (!this._stateSubscriptions.has(entityId))
      this._stateSubscriptions.set(entityId, []);
    this._stateSubscriptions.get(entityId).push(callback);

    // If already authenticated, subscribe now; otherwise it fires after auth
    if (this._authenticated)
      this._subscribeEvents();

    return () => {
      const cbs = this._stateSubscriptions.get(entityId) || [];
      const idx = cbs.indexOf(callback);
      if (idx !== -1) cbs.splice(idx, 1);
    };
  }

  // ---- Private ----------------------------------------------------------

  _send(msg) {
    if (this._socket && this._socket.readyState === WebSocket.OPEN)
      this._socket.send(JSON.stringify(msg));
  }

  _nextId() { return this._msgId++; }

  _handleMessage(msg) {
    switch (msg.type) {
      case 'auth_required':
        this._send({ type: 'auth', access_token: this._token });
        break;

      case 'auth_ok':
        this._authenticated = true;
        this._subscribeEvents();
        if (this.onConnect) this.onConnect();
        break;

      case 'auth_invalid':
        if (this.onError) this.onError(new Error('HA auth invalid: ' + msg.message));
        this._socket.close();
        break;

      case 'event':
        this._handleEvent(msg);
        break;

      case 'result':
        {
          const cb = this._pending.get(msg.id);
          if (cb) {
            this._pending.delete(msg.id);
            msg.success ? cb.resolve(msg.result) : cb.reject(new Error(msg.error?.message));
          }
        }
        break;
    }
  }

  _handleEvent(msg) {
    const event = msg.event;
    if (!event) return;

    if (event.event_type === 'state_changed') {
      const entityId = event.data?.entity_id;
      const newState = event.data?.new_state?.state;
      const cbs = this._stateSubscriptions.get(entityId);
      if (cbs) cbs.forEach(cb => cb(newState));
    }
  }

  _subscribeEvents() {
    if (this._stateSubscriptions.size === 0) return;
    const id = this._nextId();
    this._send({
      id,
      type: 'subscribe_events',
      event_type: 'state_changed',
    });
    this._subscriptions.set(id, true);
  }
}
