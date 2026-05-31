/**
 * DoughVisualizer
 * Three.js 3D surface height map + 2D heatmap canvas for the 8×8 VL53L5CX grid.
 *
 * The 64-element values array is interpreted as a row-major 8×8 grid.
 * In delta mode: positive values = dough has risen above baseline.
 * In raw mode:   values are distances in mm (lower = closer to sensor).
 */
class DoughVisualizer {
  constructor(canvas3d, canvas2d) {
    this._canvas3d  = canvas3d;
    this._canvas2d  = canvas2d;
    this._zScale    = 3.0;
    this._viewMode  = '3d';
    this._lastValues = null;
    this._lastRes    = 8;
    this._isDelta    = true;

    this._init3D();
    this._initHeatmap();
    this._animate();
  }

  // ---- Public API --------------------------------------------------------

  update(values, res, isDelta) {
    this._lastValues = values;
    this._lastRes    = res;
    this._isDelta    = isDelta;
    this._updateSurface(values, res, isDelta);
    this._updateHeatmap(values, res, isDelta);
  }

  setZScale(scale) {
    this._zScale = scale;
    if (this._lastValues)
      this._updateSurface(this._lastValues, this._lastRes, this._isDelta);
  }

  setViewMode(mode) {
    this._viewMode = mode;
  }

  // ---- Three.js init -----------------------------------------------------

  _init3D() {
    const canvas = this._canvas3d;
    const w = canvas.clientWidth  || 600;
    const h = canvas.clientHeight || 400;

    this._renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
    this._renderer.setSize(w, h);
    this._renderer.setPixelRatio(window.devicePixelRatio);
    this._renderer.setClearColor(0x0d0d0d);

    this._scene  = new THREE.Scene();
    this._camera = new THREE.PerspectiveCamera(45, w / h, 0.1, 1000);
    this._camera.position.set(0, -12, 10);
    this._camera.lookAt(0, 0, 0);

    this._controls = new THREE.OrbitControls(this._camera, canvas);
    this._controls.enableDamping = true;
    this._controls.dampingFactor = 0.08;

    // Ambient + directional light for surface shading
    this._scene.add(new THREE.AmbientLight(0xffffff, 0.4));
    const dirLight = new THREE.DirectionalLight(0xffffff, 1.0);
    dirLight.position.set(5, 10, 8);
    this._scene.add(dirLight);

    // Grid helper for reference
    const gridHelper = new THREE.GridHelper(8, 8, 0x333333, 0x222222);
    gridHelper.rotation.x = Math.PI / 2;
    this._scene.add(gridHelper);

    // Build the surface mesh (8×8 = 7×7 quads)
    const geo = new THREE.PlaneGeometry(8, 8, 7, 7);
    const mat = new THREE.MeshPhongMaterial({
      vertexColors: true,
      side: THREE.DoubleSide,
      shininess: 60,
    });
    this._mesh = new THREE.Mesh(geo, mat);
    this._scene.add(this._mesh);

    // Wireframe overlay
    const wireMat = new THREE.MeshBasicMaterial({
      color: 0x00ffff,
      wireframe: true,
      opacity: 0.12,
      transparent: true,
    });
    this._wireMesh = new THREE.Mesh(geo.clone(), wireMat);
    this._scene.add(this._wireMesh);

    // Initialise vertex colours
    this._initVertexColors();

    // Handle canvas resize
    window.addEventListener('resize', () => this._onResize());
  }

  _initVertexColors() {
    const geo = this._mesh.geometry;
    const count = geo.attributes.position.count;
    const colors = new Float32Array(count * 3);
    geo.setAttribute('color', new THREE.BufferAttribute(colors, 3));
  }

  // ---- Surface update ----------------------------------------------------

