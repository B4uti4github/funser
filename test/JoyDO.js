// JoyDO.js
// v1.1.0 - Added Audio Support
// An Display Object engine for Funser.

function DisplayObject(type) {
    this.type = type || "rect";
    this.x = 100;
    this.y = 100;
    this.width = 150;
    this.height = 80;
    this.text = "";
    this.color = { r: 0, g: 150, b: 255, a: 255 };
    this.children = [];
}

DisplayObject.prototype.render = function (parentX, parentY) {
    var px = parentX || 0;
    var py = parentY || 0;
    var globalX = this.x + px;
    var globalY = this.y + py;

    this.draw(globalX, globalY);

    for (var i = 0; i < this.children.length; i++) {
        this.children[i].render(globalX, globalY);
    }
};

DisplayObject.prototype.draw = function (gx, gy) {
    if (this.type === "rect") {
        render("rect", this.width, gx, gy, this.height, this.color);
    } else if (this.type === "text") {
        render("text", this.text, gx, gy, 20, this.color);
    } else if (this.type === "image") {
        render("image", this.text, gx, gy, 0, this.color); // Usamos 'text' para el path
    }
};

DisplayObject.prototype.addChild = function (child) {
    this.children.push(child);
};

// JoyDO.js - Sección de AudioInstance mejorada contra leaks
function AudioInstance(path) {
    this.path = path;
    this.id = -1;
}

AudioInstance.prototype.play = function () {
    // Si ya tenemos un ID, no pedimos uno nuevo a C para no saturar los canales
    if (this.id !== -1) {
        this.restart();
        return this.id;
    }
    this.id = playAudio(this.path);
    return this.id;
};

AudioInstance.prototype.restart = function () {
    if (this.id !== -1) {
        seekAudio(this.id, 0);
    } else {
        this.play();
    }
};

AudioInstance.prototype.release = function () {
    if (this.id !== -1) {
        stopAudio(this.id); // Esta función en C debe llamar a UnloadMusicStream
        this.id = -1;
    }
};