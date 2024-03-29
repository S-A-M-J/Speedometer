//ALL DATA SENT OUT FROM THE GUI TOO THE BOARD HERE

// to sanitize strings **********
function checkUserString(userString, lengthCheck, numberCheck = false) {
  if (match(userString, "#") != null || match(userString, ",") != null) {
    return 'error no # or comma';
  }
  if (userString.length >= lengthCheck) {
    return 'error too long';
  }
  if (numberCheck) {
    if(/^\d+$/.test(userString)){
      return 'invalid value entered';
    }
   
  }
  return null;
}

function checkPw() {
  let sanitizer = checkUserString(pwInput.value(), 50);
  if (sanitizer != null) {
    pwInput.value(sanitizer);
    return;
  } else {
    sendData("#pw," + pwInput.value());
  }
}

function checkppKMValue() {
  let sanitizer = checkUserString(ppKmInput.value(), 6, true);
  if (sanitizer != null) {
    if(statusTitle){
      statusTitle.style.color = 'red';
      statusTitle.textContent = sanitizer;
    }
  } else {
    sendData("#ppKm," + ppKmInput.value());
  }
}

function checkKMValue() {
  let sanitizer = checkUserString(kmstandInput.value(), 6, true);
  if (sanitizer != null) {
    if(statusTitle){
      statusTitle.style.color = 'red';
      statusTitle.textContent = sanitizer;
    }
  } else {
    sendData("#kmstand," + kmstandInput.value());
  }
}

function resetCommand() {
  sendData("#reset,");
  disconnectBle();
}

function sendDebugData() {
  sendData("#debug");
}


