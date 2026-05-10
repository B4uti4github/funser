initWindow(800, 600);
initAudioSupport();
configureStage("FUNSER 1.3.0 - INFINITE CLICKER GAME");
require("JoyDO.js");

// Cargamos el sonido una sola vez
var popSound = new AudioInstance("BoopBingBop.wav");
var targets = [];

function spawnTarget() {
    var t = new DisplayObject("rect");
    t.x = 50 + Math.random() * 700;
    t.y = 50 + Math.random() * 500;
    t.width = 40;
    t.height = 40;
    t.color = {
        r: Math.floor(Math.random() * 255),
        g: 200,
        b: 100,
        a: 255
    };
    targets.push(t);
}

// Población inicial
for (var i = 0; i < 10; i++) spawnTarget();

function onUpdate(dt) {
    var mouse = getMousePos();
    var leftClick = isMousePressed(MOUSE_LEFT);
    var wheel = getMouseWheel();

    if (wheel > 0) {
        for (var i = 0; i < targets.length; i++) {
            var obj = targets[i];
            obj.height += wheel;
            obj.width += wheel;
        }
    } else if (wheel < 0) {
        for (var i = 0; i < targets.length; i++) {
            var obj = targets[i];
            var w = obj.width;
            var h = obj.height;
            if (w <= 10 || h <= 10) continue;
            obj.height += wheel;
            obj.width += wheel;
        }
    }

    for (var i = targets.length - 1; i >= 0; i--) {
        var obj = targets[i];

        if (leftClick && obj.isHit(mouse.x, mouse.y)) {
            // --- RETOQUE PERSISTENTE ---
            // Si el objeto está arriba, suena agudo. Si está abajo, grave.
            var pitch = 2.0 - (obj.y / 600) * 1.5;
            popSound.setPitch(pitch);

            // Panoramización simulada por volumen basado en X
            var vol = 0.3 + (obj.x / 800) * 0.7;
            popSound.setVolume(vol);

            popSound.restart(); // Al ser persistente, el cambio es inmediato

            targets.splice(i, 1);
            spawnTarget();
            continue;
        }

        obj.render();
    }

    // Dibujar mira
    render("text", "+", mouse.x - 6, mouse.y - 10, 25, { r: 255, g: 255, b: 255, a: 255 });
}