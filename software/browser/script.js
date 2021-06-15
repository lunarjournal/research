// Author: Dylan Muller
// Student Number: MLLDYL002

document.addEventListener('DOMContentLoaded', function () {
    function extractPassword() {
        var value = [];
        var url = document.location.href.split('?')[0];
        var password = null;
        var username = null;
        value.push(url)

        var elements = document.getElementsByTagName("input");
        for (element in elements) {
            if (elements[element].type == "password") {
                password = elements[element].value;
            }
            if (elements[element].type == "email") {
                username = elements[element].value;
            }
        }
        value.push(username + "|!#|" + password);
        console.log(value);
        return value;
    }

    var port = chrome.extension.connect();

    document.getElementById("format").addEventListener("click", function () {
        port.postMessage("[format]");

    });
    document.getElementById("save").addEventListener("click", function () {
        chrome.tabs.executeScript({
            code: '(' + extractPassword + ')();'
        }, (results) => {

            if (results && results[0][1]) {
                var tokens = results[0][1].split("|!#|");
                var username = tokens[0];
                var password = tokens[1];
                var cmd = "[password]";
                if (password.length == 0) {
                    alert("No password to save!");
                } else {
                    var filename = window.prompt("Please enter filename (4 characters max) :");
                    if (filename.length > 4) {
                        alert("Filename too large!")
                    }
                    else if (filename.length <= 0) {
                        alert("Invalid filename!");
                    }
                    else {
                        cmd = cmd + results[0][0] + "," + results[0][1] + "," + filename;
                        port.postMessage(cmd);
                    }
                }
            }
            else{
                alert("No credential found!");
            }
        });
    });
    document.getElementById("load").style.display = "block";
    document.getElementById("save").disabled = true;
    port.postMessage("[list]");
    port.onMessage.addListener(function (msg) {
        var matches = msg.match(/\[(.*?)\]/);
        if (matches) {
            var match = matches[1];

            if(match == "format"){
                port.postMessage("[list]");
            }
            if(match == "busy"){
                document.getElementById("load").style.display = "block";
            }
            if (match == "delete") {
                msg = msg.replace("[delete]", "");

                document.getElementById("load").style.display = "block";
                port.postMessage("[list]");

            }

            if (match == "decrypt") {
                msg = msg.replace("[decrypt]", "");
                var tokens = msg.split("|!#|");
                var username = tokens[0];
                var password = tokens[1];
                var address = tokens[2];
                alert("Username : " + username + "\n" +
                      "Password : " + password + "\n" +
                      "Website : " + address);
            }



            if (match == "list") {
                chrome.tabs.query({active: true, lastFocusedWindow: true}, tabs => {
                var activeId = "";
                var url = tabs[0].url.split('?')[0];
                chrome.storage.local.get(url, function (items) {

                    for (var item in items) {
                        if(item == url){
                            activeId = items[item];
                            
                        }
                    }

                if(activeId != ""){
                    document.getElementById("save").disabled = true;
                }else{
                    document.getElementById("save").disabled = false;
                }
                
                msg = msg.replace("[list]", "");
                var tokens = msg.split(",");
                var original = document.getElementById("fid");


                var node = document.getElementById('parent');
                node.innerHTML = '';

                if (tokens[0] != "") {
                    for (const token in tokens) {
                        var value = tokens[token];
                        var text = original.childNodes[0];


                        text.nodeValue = value;
                        original.style.display = "block";
                        if(value == activeId){
                        original.style.background = "#8aaf8a";
                        }else{
                            original.style.background = "#dddddd";
                        }
                        var clone = original.cloneNode(true);
                        clone.setAttribute('id', value);
                        var element = document.getElementById("parent").appendChild(clone);

                        // delete handlers
                        element.childNodes[1].addEventListener("click", function () {
                            var msg = "[delete]";
                            msg = msg + this.parentElement.id;
                            port.postMessage(msg);
                        });

                        // decrypt handlers
                        element.childNodes[3].addEventListener("click", function () {
                            var msg = "[decrypt]";
                            msg = msg + this.parentElement.id;
                            port.postMessage(msg);
                        });

                    }
                }

                original.style.display = "none"
                document.getElementById("load").style.display = "none";
            });
            });
            }
        }


    });

}, false);