//globals
let blueToothRXCharacteristic;//this is a blu
let blueToothTXCharacteristic;//this is a blu

let blueTooth;
let connectButton;
let speedologoImg;



//status variables
let newData = false;
//let binFileInput;

function preload() {
  speedologoImg = loadImage('data/speedoConfLogo.png');
  speedoImg = loadImage('data/speedoImg.png');
}

function setup() {

  // Create a p5ble class
  console.log("setting up");
  blueTooth = new p5ble();

  connectButton = createButton('CONNECT');
  connectButton.mousePressed(connectToBle);
  connectButton.position(15, 250);
  connectButton.style('color', color(255));
  connectButton.style('background-color', color(77, 158, 106));

  let yPositionStart = 200;
  pwTitle = createElement('h3', 'Please enter password');
  pwTitle.position(10, yPositionStart);
  pwInput = createInput('', 'password');
  pwInput.position(10, pwTitle.y + pwTitle.size().height + 2);
  verifyButton = createButton('Verify');
  verifyButton.position(pwInput.x + pwInput.size().width + 5, pwInput.y);
  verifyButton.mousePressed(checkPw);
  statusTitle = createElement('h3', '');
  statusTitle.position(100, yPositionStart);
  //****************************************
  kmstandTitle = createElement('h3', 'Enter new km');
  kmstandTitle.position(10, yPositionStart);
  kmstandInput = createInput('0');
  kmstandInput.position(kmstandTitle.x + kmstandTitle.size().width + 5 , yPositionStart);
  kmstandUpdateButton = createButton('Update');
  kmstandUpdateButton.position(kmstandInput.x + kmstandInput.size().width + 5, kmstandInput.y);
  kmstandUpdateButton.mousePressed(checkValue(kmstandInput.value(), "kmStand"));
  //**************************************
  ppKmTitle = createElement('h3', 'Enter new pulses per KM');
  ppKmTitle.position(10, kmstandUpdateButton.y + kmstandUpdateButton.size().height + 30);
  ppKmInput = createInput('0');
  ppKmInput.position(ppKmTitle.x + ppKmTitle.size().width + 5 , ppKmTitle.y);
  ppKmUpdateButton = createButton('Update');
  ppKmUpdateButton.position(ppKmInput.x + ppKmInput.size().width + 5, ppKmInput.y);
  ppKmUpdateButton.mousePressed(checkValue(ppKmInput.value(), "ppKM"));

  createCanvas(1000, ppKmUpdateButton.y + 100);

  hidePasswordParam();
}

function draw() {
  drawScreen();
}
