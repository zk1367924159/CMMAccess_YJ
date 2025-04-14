var getURL = function() {
  var url = document.location.href;
  var pos = url.indexOf('/', 8);
  if(-1 == pos) {
    pos = url.length;
  }
  url = url.substring(0, pos);
  return url;
};

var GetExtAppParam = function(url, name, elem) {
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      if (xhr.status === 200) {
        let json = JSON.parse(xhr.responseText);
        elem.value = json[name];
      } else {
        var messages = document.getElementById('messages');
        messages.textContent += ('\n' + xhr.responseText);
      }
    }
  };
  xhr.send(JSON.stringify({
  "key": name,
  "waitTime": 1000
  }));
}

var SaveExtAppParam = function(url, name, elem) {
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      if (xhr.status === 200) {
        //console.log(xhr.responseText);
      } else {
        var messages = document.getElementById('messages');
        messages.textContent += ('\n' + xhr.responseText);
      }
    } else {
    }
  };
  xhr.send(JSON.stringify({
    "key": name,
    "val": elem.value,
    "waitTime": 1000
  }));
}

var RebootSys = function(url, elem) {
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      var messages = document.getElementById('messages');
      if (xhr.status === 200) {
        //console.log(xhr.responseText);
        messages.textContent = xhr.responseText;
        elem.textContent = '重启请求已经完成。';
      } else {
        messages.textContent = xhr.responseText;
      }
    } else {
    }
  };
  xhr.send(JSON.stringify({
    "waitTime": 1000
  }));
}

var load = function() {
  var url = getURL() + '/api/GetExtAppParam';
  var inputs = document.getElementsByTagName("input");
  var messages = document.getElementById('messages');
  messages.textContent = '';
  for(let x of inputs) {
    let id = x.attributes.getNamedItem("id").value;
    GetExtAppParam(url, id, x);
  }
  var selects = document.getElementsByTagName("select");
  for(let x of selects) {
    let id = x.attributes.getNamedItem("id").value;
    let target = document.querySelector('#' + id);
    GetExtAppParam(url, id, target);
  }
  var status = document.getElementById('status');
  status.textContent = '参数已装载。';
};

var save = function() {
  var url = getURL() + '/api/SaveExtAppParam';
  var inputs = document.getElementsByTagName("input");
  var messages = document.getElementById('messages');
  messages.textContent = '';
  for(let x of inputs) {
    let id = x.attributes.getNamedItem("id").value;
    SaveExtAppParam(url, id, x);
  }
  var selects = document.getElementsByTagName("select");
  for(let x of selects) {
    let id = x.attributes.getNamedItem("id").value;
    let target = document.querySelector('#' + id);
    SaveExtAppParam(url, id, target);
  }
  var status = document.getElementById('status');
  status.textContent = '参数已保存。';
};

var reboot = function() {
  var url = getURL() + '/api/RebootSys';
  var messages = document.getElementById('messages');
  messages.textContent = '';
  var status = document.getElementById('status');
  RebootSys(url, status);
  status.textContent = '重启请求已发送。';
};

var print = function() {
  var inputs = document.getElementsByTagName("input");
  for(let x of inputs) {
    console.log(x.attributes.getNamedItem("id").value);
    for(let y of x.attributes) {
      console.log("  " + y.name + "=" + y.value);
    }
  }
  var selects = document.getElementsByTagName("select");
  for(let x of selects) {
    let id = x.attributes.getNamedItem("id").value;
    console.log('select id=' + id);
    console.log(document.querySelector('#' + id).value);
  }
};

document.addEventListener("DOMContentLoaded", () => {
  var btnLoad = document.getElementById('load'); btnLoad.onclick = load;
  var btnSave = document.getElementById('save'); btnSave.onclick = save;
  var btnReboot = document.getElementById('reboot'); btnReboot.onclick = reboot;
  load();
});

