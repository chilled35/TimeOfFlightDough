/**
 * HAWebSocket — minimal Home Assistant WebSocket API client (ES module).
 */
export class HAWebSocket {
  constructor(haUrl, token) {
    this._wsUrl   = haUrl.replace(/^http/, 'ws') + '/api/websocket';
    this._token   = token;
    this._socket  = null;
    this._msgId   = 1;
    this._pending = new Map();
    this._subscriptions     = new Map();
    this._stateSubscriptions = new Map();
    this._authenticated = false;

    this.onConnect    = null;
    this.onDisconnect = null;
    this.onError      = null;
  }

  connect() {
    this._socket = new WebSocket(this._wsUrl);

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

  subscribeState(entityId, callback) {
    if (!this._stateSubscriptions.has(entityId))
      this._stateSubscriptions.set(entityId, []);
    this._stateSubscriptions.get(entityId).push(callback);

    if (this._authenticated) {
      this._subscribeEvents();
      this._fetchCurrentState(entityId, callback);
    }

    return () => {
      const cbs = this._stateSubscriptions.get(entityId) || [];
      const idx = cbs.indexOf(callback);
      if (idx !== -1) cbs.splice(idx, 1);
    };
  }

  _fetchCurrentState(entityId, callback) {
    const id = this._nextId();
    this._pending.set(id, {
      resolve: (result) => {
        const state = result?.state;
        if (state) callback(state);
      },
      reject: (err) => console.warn('get_state failed:', err),
    });
    this._send({ id, type: 'get_states' });
    // get_states returns all states; intercept in result handler
    const origResolve = this._pending.get(id).resolve;
    this._pending.set(id, {
      resolve: (result) => {
        if (Array.isArray(result)) {
          const entity = result.find(e => e.entity_id === entityId);
          if (entity) callback(entity.state);
        }
      },
      reject: (err) => console.warn('get_states failed:', err),
    });
  }

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

      case 'result': {
        const cb = this._pending.get(msg.id);
        if (cb) {
          this._pending.delete(msg.id);
          msg.success ? cb.resolve(msg.result) : cb.reject(new Error(msg.error?.message));
        }
        break;
      }
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
    this._send({ id, type: 'subscribe_events', event_type: 'state_changed' });
    this._subscriptions.set(id, true);
  }
}
