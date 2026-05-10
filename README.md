# Funser
An JS Runtime for 2D Games. Powered by mujs and raylib. Written in C.

It is 500x more lighter than NodeJS. It is not a framework, it is a runtime.

## Examples:
Run this script with funser.exe test/test1.js 

Example 1:
test1.js (it needs JoyDO.js):
```javascript
initWindow(800, 600);
configureStage("Funser Engine Test");
initAudioSupport();

require("JoyDO.js");

// Instanciamos el audio una sola vez
var bounceSound = new AudioInstance("BoopBingBop.wav");

var logo = new DisplayObject("rect");
logo.width = 120;
logo.height = 60;

var text = new DisplayObject("text");
text.text = "FUNSER";
text.x = 25;
text.y = 30;
text.color = { r: 255, g: 255, b: 255, a: 255 };

logo.addChild(text);

var velX = 300;
var velY = 300;

function onUpdate(dt) {
    logo.x += velX * dt;
    logo.y += velY * dt;

    // Rebote con AudioInstance
    if (logo.x + logo.width >= 800 || logo.x <= 0) {
        velX *= -1;
        bounceSound.restart(); // ¡Limpio!
    }

    if (logo.y + logo.height >= 600 || logo.y <= 0) {
        velY *= -1;
        bounceSound.restart(); // ¡Limpio!
    }

    logo.render();
}
```

## Building:
For building Funser you need to have MinGW-w64 installed. If you don't have it installed, you can download it from [here](https://www.mingw-w64.org/downloads/)

### Windows (MinGW):

```bash
gcc src/main.c include/mujs/one.c "include/raylib/raylib-6.0_win64_mingw-w64/lib/libraylib.a" -o funser.exe -I"include/raylib/raylib-6.0_win64_mingw-w64/include" -I"include/mujs" -lopengl32 -lgdi32 -lwinmm
```