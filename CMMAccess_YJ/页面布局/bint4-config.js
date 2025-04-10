var getURL = function() {
  let url = document.location.href;
  let pos = url.indexOf('/', 8);
  if(-1 == pos) {
    pos = url.length;
  }
  url = url.substring(0, pos) + ':9080';
  console.log("url=" + url);
  return url;
};

var load = function() {
  let url = getURL() + '/api/LoadParams';
  let inputs = document.getElementsByTagName("input");
  let selects = document.getElementsByTagName("select");
  let messages = document.getElementById('messages');
  messages.textContent = '';

  // collect keys
  const keys = [];
  for(let x of inputs) {
    let key = x.attributes.getNamedItem("id").value;
    keys.push(key);
  }
  for(let x of selects) {
    let key = x.attributes.getNamedItem("id").value;
    keys.push(key);
  }

  // query values by submitting keys
  let xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      let messages = document.getElementById('messages');
      messages.textContent = xhr.responseText;
      //console.log(xhr.responseText);
      if (xhr.status === 200) {
        let json = JSON.parse(xhr.responseText);
        // dispatch values.
        for(const key in json) {
          let elem = document.getElementById(key);
          if(!elem)
          {
            console.warn('未找到 ID 为 "' + key + '" 的元素');
          } else if(elem.tagName == 'INPUT') {
            elem.value = json[key];
          } else if(elem.tagName == 'SELECT') {
            let target = document.querySelector('#' + key);
            target.value = json[key];
          } else {
            console.log("load() : elem.tagName='" + elem.tagName + "'");
          }
        }
        // update controls complete,
        // update status text
        let status = document.getElementById('status');
        status.textContent = '参数已装载。';
      } else {
      }
    }
  };
  xhr.send(JSON.stringify(keys));
}

var validateText = function(src, err) {
  let defvalItem = src.attributes.getNamedItem("defval");
  if(defvalItem === null) return true;
  let defval = defvalItem.value;
  let patternItem = src.attributes.getNamedItem("pattern");
  if(patternItem === null) return true;
  let pattern = patternItem.value;
  console.log('defval=' + defval);
  console.log('pattern=' + pattern);
  const re = new RegExp(pattern);
  let s = src.value.trim();
  if(s.match(re) === null) {
    //src.value = defval;
    err.textContent = '数据不符合要求';
    return false;
  } else {
    err.textContent = '';
    src.value = s;
    return true;
  }
}

var save = function() {
  let url = getURL() + '/api/SaveParams';
  let inputs = document.getElementsByTagName("input");
  let selects = document.getElementsByTagName("select");
  let messages = document.getElementById('messages');
  messages.textContent = '';

  let errorCount = 0;

  // collect param keys and its values
  const params = {};
  for(let x of inputs) {
    let key = x.attributes.getNamedItem("id").value;
    let errorElement = document.getElementById(key + "-error");
    if(!(errorElement === null)) {
      if(!validateText(x, errorElement)) {
        errorCount += 1;
      }
    }
    params[key] = x.value;
  }
  if(0 < errorCount) {
    let status = document.getElementById('status');
    status.textContent = '输入错误。';
    messages.textContent = '请求未发出。';
    return;
  }
  for(let x of selects) {
    let key = x.attributes.getNamedItem("id").value;
    let target = document.querySelector('#' + key);
    params[key] = target.value;
  }
  let xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      let messages = document.getElementById('messages');
      messages.textContent = xhr.responseText;
      if (xhr.status === 200) {
        console.log(xhr.responseText);
        let status = document.getElementById('status');
        status.textContent = '参数已保存。';
      } else {
      }
    } else {
    }
  };
  console.log(JSON.stringify(params));
  xhr.send(JSON.stringify(params));
}

var reboot = function() {
  var url = getURL() + '/api/RebootSys';
  var messages = document.getElementById('messages');
  messages.textContent = '';
  var status = document.getElementById('status');
  status.textContent = '重启请求已发送。';
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      let messages = document.getElementById('messages');
      messages.textContent = xhr.responseText;
      if (xhr.status === 200) {
        //console.log(xhr.responseText);
        elem.textContent = '重启请求已经完成。';
      } else {
      }
    } else {
    }
  };
  xhr.send(JSON.stringify({
    "waitTime": 1000
  }));
}

var removeAllAlarm = function() {
  var url = getURL() + '/api/RemoveAllAlarm';
  var messages = document.getElementById('messages');
  messages.textContent = '';
  var status = document.getElementById('status');
  status.textContent = '删除所有未发送告警请求已发送。';
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.onreadystatechange = function() {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      let messages = document.getElementById('messages');
      messages.textContent = xhr.responseText;
      if (xhr.status === 200) {
        //console.log(xhr.responseText);
        elem.textContent = '删除所有未发送告警请求已经完成。';
      } else {
      }
    } else {
    }
  };
  xhr.send(JSON.stringify({
    "waitTime": 1000
  }));
}

var verifyPattern = function(src) {
  let defvalItem = src.srcElement.attributes.getNamedItem("defval");
  if(defvalItem === null) return;
  let defval = defvalItem.value;
  let patternItem = src.srcElement.attributes.getNamedItem("pattern");
  if(patternItem === null) return;
  let pattern = patternItem.value;
  console.log('defval=' + defval);
  console.log('pattern=' + pattern);
  const re = new RegExp(pattern);
  let s = src.srcElement.value.trim();
  if(s.match(re) === null) {
    src.srcElement.value = defval;
  } else {
    src.srcElement.value = s;
  }
}

var verifyTimePattern = function(src) {
  const re = new RegExp("^([0-9]{2}):([0-9]{2}):([0-9]{2})$");
  let s = src.srcElement.value.trim();
  let matched = s.match(re);
  console.log(matched);
  let key = src.srcElement.attributes.getNamedItem("id").value;
  let errorElement = document.getElementById(key + "-error");
  if(matched === null) {
    if(!(errorElement === null)) {
      errorElement.textContent = '数据不符合要求';
    }
  } else {
    let hour = parseInt(matched[1]);
    let minute = parseInt(matched[2]);
    let second = parseInt(matched[3]);
    if(hour < 24 && minute < 60 && second < 60) {
      src.srcElement.value = s;
      errorElement.textContent = '';
    } else {
      errorElement.textContent = '数据不符合要求';
    }
  }
}

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

var onPageLoad = function() {
  console.log("/js/config.js : onPageLoad()");
  var btnLoad = document.getElementById('load'); btnLoad.onclick = load;
  var btnSave = document.getElementById('save'); btnSave.onclick = save;
  var btnReboot = document.getElementById('reboot'); btnReboot.onclick = reboot;
  var btnRemoveAllAlarm = document.getElementById('remove-all-alarm'); btnRemoveAllAlarm.onclick = removeAllAlarm;
  var fsuAutoRebootTime = document.getElementById('fsu-auto-reboot-time'); fsuAutoRebootTime.onchange = verifyTimePattern;
  load();
}

onPageLoad();

/*
document.addEventListener("DOMContentLoaded", () => {
  console.log("/js/config.js : DOMContentLoaded");
  onPageLoad();
});
*/

