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