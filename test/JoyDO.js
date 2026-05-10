// JoyDO.js v1.3.5
function DisplayObject(type) {
    this.type = type || "container";
    this.x = 0;
    this.y = 0;
    this.width = 1;
    this.height = 1;
    this.text = "";
    this.color = { r: 255, g: 255, b: 255, a: 255 };
    this.children = [];
    this.drawCalls = [];
}

DisplayObject.prototype.add = function (child) {
    this.children.push(child);
    return child;
};

DisplayObject.prototype.addDrawCall = function (config) {
    this.drawCalls.push(config);
};

// --- Métodos de dibujo v1.3.5 ---
DisplayObject.prototype.drawCircle = function (r, ox, oy, col) {
    this.addDrawCall({ type: "circle", width: r, offsetX: ox, offsetY: oy, color: col });
};

DisplayObject.prototype.drawEllipse = function (rx, ry, ox, oy, col) {
    this.addDrawCall({ type: "ellipse", width: rx, height: ry, offsetX: ox, offsetY: oy, color: col });
};

DisplayObject.prototype.drawArc = function (r, start, end, ox, oy, col) {
    this.addDrawCall({ type: "arc", width: r, angleStart: start, angleEnd: end, offsetX: ox, offsetY: oy, color: col });
};

DisplayObject.prototype.render = function (px, py) {
    var gx = this.x + (px || 0);
    var gy = this.y + (py || 0);

    // Fallback retrocompatible
    if (this.drawCalls.length === 0 && this.type !== "container") {
        render(this.type, (this.type === "text" ? this.text : this.width), gx, gy, this.height, this.color);
    }

    // Comandos modernos
    for (var i = 0; i < this.drawCalls.length; i++) {
        var cmd = this.drawCalls[i];
        var rx = gx + (cmd.offsetX || 0);
        var ry = gy + (cmd.offsetY || 0);

        if (cmd.type === "arc") {
            // Sincronizado con tu C: radio(2), x(3), y(4), start(5), end(6), color(7)
            render("arc", cmd.width, rx, ry, cmd.angleStart, cmd.angleEnd, cmd.color);
        } else {
            render(cmd.type, cmd.width, rx, ry, cmd.height, cmd.color);
        }
    }

    for (var j = 0; j < this.children.length; j++) {
        this.children[j].render(gx, gy);
    }
};

DisplayObject.prototype.isHit = function (mx, my) {
    var dx = mx - this.x;
    var dy = my - this.y;
    return (dx * dx + dy * dy) <= (this.width * this.width);
};

function AudioInstance(path) { this.path = path; this.id = -1; }
AudioInstance.prototype.play = function () {
    // Si ya está sonando, no hacemos nada para evitar el solapamiento
    if (this.id !== -1) {
        this.restart();
        return this.id;
    }
    this.id = playAudio(this.path);
    return this.id;
};

// JoyDO.js
AudioInstance.prototype.restart = function () {
    // Si ya existe un ID, lo mandamos al principio.
    // Si el motor de C ya limpió el ID porque terminó, playAudio devolverá uno nuevo.
    if (this.id !== -1) {
        seekAudio(this.id, 0);
    } else {
        this.id = playAudio(this.path);
    }
};
// JoyDO.js v1.3.0
AudioInstance.prototype.setPitch = function (val) {
    if (this.id !== -1) setAudioPitch(this.id, val);
};

AudioInstance.prototype.setVolume = function (val) {
    if (this.id !== -1) setAudioVolume(this.id, val);
};

// JoyDO.js v1.3.0 - REFIX
// JoyDO.js v1.3.0
AudioInstance.prototype.restart = function () {
    if (this.id !== -1) {
        // 1. Lo mandamos al inicio (segundo 0)
        seekAudio(this.id, 0);
        // 2. Le decimos a Raylib que lo ejecute de nuevo
        // Esto funciona aunque el sonido haya llegado al final antes
        playAudioById(this.id);
    } else {
        // Si nunca se cargó, lo cargamos por primera vez
        this.id = playAudio(this.path);
    }
};

// Constantes
var MOUSE_LEFT = 0, MOUSE_RIGHT = 1, MOUSE_MIDDLE = 2;
var KEY_SPACE = 32, KEY_W = 87, KEY_S = 83, KEY_A = 65, KEY_D = 68;