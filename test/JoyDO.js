// JoyDO.js v1.3.0
function DisplayObject(type) {
    this.type = type || "rect";
    this.x = 0; this.y = 0;
    this.width = 100; this.height = 100;
    this.text = ""; // También se usa para la ruta de la imagen
    this.color = { r: 255, g: 255, b: 255, a: 255 };
    this.children = [];
}

DisplayObject.prototype.render = function (px, py) {
    var gx = this.x + (px || 0);
    var gy = this.y + (py || 0);
    if (this.type === "rect") render("rect", this.width, gx, gy, this.height, this.color);
    else if (this.type === "text") render("text", this.text, gx, gy, 20, this.color);
    else if (this.type === "image") render("image", this.text, gx, gy, 0, this.color);
    for (var i = 0; i < this.children.length; i++) this.children[i].render(gx, gy);
};

DisplayObject.prototype.isHit = function (mx, my) {
    return (mx >= this.x && mx <= this.x + this.width && my >= this.y && my <= this.y + this.height);
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