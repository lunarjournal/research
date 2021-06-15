// Author: Dylan Muller
// Student Number: MLLDYL002

var start;
var native = chrome.runtime.connectNative('com.secure.pass');
var cmd = "";
var files = [];
var port = 0;
var filename = "";
var authorized = false;
var url = "";

chrome.storage.local.get("password", function (items) {
  var password = null;
  if (items["password"] != null) {
    password = items["password"];
  }

  if (password == null) {
    var password = window.prompt("Set master password: ");
    while (password.length < 4) {
      password = window.prompt("Password must be greater than 4 characters: ");
    }
    chrome.storage.local.set({ "password": password });
    authorized = true;
  } else {
    var compare = window.prompt("Enter master password: ");
    if (compare == password) {
      authorized = true;
      alert("Successfully authenticated!");
    }else{
      alert("Invalid password, services restricted!");
    }
  }

  if (authorized == true) {
    native.onMessage.addListener(function (res) {
      console.log("response received : " + res.msg);
      if (cmd == "list") {
        var matches = res.msg.match(/\[(.*?)\]/);
        if (matches) {
          var match = matches[1];
          port.postMessage("[list]" + match);
        }
      }

      if (cmd == "delete") {
        port.postMessage("[delete]" + res.msg);
      }

      if (cmd == "decrypt") {
        var addr = res.msg + "|!#|";
        chrome.storage.local.get(null, function (items) {
          for (const key in items) {
            if (items[key] == filename) {
              addr += key;
            }
          }
          port.postMessage("[decrypt]" + addr);
        });

      }

      if (cmd == "format") {
        chrome.storage.local.clear();
        port.postMessage("[format]");
      }

      if (cmd == "decrypt_login") {

        var code = `
    var elements = document.getElementsByTagName("input");
    for (element in elements) {
        if (elements[element].type == "password") {
            elements[element].value = "STR_VALUE";
        }
        if (elements[element].type == "email") {
          elements[element].value = "STR_USER";
        }
    }

    var inputs = document.body.getElementsByTagName("*");
    var button = null;
    for (var i=0; i<inputs.length; i++) {
        if (inputs[i].getAttribute('onclick')!=null) {
            if(!inputs[i].className.includes("close"))
          button = inputs[i];
        }
      }

    if(button !=null){
        button.click();
    }
    else{
        button = document.getElementsByTagName("button");
        if(button.length != 0){
        for(var i = 0; i < button.length; i++){
            if(!button[i].className.includes("back") &&
              !button[i].className.includes("social") &&
              (button[i].innerText.toLowerCase().includes("sign")) ||
              button[i].innerText.toLowerCase().includes("log") ||
              button[i].innerText.toLowerCase().includes("next"))
            button[i].click();
        }
    }
    }

    var inputs = document.getElementsByTagName("input");
    for (var i=0; i<inputs.length; i++) {
        if (inputs[i].type.toLowerCase() == "submit") {
            if(!inputs[i].className.includes("close"))
            inputs[i].click();
        }
      }

    `
        var tokens = res.msg.split("|!#|");
        var username = tokens[0];
        var password = tokens[1];
        code = code.replace("STR_USER", username);
        code = code.replace("STR_VALUE", password);
        chrome.tabs.executeScript({
          code: code
        }, (results) => {
        });

      }

      if (cmd == "encrypt") {
        if (res.msg.includes("OK")) {
          chrome.storage.local.set({ [url]: filename }, function () {
            cmd = "list";
            native.postMessage({ cmd: "list" });
          });
        }

      }

      cmd = "";

    });

    native.onDisconnect.addListener(function () {
      console.log("disconnected..");
    });

    function isEmpty(obj) {
      return Object.keys(obj).length === 0;
    }

    chrome.extension.onConnect.addListener(function (lport) {
      port = lport;

      port.onMessage.addListener(function (msg) {
        var matches = msg.match(/\[(.*?)\]/);

        if (matches) {
          var match = matches[1];
          if (match == "format") {
            cmd = "format";
            port.postMessage("[busy]");
            native.postMessage({ cmd: "format" });
          }
          if (match == "password") {
            msg = msg.replace("[password]", "");
            msg = msg.split(",");

            chrome.storage.local.get(msg[0], function (items) {

              if (isEmpty(items) || items[[msg[0]]] == "") {
                cmd = "encrypt";
                filename = msg[2];
                url = msg[0];
                port.postMessage("[busy]");
                native.postMessage({ cmd: "encrypt", data: msg[1], filename: msg[2] });
              } else {
                alert("Entry: " + items[[msg[0]]] + " already associated with website!");
              }

            });
          }

          if (match == "list") {
            cmd = "list";
            native.postMessage({ cmd: "list" });
          }

          if (match == "delete") {
            cmd = "delete";
            msg = msg.replace("[delete]", "");
            chrome.storage.local.get(null, function (items) {
              for (var item in items) {
                if (items[item] == msg) {
                  chrome.storage.local.set({ [item]: "" }, function () {

                  });
                }
              }

            });
            native.postMessage({ cmd: "delete", filename: msg });
          }

          if (match == "decrypt") {
            cmd = "decrypt";
            msg = msg.replace("[decrypt]", "");
            filename = msg;
            native.postMessage({ cmd: "decrypt", filename: msg });
          }
        }

      });
    })

    chrome.tabs.onUpdated.addListener(function (tabId, changeInfo, tab) {
      if (changeInfo.status == 'complete') {
        var url = tab.url.split('?')[0];
        chrome.storage.local.get(url, function (items) {
          if (!isEmpty(items)) {
            var filename = items[url];
            cmd = "decrypt_login";
            native.postMessage({ cmd: "decrypt", filename: filename });
          }
        });
      }
    });
  }
});