  _updateSurface(values, res, isDelta) {
    if (!values || values.length < res * res) return;

    const geo = this._mesh.geometry;
    const pos = geo.attributes.position;
    const col = geo.attributes.color;

    // PlaneGeometry vertices go row by row, (res) vertices per row
    // values[row * res + col] maps to vertex[row * res + col]
    const n = res * res;

    // Compute min/max for colour normalisation
    const valid = values.filter(v => v !== null && v !== undefined);
    const vMin = Math.min(...valid);
    const vMax = Math.max(...valid);
    const vRange = vMax - vMin || 1;

    for (let i = 0; i < n; i++) {
      const v = values[i] ?? 0;
      // Z height: in delta mode positive = risen, scale up for visibility
      // In raw mode invert so closer (lower mm) = higher peak
      const z = isDelta
        ? (v / 100.0) * this._zScale
        : ((vMax - v) / vRange) * this._zScale;

      pos.setZ(i, z);

      // Colour: blue (low/flat) → green → red (high/risen)
      const t = (v - vMin) / vRange;
      const rgb = this._heatColour(t);
      col.setXYZ(i, rgb.r, rgb.g, rgb.b);
    }

    pos.needsUpdate = true;
    col.needsUpdate = true;
    geo.computeVertexNormals();

    // Sync wireframe geometry
    this._wireMesh.geometry.attributes.position.copy(pos);
    this._wireMesh.geometry.attributes.position.needsUpdate = true;
  }

  // ---- Heatmap (2D canvas) -----------------------------------------------

  _initHeatmap() {
    this._hCtx = this._canvas2d.getContext('2d');
  }

  _updateHeatmap(values, res, isDelta) {
    const ctx   = this._hCtx;
    const canvas = this._canvas2d;
    const w = canvas.width  = canvas.clientWidth  || 400;
    const h = canvas.height = canvas.clientHeight || 400;
    const cellW = w / res;
    const cellH = h / res;

    const valid = values.filter(v => v !== null);
    const vMin = Math.min(...valid);
    const vMax = Math.max(...valid);
    const vRange = vMax - vMin || 1;

    ctx.clearRect(0, 0, w, h);

    for (let row = 0; row < res; row++) {
      for (let c = 0; c < res; c++) {
        const v = values[row * res + c] ?? 0;
        const t = isDelta
          ? (v - vMin) / vRange
          : (vMax - v) / vRange; // invert for raw
        const { r, g, b } = this._heatColour(t);
        ctx.fillStyle = `rgb(${Math.round(r*255)},${Math.round(g*255)},${Math.round(b*255)})`;
        ctx.fillRect(c * cellW, row * cellH, cellW, cellH);

        // Zone value label
        ctx.fillStyle = 'rgba(255,255,255,0.7)';
        ctx.font = `${Math.max(10, cellW * 0.25)}px monospace`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(v, c * cellW + cellW / 2, row * cellH + cellH / 2);
      }
    }

    // Grid lines
    ctx.strokeStyle = 'rgba(255,255,255,0.15)';
    ctx.lineWidth = 1;
    for (let i = 0; i <= res; i++) {
      ctx.beginPath(); ctx.moveTo(i * cellW, 0); ctx.lineTo(i * cellW, h); ctx.stroke();
      ctx.beginPath(); ctx.moveTo(0, i * cellH); ctx.lineTo(w, i * cellH); ctx.stroke();
    }
  }

  // ---- Colour map: blue → cyan → green → yellow → red ------------------

  _heatColour(t) {
    t = Math.max(0, Math.min(1, t));
    let r, g, b;
    if (t < 0.25) {
      r = 0; g = t / 0.25; b = 1;
    } else if (t < 0.5) {
      r = 0; g = 1; b = 1 - (t - 0.25) / 0.25;
    } else if (t < 0.75) {
      r = (t - 0.5) / 0.25; g = 1; b = 0;
    } else {
      r = 1; g = 1 - (t - 0.75) / 0.25; b = 0;
    }
    return { r, g, b };
  }

  // ---- Render loop -------------------------------------------------------

  _animate() {
    requestAnimationFrame(() => this._animate());
    if (this._viewMode === '3d') {
      this._controls.update();
      this._renderer.render(this._scene, this._camera);
    }
  }

  _onResize() {
    const canvas = this._canvas3d;
    const w = canvas.clientWidth;
    const h = canvas.clientHeight;
    if (w && h) {
      this._camera.aspect = w / h;
      this._camera.updateProjectionMatrix();
      this._renderer.setSize(w, h);
    }
  }
}
