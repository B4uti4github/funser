// 1. Inicializamos la ventana primero (Requerido por nuestro main.c)
initWindow(800, 600);

// 2. Configuramos el Stage (Título e icono opcional)
configureStage("Funser Engine Test");

require("JoyDO.js");

// --- Instancias ---
var logoContainer = new DisplayObject("rect");
var texto = new DisplayObject("text");
texto.text = "FUNSER";
texto.x = 25;
texto.y = 30;
texto.color = { r: 255, g: 255, b: 255, a: 255 };

logoContainer.addChild(texto);

// --- Lógica de movimiento ---
var velX = 220;
var velY = 220;

function changeColor(obj) {
    obj.color.r = Math.floor(Math.random() * 255);
    obj.color.g = Math.floor(Math.random() * 255);
    obj.color.b = Math.floor(Math.random() * 255);
}

// Función global para que C la encuentre
function onUpdate(dt) {
    // 1. Movimiento
    logoContainer.x = logoContainer.x + (velX * dt);
    logoContainer.y = logoContainer.y + (velY * dt);

    // 2. Rebote X
    if (logoContainer.x + logoContainer.width >= 800 || logoContainer.x <= 0) {
        velX = velX * -1;
        changeColor(logoContainer);
    }

    // 3. Rebote Y
    if (logoContainer.y + logoContainer.height >= 600 || logoContainer.y <= 0) {
        velY = velY * -1;
        changeColor(logoContainer);
    }

    // 4. Dibujo
    logoContainer.render();
}