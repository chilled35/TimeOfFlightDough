/**
 * ESPHomeSensor
 * Connects to an ESPHome device's built-in web server SSE endpoint (/events)
 * to receive real-time sensor state updates without HA's 255-char state limit.
 *
 * Usage:
 *   const sse = new ESPHomeSensor('http://dough-monitor.local');
 *   sse.onState = (id, state) => { ... };
 *   sse.connect();
 */
export class ESPHomeSensor {
  constructor(baseUrl) {
    this._base   = baseUrl.replace(/\/$/, '');
    this._source = null;
    this.onState      = null;
    this.onConnect    = null;
    this.onDisconnect = null;
  }

  connect() {
    if (this._source) this._source.close();

    this._source = new EventSource(this._base + '/events');

    this._source.addEventListener('state', (e) => {
      try {
        const data = JSON.parse(e.data);
        if (data.id && data.state !== undefined) {
          if (this.onState) this.onState(data.id, data.state);
        }
      } catch (err) {
        console.warn('ESPHome SSE parse error:', err, e.data);
      }
    });

    this._source.onopen = () => {
      console.info('ESPHome SSE connected to', this._base);
      if (this.onConnect) this.onConnect();
      // Fetch current states immediately so we don't wait for the next update
      this._fetchAll();
    };

    this._source.onerror = () => {
      if (this.onDisconnect) this.onDisconnect();
    };
  }

  disconnect() {
    if (this._source) { this._source.close(); this._source = null; }
  }

  // Fetch current state of a specific text sensor via REST
  async _fetchAll() {
    try {
      const res = await fetch(this._base + '/text_sensor/grid_data');
      if (res.ok) {
        const data = await res.json();
        if (data.value && this.onState)
          this.onState('text_sensor-grid_data', data.value);
      }
    } catch (e) {
      console.warn('ESPHome REST fetch failed:', e);
    }
  }
}
