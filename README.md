# Funser
An JS Runtime for 2D Games. Powered by mujs and raylib. Written in C.

It is 500x more lighter than NodeJS. It is not a framework, it is a runtime.

### JoyDO.js (Library)

A good practice library for use this runtime, it automatizes many things, like the draw call list, The Display Objects like flash (so you can use the best practices of flash) and the audio engine.

In the future, we are going to use physics in JoyDO.js.

## Examples:
Run this script with funser.exe test/test1.js 

Example 1:
test1.js (it needs JoyDO.js):
```javascript
// test1.js
initWindow(800, 600);
initAudioSupport();
configureStage("Angry Aim Trainer v1.3.5");

require("JoyDO.js");

var popSound = playAudio("BoopBingBop.wav");
var enemies = [];
var score = 0;
var idCounter = 0; // ÍNDICE DE ID GLOBAL

function createEnemy(x, y) {
    idCounter++; // Incrementamos el ID cada vez que nace uno

    // 1. Contenedor principal (La cara)
    var face = new DisplayObject("container");
    face.x = x;
    face.y = y;
    face.width = 30; // Radio de colisión real

    // 2. Etiqueta de ID (Hijo de la cara)
    var label = new DisplayObject("text");
    label.text = "TARGET #" + idCounter;
    label.x = -25; // Centrado relativo
    label.y = -45; // Arriba de la cabeza
    label.color = { r: 255, g: 255, b: 0, a: 255 }; // Amarillo para que resalte
    face.add(label);

    var rojo = { r: 200, g: 0, b: 0, a: 255 };
    var blanco = { r: 255, g: 255, b: 255, a: 255 };

    // 3. Dibujo de la entidad
    face.drawCircle(30, 0, 0, rojo);            // Cabeza
    face.drawEllipse(5, 10, -12, -8, blanco);   // Ojo L
    face.drawEllipse(5, 10, 12, -8, blanco);    // Ojo R
    face.drawArc(15, 0, -180, 0, 10, blanco);    // Boca triste corregida

    face.vx = (Math.random() - 0.5) * 6;
    face.vy = (Math.random() - 0.5) * 6;

    return face;
}

// Generar 8 enemigos iniciales
for (var i = 0; i < 8; i++) {
    enemies.push(createEnemy(Math.random() * 700 + 50, Math.random() * 500 + 50));
}

function onUpdate(dt) {
    var mouse = getMousePos();

    for (var i = enemies.length - 1; i >= 0; i--) {
        var e = enemies[i];

        // Movimiento
        e.x += e.vx;
        e.y += e.vy;

        // Rebotes
        if (e.x < 30 || e.x > 770) e.vx *= -1;
        if (e.y < 30 || e.y > 570) e.vy *= -1;

        // Lógica de Hit
        if (isMousePressed(0) && e.isHit(mouse.x, mouse.y)) {
            playAudioById(popSound);
            enemies.splice(i, 1);
            score++;

            // Reponer enemigo
            enemies.push(createEnemy(Math.random() * 700 + 50, Math.random() * 500 + 50));

            // Dificultad extra
            if (score % 5 === 0) {
                enemies.push(createEnemy(Math.random() * 800, Math.random() * 600));
            }
            continue;
        }

        e.render();
    }

    // UI Superior
    render("text", "+", mouse.x, mouse.y, 20, { r: 255, g: 255, b: 255, a: 255 });
    render("text", "SCORE: " + score, 20, 20, 20, { r: 255, g: 255, b: 255, a: 255 });
    render("text", "LAST ID SPAWNED: " + idCounter, 20, 45, 15, { r: 200, g: 200, b: 200, a: 255 });
}
```

## Building:
For building Funser you need to have MinGW-w64 installed. If you don't have it installed, you can download it from [here](https://www.mingw-w64.org/downloads/)

### Windows (MinGW):

```bash
gcc src/main.c include/mujs/one.c "include/raylib/raylib-6.0_win64_mingw-w64/lib/libraylib.a" -o funser.exe -I"include/raylib/raylib-6.0_win64_mingw-w64/include" -I"include/mujs" -lopengl32 -lgdi32 -lwinmm
```