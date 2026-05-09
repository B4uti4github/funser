// 1. Inicializamos la ventana primero (Requerido por nuestro main.c)
initWindow(800, 600);

// 2. Configuramos el Stage (Título e icono opcional)
configureStage("Flash Engine Test");

// Variables de estado
var x = 0;
var y = 300;
var speed = 150; // Píxeles por segundo
var rotation = 0;
var colors = [
    { r: 255, g: 0, b: 0 },   // Rojo
    { r: 0, g: 255, b: 0 },   // Verde
    { r: 0, g: 150, b: 255 }, // Azul
    { r: 255, g: 200, b: 0 }  // Amarillo
];
var currentColor = colors[2];

trace("--- Motor Iniciado ---");
console.log("Resolución: 800x600");

// La función que el motor llama 60 veces por segundo
function onUpdate(dt) {
    // A. Lógica de movimiento usando Delta Time
    x += speed * dt;

    // Si se sale de la pantalla, vuelve al inicio
    if (x > 800) {
        x = -100;
        // Cambiamos el título dinámicamente cada vez que reinicia
        configureStage("Flash Engine - Vuelta completa!");
    }

    // B. Renderizado de Rectángulos
    // render("rect", width, x, y, height, color)
    render("rect", 100, x, y - 50, 100, currentColor);

    // C. Renderizado de Texto
    // render("text", contenido, x, y, size, color)
    render("text", "Delta Time: " + dt.toFixed(4), 10, 10, 20, { r: 0, g: 0, b: 0 });
    render("text", "FPS: " + (1 / dt).toFixed(0), 10, 35, 20, { r: 50, g: 50, b: 50 });
    render("text", "Presiona 'trace' en la consola", 10, 560, 18, { r: 150, g: 150, b: 150 });

    // D. Renderizado de Imagen (Descomenta si tienes un archivo "hero.png")
    // render("image", "hero.png", x + 25, y - 25);

    // E. Uso de la consola para debug (no abusar en onUpdate porque llena la terminal)
    if (Math.random() > 0.995) {
        console.log("Posicion X actual:", x.toFixed(2));
    }
}