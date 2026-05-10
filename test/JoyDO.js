// JoyDO.js
// v1.0.0
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

// Métodos vía Prototype
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
    }
};

DisplayObject.prototype.addChild = function (child) {
    this.children.push(child);
};