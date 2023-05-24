
let speedoImgPosX = 400;

function drawScreen() {
  textSize(18);
  textAlign(LEFT, TOP);
  background(255);
  fill(0);
  image(speedologoImg, 0, 0);
  if (isConnected) {
    //text('Bluetooth Connected :)', 10, 140);
    if (newData) {
      image(speedoImg, speedoImgPosX, 200, 200, 200);
      fill(0);

      textAlign(LEFT, TOP);
      textSize(18);
      fill(0);
      
    }
  } else {
    //text('Bluetooth Disconnected :/', 80, 150);
    newData=false;
  }
}
